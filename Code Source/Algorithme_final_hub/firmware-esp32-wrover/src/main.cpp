#include "HubDataStorage.hpp"
#include "WifiMessageManager.hpp"
#include <ArduinoEigenDense.h>
#include <ArduinoJson.h>

//Hugues : Test communications via WiFi.
#include <WiFi.h>
#include <HTTPClient.h>
//\Hugues

#include "Trilateration.h"
#include "GridLibrary.hpp"
#include "Polygone.hpp"
#include "RecuperationDonneesAncres.hpp"
#include "ScreenCommunicationManager.hpp"

RecuperationDonneesAncres recupDonnees;
UWBModuleList vAnchors;
Polygone vSafeZone;

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

    // 2. MISE À JOUR DES ANCRES (vAnchors)
    // Code à adapter/décommenter selon les méthodes de votre classe UWBModuleList
    int anchorsCount = 0;
    for (JsonVariant s : sensorsJson) {
        float sx = s["x"].as<float>();
        float sy = s["y"].as<float>();
        // vAnchors.mettreAJourAncre(anchorsCount, sx, sy);
        anchorsCount++;
    }
    Serial.printf("[Main Hub] %d ancres de capteurs synchronisees.\n", anchorsCount);
    Serial.println("[Main Hub] --- Fin d'application de la configuration ---\n");
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  initHub();
  setupScreenCommunication();

  // FIX : On initialise les variables GLOBALES (on a retiré "UWBModuleList" et "Polygone" devant)
  vAnchors = initAnchors("Anchors");
  Serial.println(("Contenu de la liste des ancres :\n" + vAnchors.toString()).c_str());

  UWBModuleList vTags; // Liste locale des tags (vide)

  vSafeZone = Polygone(0, initSafeZone("SafeZone"));
  Serial.println(("Contenu de la zone de sécurité :\n" + vSafeZone.toString()).c_str());
 
  Serial.println("Adresse MAC :");
  Serial.println(WiFi.macAddress());
  initWifi();

  if (vSafeZone.getPoints().size() > 0) {
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
    for (const DistanceMoyennes& tag : tagsPretsPourMaths) {
      Serial.printf("[30Hz] Tag %d |D0:%.2f | D1:%.2f | D2:%.2f | D3:%.2f\n", tag.tag_id, tag.distances[0], tag.distances[1], tag.distances[2], tag.distances[3]);
    }
  }

  static unsigned long chronoRuntime = 0;
  if (millis() - chronoRuntime >= 33) { 
      int ids[3] = {4, 5, 6};
      float xs[3] = {10, -5, 7};
      float ys[3] = {-10, 4, 10};
      float dists[3] = {14.9, 8.5, 12.5};
      bool alarmes[3] = {false, true, false}; 

      envoyerMiseAJourTagsRuntime(ids, xs, ys, dists, alarmes, 3);
      chronoRuntime = millis();
  }
}

void executer_HUB_STATE_DETECTING_TAGS_FOR_INIT() {
  Serial.println("[Machine Etats] Étape : Scan initial des tags demandé...");
  delay(1000); 

  int listeIds[3] = {101, 105, 110};
  float listeDistances[3] = {4.2, 2.1, 7.8};
  int nombreDeTagsTrouves = 3;

  envoyerListeTagsDecouverts(listeIds, listeDistances, nombreDeTagsTrouves);

  etatActuelHub = HUB_STATE_IDLE;
  Serial.println("[Machine Etats] Scan terminé. Liste envoyée. Retour en IDLE.");
}

void executer_HUB_STATE_GENERATING_GEOMETRY() {
  Serial.printf("[Machine Etats] Étape : Génération géométrie pour le Tag cible #%d...\n", idTagSelectionne);
  delay(1500); 

  float ancresCalculees[4][2] = {
      {-1.0, 2.0},  {1.0, 2.0}, {-1.0, -2.0}, {1.0, -2.0}
  };

  float zone64PointsCalculee[64][2];
  for (int i = 0; i < 64; i++) {
      float angle = (i * 2.0 * PI) / 64.0;
      zone64PointsCalculee[i][0] = cos(angle) * 7.0; 
      zone64PointsCalculee[i][1] = sin(angle) * 7.0;
  }

  envoyerGeometrieCalibration(ancresCalculees, zone64PointsCalculee);

  etatActuelHub = HUB_STATE_IDLE;
  Serial.println("[Machine Etats] Géométrie initiale envoyée. Retour en IDLE.");
}

void executer_HUB_STATE_COLLECTING_POINTS() {
  static unsigned long chronoLog = 0;
  if (millis() - chronoLog >= 2000) {
    Serial.printf("[Machine Etats] En cours d'acquisition de la zone d'exclusion");
    chronoLog = millis();
  }

  // APPEL D'UNE FONCTION POUR FAIRE CA.
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