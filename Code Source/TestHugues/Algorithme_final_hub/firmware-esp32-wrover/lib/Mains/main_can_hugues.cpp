#include "Config.hpp"
#include "HubDataStorage.hpp"
#include "WifiMessageManager.hpp"
#include "GestionnaireAncres.hpp"
#include "InitAnchorPosition.hpp"

// ---- AJOUT DE LA LIBRAIRIE CAN ----
#include "CANMessageManager.hpp" 

#include <ArduinoEigenDense.h>
#include <ArduinoJson.h>
#include <cmath> 

#include <WiFi.h>
#include <HTTPClient.h>

#include "Trilateration.h"
#include "GridLibrary.hpp"
#include "Polygone.hpp"
#include "RecuperationDonneesAncres.hpp"
#include "ScreenCommunicationManager.hpp"
#include "ListeDistanceLibrary.hpp"
#include "LissagePlanExclusion.hpp"
#include "CalibrationManager.hpp"

// --- VARIABLES GLOBALES ---
RecuperationDonneesAncres recupDonnees;
UWBModuleList vAnchors;
Polygone vSafeZone;
CalibrationManager calibManager;

// Dictionnaire pour retenir la hauteur (Z) des ancres avant que l'écran ne valide le X et Y
std::map<int, float> hauteursAncresTemporaires;

// Variables globales de la machine à états partagées avec le ScreenCommunicationManager
volatile HubState etatActuelHub = HUB_STATE_IDLE;
volatile int idTagSelectionne = -1;

/**
 * @brief Fonction vitale pour le mode FILAIRE (CAN).
 * Doit être appelée très fréquemment pour vider le buffer matériel TWAI (CAN)
 * et injecter les distances reçues des ancres dans l'algorithme de calcul.
 */
void ecouterReseauFilaire() {
    if (MODE_ACTUEL == MODE_WIRED) {
        twai_message_t messageRecu;
        
        // Tant qu'il y a des messages en attente dans le buffer CAN, on les lit
        while (receiveCanMessage(messageRecu)) {
            DecodedData donnees;
            
            // On décode la trame brute en structure lisible
            if (decodeCanMessage(messageRecu, donnees)) {
                
                // Si le message contient la liste des 4 distances d'un Tag (Type 4)
                if (donnees.type == MESSAGE_TAG_ID_AND_ALL_DISTANCES) {
                    
                    // Sécurité : on vérifie qu'on a bien au moins 4 distances dans le vecteur
                    if (donnees.aDistances.size() >= 4) {
                        // On injecte les données dans l'accumulateur/lisseur central !
                        // C'est l'équivalent parfait de 'OnWifidataReceived'
                        recupDonnees.injecterDonnee(donnees.id_tag, 
                                                    donnees.aDistances[0], 
                                                    donnees.aDistances[1], 
                                                    donnees.aDistances[2], 
                                                    donnees.aDistances[3]);
                    }
                }
            }
        }
    }
}

// Fonction de réception (Interruption matérielle) exclusive au Wi-Fi
void OnWifidataReceived(int tagId, float d0, float d1, float d2, float d3) {
  if (MODE_ACTUEL == MODE_WIFI) {
    recupDonnees.injecterDonnee(tagId, d0, d1, d2, d3);
  }
}

// Application de la configuration reçue de l'IHM Écran
void appliquerNouvelleConfigurationMaterielle(JsonArray zoneJson, JsonArray sensorsJson) {
    Serial.println("\n[Main Hub] --- Réception de la configuration validée par l'écran ---");
   
    // 1. MISE À JOUR DE LA ZONE DE SÉCURITÉ (vSafeZone)
    std::vector<V3> nouveauxSommets;
    for (JsonVariant v : zoneJson) {
        // L'écran renvoie la forme géométrique autour du véhicule.
        // Le Z est forcé à 0 car nous sommes désormais dans le repère 2D du plan du véhicule.
        nouveauxSommets.push_back(V3(v["x"].as<float>(), v["y"].as<float>(), 0.0f));
    }
   
    vSafeZone = Polygone(0, nouveauxSommets);
    Serial.printf("[Main Hub] %d sommets appliques à vSafeZone.\n", nouveauxSommets.size());
    saveData("SafeZone", "Points", vSafeZone.getPoints().data(), vSafeZone.getPoints().size());

    // 2. MISE À JOUR DES ANCRES (Fusion 2D Écran + Hauteur 1D Calculée)
    int anchorsCount = 0;
    std::vector<int> aIds = vAnchors.giveModuleIdList();
    std::map<int, V3> mapAncresPourFlash;

    for (JsonVariant s : sensorsJson) {
        if (anchorsCount < aIds.size()) {
            int id = aIds[anchorsCount];
            float sx = s["x"].as<float>(); // Position X renvoyée par rapport au centre véhicule (0,0)
            float sy = s["y"].as<float>(); // Position Y renvoyée par rapport au centre véhicule (0,0)
            
            // On récupère la hauteur qu'on avait mise de côté (ou 0 par défaut)
            // C'est ce qui nous permet de garder la vraie trilatération 3D !
            float sz = hauteursAncresTemporaires.count(id) ? hauteursAncresTemporaires[id] : 0.0f;
            
            V3 nouvellePos(sx, sy, sz);
            
            vAnchors.setModulePosition(id, nouvellePos);
            mapAncresPourFlash[id] = nouvellePos;
        }
        anchorsCount++;
    }
    
    saveMapData("Anchors", "Positions", mapAncresPourFlash);

    Serial.printf("[Main Hub] %d ancres de capteurs synchronisees.\n", anchorsCount);
    Serial.println("[Main Hub] --- Fin d'application de la configuration ---\n");
    
    // Le Hub est officiellement calibré et prêt à surveiller les ouvriers
    etatActuelHub = HUB_STATE_RUNNING;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  initHub();
  setupScreenCommunication();
  initGestionnaireAncres(); 

  // --- INITIALISATION DES CANAUX DE COMMUNICATION ---
  if (MODE_ACTUEL == MODE_WIRED) {
      // Configuration physique du bus CAN
      initCan(HUB_RX_PIN, HUB_TX_PIN);
      Serial.println("[Boot] Bus CAN (Filaire) initialisé.");
  } else {
      // Configuration ESP-NOW Wi-Fi
      Serial.println("Adresse MAC :");
      Serial.println(WiFi.macAddress());
      initWifi();
      Serial.println("[Boot] Wi-Fi ESP-NOW initialisé.");
  }

  // Restauration de la mémoire Flash NVS
  vAnchors = initAnchors("Anchors");
  vSafeZone = Polygone(0, initSafeZone("SafeZone"));
  
  // Prépare le gestionnaire de calibration avec le bon épicentre 
  // au cas où l'utilisateur relance une calibration depuis l'écran
  calibManager.initialiserEpicentre(vAnchors);
 
  // Démarre directement la surveillance si on a une zone et 4 ancres valides en mémoire
  if (vSafeZone.getPoints().size() > 0 && vAnchors.size() == 4) {
      etatActuelHub = HUB_STATE_RUNNING;
      Serial.println("[Boot] Configuration existante trouvée. Passage automatique en RUNNING.");
  } else {
      etatActuelHub = HUB_STATE_IDLE;
  }
}

void executer_HUB_STATE_RUNNING() {
  std::vector<DistanceMoyennes> tagsPretsPourMaths;

  // On demande les dernières données calculées et moyennées (sur 30Hz)
  if (recupDonnees.getDonneesLissees(tagsPretsPourMaths)) {
      
      int ids[MAX_TAGS];
      float xs[MAX_TAGS], ys[MAX_TAGS], distsScreen[MAX_TAGS];
      bool alarmes[MAX_TAGS];
      int tagCount = 0;
      std::vector<int> aIds = vAnchors.giveModuleIdList();

      // Pour chaque tag détecté par le système UWB
      for (const DistanceMoyennes& tag : tagsPretsPourMaths) {
          
          // On refait le lien entre la distance lue et le véritable ID de l'ancre
          std::unordered_map<int, float> distMap;
          for(int i = 0; i < 4 && i < aIds.size(); i++) {
              distMap[aIds[i]] = tag.distances[i];
          }

          // TRILATÉRATION
          // Le résultat pos3D donne la position du tag par rapport au centre du véhicule.
          V3 pos3D = trilateration3D(vAnchors, distMap);

          // SÉCURITÉ VERTICALE
          // Si le tag est à plus de 5m en hauteur, c'est sûrement une grue, on ignore.
          if (std::abs(pos3D.getZ()) > 5.0f) {
              continue; 
          }

          // ÉVALUATION DANGER
          // On ramène le tag sur le plan du sol (Z=0) pour vérifier s'il franchit la ligne 2D
          V3 posProjectee2D(pos3D.getX(), pos3D.getY(), 0.0f);
          bool inDanger = vSafeZone.isInside(posProjectee2D);
          float distSafeZone = vSafeZone.getDistanceFrom(posProjectee2D);

          // RETOUR D'INFORMATION VERS LE TAG
          // Si l'ouvrier n'est pas dans le polygone (distSafeZone > 0), 
          // on lui envoie sa distance pour allumer ses LEDs de couleur ou biper.
          if (distSafeZone > 0.0f) {
              if (MODE_ACTUEL == MODE_WIRED) {
                  // Le Hub dit à l'Ancre 0 (Maître) de faire le pont vers le Tag via l'UWB
                  // sendCanDistance() va forger un paquet CAN pour l'ancre.
                  sendCanDistance(aIds[0], tag.tag_id, distSafeZone);
              } else {
                  // Mode Wi-Fi : on utiliserait ici un envoi ESP-NOW
                  // envoyerOrdreChangementRole(tag.tag_id, COMMAND_UPDATE_DISTANCE, distSafeZone);
              }
          }

          // Stockage pour affichage sur l'écran déporté
          if (tagCount < MAX_TAGS) {
              ids[tagCount] = tag.tag_id;
              xs[tagCount] = posProjectee2D.getX();
              ys[tagCount] = posProjectee2D.getY();
              distsScreen[tagCount] = distSafeZone;
              alarmes[tagCount] = inDanger;
              tagCount++;
          }
      }

      // Envoi du rafraîchissement global à l'écran via l'UART
      static unsigned long chronoRuntime = 0;
      if (millis() - chronoRuntime >= FENETRE_MS && tagCount > 0) {
          envoyerMiseAJourTagsRuntime(ids, xs, ys, distsScreen, alarmes, tagCount);
          chronoRuntime = millis();
      }
  }
}

void executer_HUB_STATE_DETECTING_TAGS_FOR_INIT() {
  Serial.println("[Machine Etats] Étape : Scan initial des tags demandé...");

  // 1) Auto-Calibration matérielle via descente de gradient
  // Va interroger les ancres pour qu'elles se mesurent entre elles (Wi-Fi ou CAN)
  initAnchorsPosition(vAnchors);
  
  // 2) Configuration du repère temporaire (L'origine devient le milieu des ancres)
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
          // On calcule la distance brute entre le tag cible et l'épicentre
          V3 vecteurPosition = pos3D - calibManager.getEpicentre();

          listeIds.push_back(tag.tag_id);
          listeDistances.push_back(vecteurPosition.norm());
      }
  }

  // On envoie cette liste à l'écran pour que l'utilisateur choisisse le tag qui servira de stylo
  envoyerListeTagsDecouverts(listeIds.data(), listeDistances.data(), listeIds.size());

  etatActuelHub = HUB_STATE_IDLE;
  Serial.println("[Machine Etats] Scan terminé. Liste envoyée. Retour en IDLE.");
}

void executer_HUB_STATE_COLLECTING_POINTS() {
  static unsigned long chronoLog = 0;
  if (millis() - chronoLog >= 2000) {
    Serial.printf("[Machine Etats] En cours d'acquisition de la zone d'exclusion\n");
    chronoLog = millis();
  }

  static unsigned long chronoAcquisition = 0;
  
  // Toutes les X millisecondes (FENETRE_MS), on prélève un point
  if (millis() - chronoAcquisition >= FENETRE_MS) { 
      std::vector<DistanceMoyennes> tagsLisses;
      std::vector<int> aIds = vAnchors.giveModuleIdList();

      if (recupDonnees.getDonneesLissees(tagsLisses)) {
          for (const auto& tag : tagsLisses) {
              
              // On ignore tous les tags, sauf celui sélectionné par l'utilisateur
              if (tag.tag_id == idTagSelectionne) {
                  std::unordered_map<int, float> distMap;
                  for(int i = 0; i < 4 && i < aIds.size(); i++) {
                      distMap[aIds[i]] = tag.distances[i];
                  }

                  // Calcul du point courant
                  V3 pos3D = trilateration3D(vAnchors, distMap);
                  
                  // On stocke le point en mémoire RAM par rapport à l'épicentre (temporaire)
                  calibManager.ajouterPoint(pos3D - calibManager.getEpicentre());
              }
          }
      }
      chronoAcquisition = millis();
  }
}

void executer_HUB_STATE_GENERATING_GEOMETRY() {
  Serial.printf("[Machine Etats] Étape : Génération géométrie pour le Tag cible #%d...\n", idTagSelectionne);

  if (calibManager.getNombrePoints() < 3) {
      Serial.println("[Erreur] Pas assez de points pour générer un plan mathématique.");
      etatActuelHub = HUB_STATE_IDLE;
      return;
  }

  const std::vector<V3>& pointsCollectes = calibManager.getPoints();
  
  // On passe le nuage de points à l'algorithme d'Analyse en Composantes Principales (ACP)
  LissageVehicule::PlanLocal planVehicule = LissageVehicule::calculerPlanMoyen(pointsCollectes);
  
  // On découpe la forme lissée en un contour précis de 64 points
  Polygone zone64 = LissageVehicule::echantillonner64Points(0, planVehicule, pointsCollectes);
  const std::vector<V3>& pts = zone64.getPoints();

  float ancresCalculees[4][2];
  std::vector<int> aIds = vAnchors.giveModuleIdList();
  
  // Vidange du dictionnaire des hauteurs pour éviter la pollution entre deux calibrations
  hauteursAncresTemporaires.clear(); 

  // Projection mathématique des Ancres (3D -> 2D)
  for (int i = 0; i < 4 && i < aIds.size(); i++) {
      int id = aIds[i];
      // On calcule le vecteur de l'ancre par rapport au centre du plan
      V3 posRelative = (vAnchors.getModule(id).getPosition() - calibManager.getEpicentre()) - planVehicule.centre;
      
      // On l'écrase sur l'axe U et V (X et Y virtuels)
      ancresCalculees[i][0] = prodScal(posRelative, planVehicule.axeU);
      ancresCalculees[i][1] = prodScal(posRelative, planVehicule.axeV);
      
      // /!\ IMPORTANT : On met de côté la hauteur Z (Normale) de l'ancre.
      // Cela permet à la trilatération finale de toujours marcher en 3D !
      hauteursAncresTemporaires[id] = prodScal(posRelative, planVehicule.normale);
  }

  // Projection mathématique des 64 points de la zone
  float zone64PointsCalculee[64][2];
  for (int i = 0; i < 64 && i < pts.size(); i++) {
      V3 posRelative = pts[i] - planVehicule.centre;
      zone64PointsCalculee[i][0] = prodScal(posRelative, planVehicule.axeU);
      zone64PointsCalculee[i][1] = prodScal(posRelative, planVehicule.axeV);
  }

  // On envoie le brouillon à l'écran. 
  // La tablette se chargera de tout glisser pour mettre le véhicule au centre.
  envoyerGeometrieCalibration(ancresCalculees, zone64PointsCalculee);

  calibManager.viderPoints();
  etatActuelHub = HUB_STATE_IDLE;
  Serial.println("[Machine Etats] Géométrie initiale envoyée. Retour en IDLE. (Attente Validation Utilisateur)");
}

void executer_HUB_STATE_IDLE() {
    // Le processeur se repose. Il attend que l'interruption matérielle de l'UART 
    // (depuis la tablette) modifie la variable etatActuelHub pour déclencher une action.
}

void reinitialiserObjetsMetierHub() {
  vSafeZone = Polygone(0, std::vector<V3>());
  Serial.println("[Main Hub] Configuration entièrement vidée de la RAM.");
}

void loop() {
  // Gère la réception des paquets JSON depuis la tablette
  loopScreenCommunication();

  // /!\ INDISPENSABLE POUR LE BUS CAN /!\
  // Contrairement au Wi-Fi, le CAN ne possède pas de fonction de "Callback automatique".
  // Nous devons donc interroger manuellement la carte à chaque milliseconde pour voir
  // si des ancres nous ont envoyé leurs calculs UWB.
  ecouterReseauFilaire();

  // Machine à états Switch/Case (Design Pattern State)
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
