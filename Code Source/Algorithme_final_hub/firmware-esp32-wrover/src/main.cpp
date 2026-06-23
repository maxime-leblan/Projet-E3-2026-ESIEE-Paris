#include "Config.hpp"
#include "HubDataStorage.hpp"
#include "GestionnaireAncres.hpp"
#include "InitAnchorPosition.hpp"

// ---- LIBRAIRIE CAN ----
#include "CANMessageManager.hpp"

#include <ArduinoEigenDense.h>
#include <ArduinoJson.h>
#include <cmath>

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
 * @brief Fonction vitale pour le réseau CAN.
 * Doit être appelée à chaque cycle de loop() pour vider le buffer matériel TWAI
 * et injecter les distances reçues des ancres dans l'accumulateur.
 */
void ecouterReseauFilaire() {
    twai_message_t messageRecu;
   
    while (receiveCanMessage(messageRecu)) {
        DecodedData donnees;

        digitalWrite(2, LOW);
       
        if (decodeCanMessage(messageRecu, donnees)) {
            if (donnees.type == MESSAGE_TAG_ID_AND_ALL_DISTANCES) {
                if (donnees.aDistances.size() >= 4) {
                    
                    // --- NOUVEAU LOG : Réception des distances depuis l'Ancre ---
                    Serial.printf("[HUB CAN RX] Distances du Tag %d recues via Ancre %d : [%d, %d, %d, %d]\n", 
                                  donnees.id_tag, donnees.id_ancre,
                                  donnees.aDistances[0], donnees.aDistances[1],
                                  donnees.aDistances[2], donnees.aDistances[3]);

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

// Application de la configuration reçue de l'IHM Écran
void appliquerNouvelleConfigurationMaterielle(JsonArray zoneJson, JsonArray sensorsJson) {
    Serial.println("\n[Main Hub] --- Réception de la configuration validée par l'écran ---");
   
    // 1. MISE À JOUR DE LA ZONE DE SÉCURITÉ
    std::vector<V3> nouveauxSommets;
    for (JsonVariant v : zoneJson) {
        nouveauxSommets.push_back(V3(v["x"].as<float>(), v["y"].as<float>(), 0.0f));
    }
   
    vSafeZone = Polygone(0, nouveauxSommets);
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
            float sz = vAnchors.getModule(id).getPosition().getZ();
           
            V3 nouvellePos(sx, sy, sz);
           
            vAnchors.setModulePosition(id, nouvellePos);
            mapAncresPourFlash[id] = nouvellePos;
        }
        anchorsCount++;
    }
   
    saveMapData("Anchors", "Positions", mapAncresPourFlash);
    etatActuelHub = HUB_STATE_RUNNING;
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n[Boot] --- Initialisation du Hub ---");

    pinMode(2, OUTPUT);

    initHub();
    setupScreenCommunication();
    initGestionnaireAncres();

    // Initialisation CAN (Le Wi-Fi a été retiré)
    initCan(HUB_RX_PIN, HUB_TX_PIN);
    Serial.println("[Boot] Bus CAN (Filaire) initialisé.");

    // Restauration de la mémoire Flash NVS

    // VRAI LIGNE DE CODE
    // vAnchors = initAnchors("Anchors");

    // DEBUT CODE DE TEST
    vector<V3> vSensorsPosition = {V3(3, 0, 1), V3(0, 3, 0), V3(3, 6, 0), V3(6, 3, 0)};

    // on les ajoute dans la liste des capteurs
    for (int i = 1; i <= ANCHORS_NUMBER; i++)
    {
        UWBModule vTemp = UWBModule(i, vSensorsPosition[i - 1]);
        vAnchors.addModule(vTemp.getId(), vTemp);
    }
    // FIN CODE DE TEST

    vSafeZone = Polygone(0, initSafeZone("SafeZone"));
 
    calibManager.initialiserEpicentre(vAnchors);
 
    // Démarrage automatique si la configuration existe
    if (vSafeZone.getPoints().size() > 0 && vAnchors.size() == 4) {
        etatActuelHub = HUB_STATE_RUNNING;
        Serial.println("[Boot] Configuration existante trouvée. Passage en RUNNING.");
    } else {
        etatActuelHub = HUB_STATE_IDLE;
    }

    Serial.println("[Boot] --- Fin Setup ---\n");
}

void executer_HUB_STATE_RUNNING() {
    static unsigned long lastPollTimeRunning = 0;
    const unsigned long POLL_INTERVAL = 20; // 50 Hz
    uint8_t ancreCible = 0; // L'ancre désignée pour renvoyer la donnée
    
    uint8_t tagCible = 4;   

    // 1. POLLING CYCLIQUE NON-BLOQUANT
    if (millis() - lastPollTimeRunning >= POLL_INTERVAL) {
        Serial.printf("[HUB POLL] [%lu] Envoi requête CAN (Ancre: %d, Cible Tag: %d)\n", millis(), ancreCible, tagCible);
        sendCanRequestDistances(ancreCible, tagCible);
        lastPollTimeRunning = millis();
    }

    // 2. RÉCUPÉRATION ET TRAITEMENT
    DistanceMoyennes tagMoyenne;
    
    if (recupDonnees.getDonneesLisseesPourTag(tagCible, tagMoyenne)) {
        Serial.printf("[HUB DATA] Succès : Données lissées récupérées pour le Tag %d.\n", tagCible);
        
        std::vector<int> aIds = vAnchors.giveModuleIdList();
        std::unordered_map<int, float> distMap;

        Serial.printf("[HUB MATHS] Distances lissées (cm) pour le Tag %d : [%.2f, %.2f, %.2f, %.2f]\n", 
                      tagMoyenne.tag_id,
                      tagMoyenne.distances[0],
                      tagMoyenne.distances[1],
                      tagMoyenne.distances[2],
                      tagMoyenne.distances[3]);
        
        Serial.printf("[HUB MATHS] Nombre d'ancres trouvées en mémoire (vAnchors) : %d\n", aIds.size());

        for(int i = 0; i < 4 && i < aIds.size(); i++) {
            distMap[aIds[i]] = tagMoyenne.distances[i]/100.0f; // Conversion en mètres
        }

        Serial.printf("[HUB MATHS] Distances mappées (mètres) pour le Tag %d : [", tagMoyenne.tag_id);
        for (const auto& pair : distMap) {
            Serial.printf("Ancre %d: %.2fm, ", pair.first, pair.second);
        }
        Serial.println("]");

        // TRILATÉRATION
        // On initialise la matrice A nécessaire au calcul
        Serial.println("[HUB TRILAT] Début de l'initialisation de la Matrice A...");
        initMatrixA(vAnchors);
        
        Serial.printf("[HUB TRILAT] État actuel des ancres : %s\n", vAnchors.toString().c_str());

        Serial.println("[HUB TRILAT] Lancement du calcul trilateration3D...");
        V3 pos3D = trilateration3D(vAnchors, distMap);

        Serial.printf("[HUB MATHS] Position 3D calculée pour le Tag %d : X=%.2f, Y=%.2f, Z=%.2f\n", 
                      tagMoyenne.tag_id, pos3D.getX(), pos3D.getY(), pos3D.getZ());

        if (std::abs(pos3D.getZ()) > HAUTEUR_MAX_TAG_METRES) {
            Serial.printf("[HUB TRILAT ERREUR] Calcul rejeté ! Z (%.2fm) dépasse la limite max (%.2fm).\n", 
                          std::abs(pos3D.getZ()), HAUTEUR_MAX_TAG_METRES);
            return;
        }

        Serial.printf("[HUB MATHS] Position 2D projetée pour le Tag %d : X=%.2f, Y=%.2f\n", 
                      tagMoyenne.tag_id, pos3D.getX(), pos3D.getY());

        // ÉVALUATION DANGER
        Serial.println("[HUB ZONE] Évaluation du danger par rapport à la SafeZone...");
        V3 posProjectee2D(pos3D.getX(), pos3D.getY(), 0.0f);
        bool inDanger = vSafeZone.isInside(posProjectee2D);
        float distSafeZone = vSafeZone.getDistanceFrom(posProjectee2D);
        float distCentreVehicule = std::sqrt(pos3D.getX() * pos3D.getX() + pos3D.getY() * pos3D.getY());

        Serial.printf("[HUB ZONE] Résultat Tag %d -> Danger: %s | Dist. SafeZone: %.2fm | Dist. Centre: %.2fm\n", 
                      tagMoyenne.tag_id, inDanger ? "OUI" : "NON", distSafeZone, distCentreVehicule);

        Serial.printf("\n[HUB MATHS] Tag %d à %.2fm de la SafeZone. Envoi alerte CAN à Ancre %d.\n", 
                      tagMoyenne.tag_id, distSafeZone, ancreCible);

        // RETOUR D'INFORMATION (Alerte CAN)
        Serial.printf("[HUB ALERT] Envoi du message CAN (distance: %.2fm) en cours...\n", distSafeZone);
        sendCanDistance(ancreCible, tagMoyenne.tag_id, distSafeZone);
        
        if (distSafeZone <= 0) {
            Serial.println("[HUB ALERT DANGER] *** VIOLATION DE LA SAFEZONE ! ACTIVATION DU BUZZER ***");
            tone(BUZZER_GPIO_PIN, BUZZER_FREQUENCY, 1000);
        }

        // Mise à jour de l'écran (Tableaux C pour l'interface de ScreenCommunicationManager)
        int ids[1] = {tagMoyenne.tag_id};
        float xs[1] = {posProjectee2D.getX()};
        float ys[1] = {posProjectee2D.getY()};
        float distsScreen[1] = {distCentreVehicule};
        bool alarmes[1] = {inDanger};
        
        static unsigned long chronoRuntime = 0;
        if (millis() - chronoRuntime >= 33) { // Limitation du rafraîchissement écran (30 FPS max)
            Serial.printf("[HUB IHM] [%lu] Rafraîchissement de l'écran déclenché (FPS respecté).\n", millis());
            envoyerMiseAJourTagsRuntime(ids, xs, ys, distsScreen, alarmes, 1);
            chronoRuntime = millis();
        } else {
            // (Optionnel) Dé-commente cette ligne si tu veux voir quand l'écran skippe une frame pour maintenir les performances
            // Serial.println("[HUB IHM] Rafraîchissement écran ignoré (limite 30 FPS).");
        }
    } else {
        // --- LOG ANTI-FLOOD SI AUCUNE DONNÉE ---
        // Permet de savoir si le système tourne à vide sans spammer la console 50 fois par seconde.
        static unsigned long lastNoDataLog = 0;
        if (millis() - lastNoDataLog > 1000) {
            Serial.printf("[HUB DATA ALERTE] Aucune donnée lissée disponible pour le Tag %d en ce moment...\n", tagCible);
            lastNoDataLog = millis();
        }
    }
}

void executer_HUB_STATE_DETECTING_TAGS_FOR_INIT() {
    Serial.println("[Hub] Lancement de l'auto-calibration matérielle des ancres...");
   
    // Cette fonction gère son propre timeout/blocage CAN dans InitAnchorPosition.cpp
    initAnchorsPosition(vAnchors);
   
    // Changement de repère officiel
    alignAnchorsCoordinatesWithGridOrigin(vAnchors);
    calibManager.viderPoints();
   
    // Configuration pour l'étape suivante
    idTagSelectionne = 4;
   
    Serial.println("[Hub] Initialisation terminée. Passage en collecte de points pour géométrie.");
    etatActuelHub = HUB_STATE_COLLECTING_POINTS;
}

void executer_HUB_STATE_COLLECTING_POINTS() {
    static unsigned long lastPollTimeCalib = 0;
    const unsigned long POLL_INTERVAL = 20;

    // 1. POLLING POUR LA CALIBRATION
    if (millis() - lastPollTimeCalib >= POLL_INTERVAL) {
        // Demande des distances du Tag d'étalonnage à l'Ancre 0
        sendCanRequestDistances(0, idTagSelectionne);
        lastPollTimeCalib = millis();
    }

    // 2. RÉCUPÉRATION ET TRAITEMENT
    DistanceMoyennes tagMoyenne;
    
    // On extrait spécifiquement le Tag que l'ouvrier tient en main
    if (recupDonnees.getDonneesLisseesPourTag(idTagSelectionne, tagMoyenne)) {
        
        std::unordered_map<int, float> dists;
        for (int i = 0; i < 4; i++) {
            if (tagMoyenne.distances[i] > 0.0f) {
                dists[i] = tagMoyenne.distances[i]; 
            }
        }

        // Il faut au moins 3 distances valides pour un calcul 3D viable
        if (dists.size() >= 3) {
            V3 pos3D = trilateration3D(vAnchors, dists);
            calibManager.ajouterPoint(pos3D);
        }
    }
}

void executer_HUB_STATE_GENERATING_GEOMETRY() {
    Serial.printf("[Machine Etats] Génération de la géométrie pour le Tag cible #%d...\n", idTagSelectionne);

    if (calibManager.getNombrePoints() < 3) {
        Serial.println("[Erreur] Nombre de points insuffisant pour le plan mathématique.");
        etatActuelHub = HUB_STATE_IDLE;
        return;
    }

    const std::vector<V3>& pointsCollectes = calibManager.getPoints();
   
    LissageVehicule::PlanLocal planVehicule = LissageVehicule::calculerPlanMoyen(pointsCollectes);
    Polygone zone64 = LissageVehicule::echantillonner64Points(0, planVehicule, pointsCollectes);
    
    // Changement de repère de la zone
    std::vector<V3> pointsChangeRepere = changeCoordinateSystem(zone64.getPoints(), planVehicule.normale);

    // Changement de repère des Ancres
    std::vector<int> aIds = vAnchors.giveModuleIdList();
    std::vector<V3> positionsAncres;
    for (int id : aIds) {
        positionsAncres.push_back(vAnchors.getModule(id).getPosition());
    }
    std::vector<V3> ancresChangeRepere = changeCoordinateSystem(positionsAncres, planVehicule.normale);

    // Enregistrement des nouvelles coordonnées en RAM
    for (size_t i = 0; i < aIds.size(); i++) {
        vAnchors.setModulePosition(aIds[i], ancresChangeRepere[i]);
    }

    // Préparation pour l'écran (2D)
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

    // Envoi de la géométrie via UART
    envoyerGeometrieCalibration(ancresCalculees, zone64PointsCalculee);

    calibManager.viderPoints();
    etatActuelHub = HUB_STATE_IDLE;
    Serial.println("[Machine Etats] Géométrie transmise. Retour en IDLE.");
}

void executer_HUB_STATE_IDLE() {
    // Repos du Hub. Modifié uniquement par interruption UART depuis l'IHM.
}

void reinitialiserObjetsMetierHub() {
    vSafeZone = Polygone(0, std::vector<V3>());
    Serial.println("[Main Hub] Configuration vidée de la RAM.");
}

void loop() {
    // Gestion de l'UART
    loopScreenCommunication();

    // INTERCEPTION CAN OBLIGATOIRE
    // Indispensable au fonctionnement du polling, charge les données de manière asynchrone.
    ecouterReseauFilaire();

    // Design Pattern State
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