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

#define HAUTEUR_MAX_TAG_METRES 5.0f
#define BUZZER_GPIO_PIN 4
#define BUZZER_FREQUENCY 600

// --- VARIABLES GLOBALES ---
RecuperationDonneesAncres recupDonnees;
UWBModuleList vAnchors;
Polygone vSafeZone;
CalibrationManager calibManager;

// Dictionnaire pour retenir la hauteur (Z) des ancres avant que l'écran ne valide le X et Y
std::map<int, float> hauteursAncresTemporaires;



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

            // on allume la LED
            digitalWrite(2, LOW);
            
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
            float sx = s["x"].as<float>(); 
            float sy = s["y"].as<float>(); 
            
            // CORRECTION : On récupère la vraie hauteur Z qui est DÉJÀ en mémoire grâce à la rotation
            float sz = vAnchors.getModule(id).getPosition().getZ();
            
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

  // on configure la LED du hub
  pinMode(2, OUTPUT);

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
    // ====================================================================
    // 1. POLLING CYCLIQUE (Le Hub dicte le tempo)
    // ====================================================================
    static unsigned long lastPollTime = 0;
    const unsigned long POLL_INTERVAL = 20; // Fréquence d'interrogation de 50 Hz

    if (millis() - lastPollTime >= POLL_INTERVAL) {
        // Pour ce test unitaire : On cible exclusivement l'Ancre 0 pour le Tag 0
        uint8_t ancreCible = 0;
        uint8_t tagCible = 0;
        
        sendCanRequestDistances(ancreCible, tagCible);
        lastPollTime = millis();
    }

    // ====================================================================
    // 2. RÉCEPTION ET TRAITEMENT ("Dès la réception des 4 distances")
    // ====================================================================
    std::vector<DistanceMoyennes> tagsPretsPourMaths;

    // La fonction getDonneesLissees() fait exactement ce que tu as demandé :
    // Elle regroupe instantanément les distances reçues et établit la liste 
    // des tags éligibles à la trilatération à la fin de la fenêtre de temps.
    if (recupDonnees.getDonneesLissees(tagsPretsPourMaths)) {
        
        int ids[MAX_TAGS];
        float xs[MAX_TAGS], ys[MAX_TAGS], distsScreen[MAX_TAGS];
        bool alarmes[MAX_TAGS];
        int tagCount = 0;
        std::vector<int> aIds = vAnchors.giveModuleIdList();

        for (const DistanceMoyennes& tag : tagsPretsPourMaths) {
            
            // Sécurité pour le test : on s'assure qu'on ne traite que le Tag 0
            if (tag.tag_id != 0) continue;

            std::unordered_map<int, float> distMap;
            for(int i = 0; i < 4 && i < aIds.size(); i++) {
                distMap[aIds[i]] = tag.distances[i];
            }

            // TRILATÉRATION
            V3 pos3D = trilateration3D(vAnchors, distMap);

            if (std::abs(pos3D.getZ()) > HAUTEUR_MAX_TAG_METRES) {
                continue; 
            }

            // ÉVALUATION DANGER
            V3 posProjectee2D(pos3D.getX(), pos3D.getY(), 0.0f);
            bool inDanger = vSafeZone.isInside(posProjectee2D);
            float distSafeZone = vSafeZone.getDistanceFrom(posProjectee2D);
            float distCentreVehicule = std::sqrt(pos3D.getX() * pos3D.getX() + pos3D.getY() * pos3D.getY());

            Serial.printf("[HUB MATHS] Tag %d positionne en X:%.2f, Y:%.2f -> DistZone: %.2f m\n", 
                          tag.tag_id, posProjectee2D.getX(), posProjectee2D.getY(), distSafeZone);

            // ====================================================================
            // 3. RETOUR D'INFORMATION (Renvoi de l'alerte à l'ancre par CAN)
            // ====================================================================
            if (MODE_ACTUEL == MODE_WIRED) {
                uint8_t ancreRelais = 0; // On demande à l'Ancre 0 de relayer le message radio
                
                // Envoi de la trame CAN contenant l'ID du tag et sa distance au danger
                sendCanDistance(ancreRelais, tag.tag_id, distSafeZone);
                Serial.printf("[HUB TX] Distance de securite renvoyee par CAN a l'Ancre %d\n", ancreRelais);

                if (distSafeZone <= 0) {
                    tone(BUZZER_GPIO_PIN, BUZZER_FREQUENCY, 1000);
                }
            }

            // Stockage pour l'écran
            if (tagCount < MAX_TAGS) {
                ids[tagCount] = tag.tag_id;
                xs[tagCount] = posProjectee2D.getX();
                ys[tagCount] = posProjectee2D.getY();
                distsScreen[tagCount] = distCentreVehicule; 
                alarmes[tagCount] = inDanger;
                tagCount++;
            }
        }

        // Mise à jour de l'écran 
        static unsigned long chronoRuntime = 0;
        if (millis() - chronoRuntime >= FENETRE_MS && tagCount > 0) {
            envoyerMiseAJourTagsRuntime(ids, xs, ys, distsScreen, alarmes, tagCount);
            chronoRuntime = millis();
        }
    }
}

void executer_HUB_STATE_DETECTING_TAGS_FOR_INIT() {
    Serial.println("[Hub] Lancement de l'auto-calibration des ancres...");
    
    // 1) Auto-calibration des 4 ancres par descente de gradient
    initAnchorsPosition(vAnchors);
    
    // 2) VRAI CHANGEMENT DE REPÈRE : L'origine (0,0,0) devient le milieu des ancres.
    // Cette fonction de GridLibrary translate officiellement et définitivement 
    // les coordonnées des 4 ancres dans la RAM.
    alignAnchorsCoordinatesWithGridOrigin(vAnchors);
    
    // On s'assure que le tableau de points de calibration est bien vide
    calibManager.viderPoints();
    
    // 3) Solution de contournement Prototype : On force le Tag 4
    idTagSelectionne = 4;
    
    Serial.println("[Hub] Initialisation terminée. Repère centré sur (0,0,0).");
    Serial.println("[Hub] Tag 4 sélectionné par défaut. Passage automatique en collecte de points.");
    
    // On bascule directement à l'étape suivante sans attendre l'écran
    etatActuelHub = HUB_STATE_COLLECTING_POINTS;
}

void executer_HUB_STATE_COLLECTING_POINTS() {
    // Cette fonction boucle en permanence. On sortira de cet état uniquement 
    // lorsque l'écran enverra la commande UART "stop_calib" (géré dans loopScreenCommunication).

    std::vector<DistanceMoyennes> tagsLisses;
    
    // On récupère les données lissées (buffer temporel de 1/30s)
    if (recupDonnees.getDonneesLissees(tagsLisses)) {
        
        for (const auto& tag : tagsLisses) {
            // On ne s'intéresse qu'au tag que l'ouvrier tient en main (le Tag 4)
            if (tag.tag_id == idTagSelectionne) { 
                
                // On prépare le dictionnaire pour la trilatération
                std::unordered_map<int, float> dists;
                for (int i = 0; i < 4; i++) {
                    if (tag.distances[i] > 0.0f) {
                        dists[i] = tag.distances[i]; // L'index 'i' correspond à l'ID de l'ancre (0 à 3)
                    }
                }

                // Il faut au moins 3 distances valides pour calculer une position 3D
                if (dists.size() >= 3) {
                    
                    // Calcul de la position 3D du Tag.
                    // IMPORTANT : Comme vAnchors a été translaté à l'étape d'avant, 
                    // pos3D est DÉJÀ dans le bon repère centré !
                    V3 pos3D = trilateration3D(vAnchors, dists);
                    
                    // On enregistre directement le point pur. Fini le bricolage !
                    calibManager.ajouterPoint(pos3D);
                    
                    // Optionnel : Un petit print pour voir qu'on enregistre bien
                    Serial.printf("[Collecte] Point ajouté : X=%.2f, Y=%.2f, Z=%.2f (Total: %d points)\n", 
                                  pos3D.getX(), pos3D.getY(), pos3D.getZ(), calibManager.getNombrePoints());
                }
            }
        }
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
    
    // 1. Calcul du plan incliné à partir du nuage de points
    LissageVehicule::PlanLocal planVehicule = LissageVehicule::calculerPlanMoyen(pointsCollectes);
    Polygone zone64 = LissageVehicule::echantillonner64Points(0, planVehicule, pointsCollectes);
    const std::vector<V3>& pts = zone64.getPoints();

    // --- CORRECTION : VRAI CHANGEMENT DE REPÈRE ---
    // 2. On change le repère des 64 points en passant la normale du plan 
    // comme vecteur de base pour le nouveau repère
    std::vector<V3> pointsChangeRepere = changeCoordinateSystem(pts, planVehicule.normale);

    // 3. On applique le MÊME changement de repère aux 4 ancres pour qu'elles suivent
    std::vector<int> aIds = vAnchors.giveModuleIdList();
    std::vector<V3> positionsAncres;
    for (int id : aIds) {
        positionsAncres.push_back(vAnchors.getModule(id).getPosition());
    }
    std::vector<V3> ancresChangeRepere = changeCoordinateSystem(positionsAncres, planVehicule.normale);

    // 4. On met à jour la mémoire RAM du Hub avec ces nouvelles coordonnées
    // /!\ TRÈS IMPORTANT : On conserve le Z (ex: Z=0.7) ! On n'aplatit rien.
    for (size_t i = 0; i < aIds.size(); i++) {
        vAnchors.setModulePosition(aIds[i], ancresChangeRepere[i]);
    }

    // --- PRÉPARATION POUR L'ÉCRAN ---
    // La fonction envoyerGeometrieCalibration de l'UART attend des tableaux 2D.
    // On extrait donc uniquement les X et Y du nouveau repère pour l'affichage.
    float ancresCalculees[4][2];
    for (size_t i = 0; i < 4 && i < aIds.size(); i++) {
        ancresCalculees[i][0] = ancresChangeRepere[i].getX();
        ancresCalculees[i][1] = ancresChangeRepere[i].getY();
    }

    float zone64PointsCalculee[64][2];
    for (size_t i = 0; i < 64 && i < pointsChangeRepere.size(); i++) {
        zone64PointsCalculee[i][0] = pointsChangeRepere[i].getX();
        zone64PointsCalculee[i][1] = pointsChangeRepere[i].getY();
    }

    // Envoi de la géométrie au format 2D pour la tablette
    envoyerGeometrieCalibration(ancresCalculees, zone64PointsCalculee);

    calibManager.viderPoints();
    etatActuelHub = HUB_STATE_IDLE;
    Serial.println("[Machine Etats] Géométrie initiale (avec changement de repère) envoyée. Retour en IDLE.");
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
