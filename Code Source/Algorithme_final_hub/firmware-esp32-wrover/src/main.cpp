#include "Config.hpp"
#include "HubDataStorage.hpp"
#include "WifiMessageManager.hpp"
#include "GestionnaireAncres.hpp"
#include "InitAnchorPosition.hpp"
#include <ArduinoEigenDense.h>
#include <ArduinoJson.h>
#include <cmath>

//Hugues : Test communications via WiFi.
#include <WiFi.h>
#include <HTTPClient.h>
//\Hugues

#include "Trilateration.h"
#include "GridLibrary.hpp"
#include "Polygone.hpp"
#include "RecuperationDonneesAncres.hpp"
#include "ScreenCommunicationManager.hpp"
#include "ListeDistanceLibrary.hpp"
#include "LissagePlanExclusion.hpp"
#include "CalibrationManager.hpp"

RecuperationDonneesAncres recupDonnees;
UWBModuleList vAnchors;
Polygone vSafeZone;
CalibrationManager calibManager;

// Dictionnaire pour retenir la hauteur (Z) des ancres avant que l'écran ne valide le X et Y
std::map<int, float> hauteursAncresTemporaires;

void ecouterReseauFilaire() {
  if (MODE_ACTUEL == MODE_WIRED) {
    // MAXIME
  }
}

void OnWifidataReceived(int tagId, float d0, float d1, float d2, float d3) {
  if (MODE_ACTUEL == MODE_WIFI) {
    recupDonnees.injecterDonnee(tagId, d0, d1, d2, d3);
  }
}

// Application de la configuration reçue de l'IHM Écran
void appliquerNouvelleConfigurationMaterielle(JsonArray zoneJson, JsonArray sensorsJson) {
    Serial.println("\n[Main Hub] --- Réception d'une nouvelle configuration matérielle ---");
   
    // 1. MISE À JOUR DE LA ZONE DE SÉCURITÉ (vSafeZone)
    std::vector<V3> nouveauxSommets;
    for (JsonVariant v : zoneJson) {
        float x = v["x"].as<float>();
        float y = v["y"].as<float>();
        // Le téléphone envoie du X et Y (2D), on initialise le Z à 0.0f
        nouveauxSommets.push_back(V3(x, y, 0.0f));
    }
   
    // On écrase l'ancienne zone globale par le nouveau polygone mis à jour
    vSafeZone = Polygone(0, nouveauxSommets);
    Serial.printf("[Main Hub] %d sommets appliques à vSafeZone.\n", nouveauxSommets.size());
    saveData("SafeZone", "Points", vSafeZone.getPoints().data(), vSafeZone.getPoints().size());

    // 2. MISE À JOUR DES ANCRES (vAnchors)
    // On récupère le X et Y de la tablette (0,0 = véhicule) + le Z qu'on avait mis de côté
    int anchorsCount = 0;
    std::vector<int> aIds = vAnchors.giveModuleIdList();
    std::map<int, V3> mapAncresPourFlash;

    for (JsonVariant s : sensorsJson) {
        if (anchorsCount < aIds.size()) {
            int id = aIds[anchorsCount];
            float sx = s["x"].as<float>();
            float sy = s["y"].as<float>();
            
            float sz = hauteursAncresTemporaires.count(id) ? hauteursAncresTemporaires[id] : 0.0f;
            V3 nouvellePos(sx, sy, sz);
            
            // vAnchors.mettreAJourAncre(anchorsCount, sx, sy); -> Remplacé par la vraie méthode
            vAnchors.setModulePosition(id, nouvellePos);
            mapAncresPourFlash[id] = nouvellePos;
        }
        anchorsCount++;
    }
    
    saveMapData("Anchors", "Positions", mapAncresPourFlash);

    Serial.printf("[Main Hub] %d ancres de capteurs synchronisees.\n", anchorsCount);
    Serial.println("[Main Hub] --- Fin d'application de la configuration ---\n");
    
    etatActuelHub = HUB_STATE_RUNNING;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  initHub();
  setupScreenCommunication();
  initGestionnaireAncres(); // Essentiel pour initialiser le carnet d'adresses

  // FIX : On initialise les variables GLOBALES (on a retiré "UWBModuleList" et "Polygone" devant)
  vAnchors = initAnchors("Anchors");
  Serial.println(("Contenu de la liste des ancres :\n" + vAnchors.toString()).c_str());

  UWBModuleList vTags; // Liste locale des tags (vide)

  vSafeZone = Polygone(0, initSafeZone("SafeZone"));
  Serial.println(("Contenu de la zone de sécurité :\n" + vSafeZone.toString()).c_str());
  
  // Prépare l'épicentre au cas où une calibration est relancée
  calibManager.initialiserEpicentre(vAnchors);
 
  Serial.println("Adresse MAC :");
  Serial.println(WiFi.macAddress());
  initWifi();

  if (vSafeZone.getPoints().size() > 0 && vAnchors.size() == 4) {
      etatActuelHub = HUB_STATE_RUNNING;
      Serial.println("[Boot] Configuration existante trouvée. Passage automatique en RUNNING.");
  } else {
      etatActuelHub = HUB_STATE_IDLE;
  }
}

void executer_HUB_STATE_RUNNING() {
  ecouterReseauFilaire();

  std::vector<DistanceMoyennes> tagsPretsPourMaths;
  if (recupDonnees.getDonneesLissees(tagsPretsPourMaths)) {
      
      int ids[MAX_TAGS];
      float xs[MAX_TAGS], ys[MAX_TAGS], distsScreen[MAX_TAGS];
      bool alarmes[MAX_TAGS];
      int tagCount = 0;
      std::vector<int> aIds = vAnchors.giveModuleIdList();

      for (const DistanceMoyennes& tag : tagsPretsPourMaths) {
          // Affichage conservé pour le debug
          // Serial.printf("[30Hz] Tag %d |D0:%.2f | D1:%.2f | D2:%.2f | D3:%.2f\n", tag.tag_id, tag.distances[0], tag.distances[1], tag.distances[2], tag.distances[3]);

          std::unordered_map<int, float> distMap;
          for(int i = 0; i < 4 && i < aIds.size(); i++) {
              distMap[aIds[i]] = tag.distances[i];
          }

          V3 pos3D = trilateration3D(vAnchors, distMap);

          // Si le tag est à plus de 5m en hauteur, on ne l'envoie pas
          if (std::abs(pos3D.getZ()) > 5.0f) {
              continue; 
          }

          V3 posProjectee2D(pos3D.getX(), pos3D.getY(), 0.0f);
          bool inDanger = vSafeZone.isInside(posProjectee2D);
          float distSafeZone = vSafeZone.getDistanceFrom(posProjectee2D);

          if (tagCount < MAX_TAGS) {
              ids[tagCount] = tag.tag_id;
              xs[tagCount] = posProjectee2D.getX();
              ys[tagCount] = posProjectee2D.getY();
              distsScreen[tagCount] = distSafeZone;
              alarmes[tagCount] = inDanger;
              tagCount++;
          }
      }

      static unsigned long chronoRuntime = 0;
      if (millis() - chronoRuntime >= FENETRE_MS && tagCount > 0) {
          envoyerMiseAJourTagsRuntime(ids, xs, ys, distsScreen, alarmes, tagCount);
          chronoRuntime = millis();
      }
  }
}

void executer_HUB_STATE_DETECTING_TAGS_FOR_INIT() {
  Serial.println("[Machine Etats] Étape : Scan initial des tags demandé...");
  delay(1000);

  initAnchorsPosition(vAnchors);
  calibManager.initialiserEpicentre(vAnchors);
  calibManager.viderPoints();

  std::vector<DistanceMoyennes> tagsLisses;
  std::vector<int> listeIds;
  std::vector<float> listeDistances;
  std::vector<int> aIds = vAnchors.giveModuleIdList();

  if (recupDonnees.getDonneesLissees(tagsLisses)) {
      for (const auto& tag : tagsLisses) {
          std::unordered_map<int, float> distMap;
          for(int i = 0; i < 4 && i < aIds.size(); i++) {
              distMap[aIds[i]] = tag.distances[i];
          }

          V3 pos3D = trilateration3D(vAnchors, distMap);
          V3 vecteurPosition = pos3D - calibManager.getEpicentre();

          listeIds.push_back(tag.tag_id);
          listeDistances.push_back(vecteurPosition.norm());
      }
  }

  // Envoi dynamique
  envoyerListeTagsDecouverts(listeIds.data(), listeDistances.data(), listeIds.size());

  etatActuelHub = HUB_STATE_IDLE;
  Serial.println("[Machine Etats] Scan terminé. Liste envoyée. Retour en IDLE.");
}

void executer_HUB_STATE_GENERATING_GEOMETRY() {
  Serial.printf("[Machine Etats] Étape : Génération géométrie pour le Tag cible #%d...\n", idTagSelectionne);
  delay(1500);

  if (calibManager.getNombrePoints() < 3) {
      Serial.println("[Erreur] Pas assez de points pour la géométrie.");
      etatActuelHub = HUB_STATE_IDLE;
      return;
  }

  const std::vector<V3>& pointsCollectes = calibManager.getPoints();
  LissageVehicule::PlanLocal planVehicule = LissageVehicule::calculerPlanMoyen(pointsCollectes);
  Polygone zone64 = LissageVehicule::echantillonner64Points(0, planVehicule, pointsCollectes);
  const std::vector<V3>& pts = zone64.getPoints();

  float ancresCalculees[4][2];
  std::vector<int> aIds = vAnchors.giveModuleIdList();
  hauteursAncresTemporaires.clear();

  for (int i = 0; i < 4 && i < aIds.size(); i++) {
      int id = aIds[i];
      V3 posRelative = (vAnchors.getModule(id).getPosition() - calibManager.getEpicentre()) - planVehicule.centre;
      
      ancresCalculees[i][0] = prodScal(posRelative, planVehicule.axeU);
      ancresCalculees[i][1] = prodScal(posRelative, planVehicule.axeV);
      
      hauteursAncresTemporaires[id] = prodScal(posRelative, planVehicule.normale);
  }

  float zone64PointsCalculee[64][2];
  for (int i = 0; i < 64 && i < pts.size(); i++) {
      V3 posRelative = pts[i] - planVehicule.centre;
      zone64PointsCalculee[i][0] = prodScal(posRelative, planVehicule.axeU);
      zone64PointsCalculee[i][1] = prodScal(posRelative, planVehicule.axeV);
  }

  envoyerGeometrieCalibration(ancresCalculees, zone64PointsCalculee);

  calibManager.viderPoints();
  etatActuelHub = HUB_STATE_IDLE;
  Serial.println("[Machine Etats] Géométrie initiale envoyée. Retour en IDLE.");
}

void executer_HUB_STATE_COLLECTING_POINTS() {
  static unsigned long chronoLog = 0;
  if (millis() - chronoLog >= 2000) {
    Serial.printf("[Machine Etats] En cours d'acquisition de la zone d'exclusion\n");
    chronoLog = millis();
  }

  static unsigned long chronoAcquisition = 0;
  if (millis() - chronoAcquisition >= FENETRE_MS) { 
      std::vector<DistanceMoyennes> tagsLisses;
      std::vector<int> aIds = vAnchors.giveModuleIdList();

      if (recupDonnees.getDonneesLissees(tagsLisses)) {
          for (const auto& tag : tagsLisses) {
              if (tag.tag_id == idTagSelectionne) {
                  std::unordered_map<int, float> distMap;
                  for(int i = 0; i < 4 && i < aIds.size(); i++) {
                      distMap[aIds[i]] = tag.distances[i];
                  }

                  V3 pos3D = trilateration3D(vAnchors, distMap);
                  calibManager.ajouterPoint(pos3D - calibManager.getEpicentre());
              }
          }
      }
      chronoAcquisition = millis();
  }
}

void executer_HUB_STATE_IDLE() {
    // Attente passive d'une configuration ou d'une commande
}

void reinitialiserObjetsMetierHub() {
  vSafeZone = Polygone(0, std::vector<V3>());
  Serial.println("[Main Hub] Configuration entièrement vidée de la RAM.");
}

void loop() {
  loopScreenCommunication();

  switch (etatActuelHub) {
      case HUB_STATE_IDLE:
          executer_HUB_STATE_IDLE();
          break;
      case HUB_STATE_DETECTING_TAGS_FOR_INIT:
          executer_HUB_STATE_DETECTING_TAGS_FOR_INIT();
          break;
      case HUB_STATE_COLLECTING_POINTS:
          executer_HUB_STATE_COLLECTING_POINTS();
          break;
      case HUB_STATE_GENERATING_GEOMETRY:
          executer_HUB_STATE_GENERATING_GEOMETRY();
          break;
      case HUB_STATE_RUNNING:
          executer_HUB_STATE_RUNNING();
          break;
  }
}