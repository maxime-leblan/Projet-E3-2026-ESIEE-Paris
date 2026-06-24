#include "Config.hpp"
#include "HubDataStorage.hpp"
#include "GestionnaireAncres.hpp"
#include "InitAnchorPosition.hpp"
#include "CANMessageManager.hpp"
#include "RelaisBoutonBuzzer.hpp"

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

//#define DEBUG_POSITION

// --- VARIABLES GLOBALES ---
RecuperationDonneesAncres recupDonnees;
UWBModuleList vAnchors;
Polygone vSafeZone;
CalibrationManager calibManager;

// Dictionnaire pour retenir la hauteur (Z) des ancres avant que l'écran ne valide le X et Y
std::map<int, float> hauteursAncresTemporaires;

#define HAUTEUR_MAX_TAG_METRES 5.0f

#define TAG_CIBLE 4 // ID du tag que l'on souhaite suivre en temps réel (pour le polling du Hub)
#define ANCRE_MASTER 0

extern bool aDesDistancesManuelles;

// Prototypes
void afficherCoordonneesAncres();
void afficherCoordonneesTag(uint8_t tagId, const V3& pos3D);
void clearSerialMonitor();

/**
 * @brief Efface l'écran du moniteur série et replace le curseur en haut à gauche.
 * Note : Fonctionne parfaitement avec le moniteur de PlatformIO.
 */
void clearSerialMonitor() {
    // \033[2J : Code ANSI pour effacer tout l'écran
    // \033[H  : Code ANSI pour ramener le curseur à la position d'origine (0,0)
    Serial.print("\033[2J\033[H");
}

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

V3 getCoordonnesTag(uint8_t tagCible) {
    V3 pos3D(0, 0, 0); // Position de sécurité par défaut si l'ancre ne répond pas

    // 1. PURGE DU BUFFER CAN
    // On nettoie la file d'attente pour ne pas s'appuyer sur une ancienne trame par erreur
    twai_message_t vMessage;
    while(receiveCanMessage(vMessage)) { /* Vidage silencieux */ }

    // 2. ENVOI DE LA REQUÊTE
    Serial.printf("[HUB SYNC] Requête immédiate des distances pour le Tag %d à l'Ancre %d...\n", tagCible, ANCRE_MASTER);
    sendCanRequestDistances(ANCRE_MASTER, tagCible);

    // 3. ATTENTE ACTIVE (Bloquante avec Timeout de 150 ms)
    unsigned long startWait = millis();
    bool responseReceived = false;
    DecodedData donnees;

    while (millis() - startWait < 150) {
        if (receiveCanMessage(vMessage)) {
            if (decodeCanMessage(vMessage, donnees)) {
                
                // On s'assure d'intercepter la bonne réponse pour le bon tag
                if (donnees.type == MESSAGE_TAG_ID_AND_ALL_DISTANCES && donnees.id_tag == tagCible) {
                    responseReceived = true;
                    break; // On a notre tableau de distances, on sort immédiatement de la boucle !
                }
            }
        }
        delay(2); // Évite de faire surchauffer le processeur pendant l'écoute
    }

    // 4. TRAITEMENT MATHÉMATIQUE SI RÉPONSE REÇUE
    if (responseReceived && donnees.aDistances.size() >= 4) {
        Serial.printf("[HUB DATA] Distances reçues en direct : [%d, %d, %d, %d] cm\n", 
                      donnees.aDistances[0], donnees.aDistances[1], donnees.aDistances[2], donnees.aDistances[3]);

        std::vector<int> aIds = vAnchors.giveModuleIdList();
        std::unordered_map<int, float> distMap;

        // Conversion en mètres pour la bibliothèque géométrique
        for(int i = 0; i < 4 && i < aIds.size(); i++) {
            distMap[aIds[i]] = donnees.aDistances[i] / 100.0f; 
        }

        // Exécution de l'algorithme de trilatération
        initMatrixA(vAnchors); 
        pos3D = trilateration3D(vAnchors, distMap);
        afficherCoordonneesTag(TAG_CIBLE, pos3D);
        
        /* Contrôle de validité mathématique
        if (std::isnan(pos3D.getX()) || std::abs(pos3D.getZ()) > HAUTEUR_MAX_TAG_METRES) {
            Serial.println("[HUB TRILAT] Singularité mathématique (NaN) ou hors zone. Retour à l'origine.");
            afficherCoordonneesAncres();
            return V3(0, 0, 0);
        }*/

    } else {
        Serial.printf("[HUB DATA] Echec (Timeout) : Aucune réponse de l'ancre %d dans le temps imparti.\n", ANCRE_MASTER);
    }
    return pos3D;
    
}

float getDistanceToEpicentreFromTag(V3 pos3D){
    return std::sqrt(pos3D.getX() * pos3D.getX() + pos3D.getY() * pos3D.getY());
}

/**
 * Recupere la distance du tag cible et l'envoie à l'application
 * Des que l'utilisateur aura selectionne le tag, l'applicatino enverra une commande qui executera HUB_COLLECTING_POINT avec le bon tag
*/
void AskUserForTag() {
    Serial.println("Recupération de la distance à l'épicentre du tag cible pour l'etalonnage");
    
    V3 tag4_position = getCoordonnesTag(TAG_CIBLE);

    int ids[1] = {TAG_CIBLE};
    float distances[1] = {getDistanceToEpicentreFromTag(tag4_position)};
    int count = 1;
    envoyerListeTagsDecouverts(ids, distances, count);
    Serial.println("Distance envoyée à l'appplication");
    etatActuelHub = HUB_STATE_IDLE;
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
    afficherCoordonneesAncres();
    etatActuelHub = HUB_STATE_RUNNING;
}

void afficherCoordonneesAncres() {
    Serial.println("\n===================================================");
    Serial.println("           COORDONNÉES DES 4 ANCRES");
    Serial.println("===================================================");
    Serial.println(" ID  |   X (m)   |   Y (m)   |   Z (m)  ");
    Serial.println("---------------------------------------------------");

    std::vector<int> listeIds = vAnchors.giveModuleIdList();

    // On boucle sur les 4 ancres attendues du système
    for (int id : listeIds) {
        // Récupération de l'objet module et de sa position V3
        UWBModule ancre = vAnchors.getModule(id);
        V3 pos = ancre.getPosition();

        // Affichage aligné et propre
        Serial.printf(" [%d] |  %7.2f  |  %7.2f  |  %7.2f  \n", 
                      id, 
                      pos.getX(), 
                      pos.getY(), 
                      pos.getZ());
    }
    Serial.println("===================================================\n");
}

void afficherCoordonneesTag(uint8_t tagId, const V3& pos3D) {
    Serial.println("\n===================================================");
    Serial.printf("           COORDONNÉES DU TAG %d\n", tagId);
    Serial.println("===================================================");
    Serial.println(" ID  |   X (m)   |   Y (m)   |   Z (m)   ");
    Serial.println("---------------------------------------------------");

    // Affichage aligné avec le même formatage de flottants
    Serial.printf(" [%d] |  %7.2f  |  %7.2f  |  %7.2f  \n", 
                  tagId, 
                  pos3D.getX(), 
                  pos3D.getY(), 
                  pos3D.getZ());
                  
    Serial.println("===================================================\n");
}

void setup() {
    initRelaisBoutonBuzzer(); // Immédiat pour mettre le système sous tension
    
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n[Boot] --- Initialisation du Hub ---");

    initRamHub();
    setupScreenCommunication();
    initGestionnaireAncres();

    // Initialisation CAN (Le Wi-Fi a été retiré)
    initCan(HUB_RX_PIN, HUB_TX_PIN);
    Serial.println("[Boot] Bus CAN (Filaire) initialisé.");

    // Restauration de la mémoire Flash NVS

    // VRAI LIGNE DE CODE
    vAnchors = initAnchors("Anchors");
    // DEBUT CODE DE TEST
    // FIN CODE DE TEST

    vSafeZone = Polygone(0, initSafeZone("SafeZone"));

    String vText = String((vSafeZone.toString()).c_str());
    Serial.println("[EXTRACTION FLASH] Coordonnées deans la SafeZone extraites :\n" + vText);
 
    calibManager.initialiserEpicentre(vAnchors);
    
    // Démarrage automatique si la configuration existe
    #ifdef DEBUG_POSITION

    etatActuelHub = HUB_STATE_DEBUG_3D_MEASURE;
    Serial.println("Mode debug position activé. demarrage dans 2 secondes...");
    delay(2000);

    #else

    if (vSafeZone.getPoints().size() > 0 && vAnchors.size() == 4) {
        etatActuelHub = HUB_STATE_RUNNING;
        Serial.println("[Boot] Configuration existante trouvée. Passage en RUNNING.");
    } else {
        etatActuelHub = HUB_STATE_IDLE;
    }

    #endif

    Serial.println("[Boot] --- Fin Setup ---\n");

    Serial.println("Transformation des distances en coordonnées temporaires pour les ancres");

    initTestHardcodedAnchorsPosition(vAnchors);

}

void executer_HUB_STATE_RUNNING() {

        V3 pos3D = getCoordonnesTag(TAG_CIBLE);   

        if (std::abs(pos3D.getZ()) > HAUTEUR_MAX_TAG_METRES) {
            Serial.printf("[HUB TRILAT ERREUR] Calcul rejeté ! Z (%.2fm) dépasse la limite max (%.2fm).\n", 
                          std::abs(pos3D.getZ()), HAUTEUR_MAX_TAG_METRES);
            return;
        }

        Serial.printf("[HUB MATHS] Position 2D projetée pour le Tag %d : X=%.2f, Y=%.2f\n", 
                      TAG_CIBLE, pos3D.getX(), pos3D.getY());

        // ÉVALUATION DANGER
        Serial.println("[HUB ZONE] Évaluation du danger par rapport à la SafeZone...");
        V3 posProjectee2D(pos3D.getX(), pos3D.getY(), 0.0f);
        bool inDanger = vSafeZone.isInside(posProjectee2D);
        float distSafeZone = vSafeZone.getDistanceFrom(posProjectee2D);
        float distCentreVehicule = getDistanceToEpicentreFromTag(pos3D);

        Serial.printf("[HUB ZONE] Résultat Tag %d -> Danger: %s | Dist. SafeZone: %.2fm | Dist. Centre: %.2fm\n", 
                      TAG_CIBLE, inDanger ? "OUI" : "NON", distSafeZone, distCentreVehicule);

        Serial.printf("\n[HUB MATHS] Tag %d à %.2fm de la SafeZone. Envoi alerte CAN à Ancre %d.\n", 
                      TAG_CIBLE, distSafeZone, ANCRE_MASTER);

        // RETOUR D'INFORMATION (Alerte CAN)
        Serial.printf("[HUB ALERT] Envoi du message CAN (distance: %.2fm) en cours...\n", distSafeZone);
        sendCanDistance(ANCRE_MASTER, TAG_CIBLE, distSafeZone);
        
        if (inDanger) {
            Serial.println("[HUB ALERT DANGER] *** VIOLATION DE LA SAFEZONE ! ACTIVATION DU BUZZER ***");
            faireSonnerBuzzer(1000); // 1 seconde
        }

        // Mise à jour de l'écran (Tableaux C pour l'interface de ScreenCommunicationManager)
        int ids[1] = {TAG_CIBLE};
        float xs[1] = {posProjectee2D.getX()};
        float ys[1] = {posProjectee2D.getY()};
        float distsScreen[1] = {distCentreVehicule};
        bool alarmes[1] = {inDanger};
        Serial.printf("[HUB IHM] Préparation de l'envoi des données au Screen (Tag %d, X=%.2f, Y=%.2f, DistCentre=%.2f, Danger=%s)\n", 
                      TAG_CIBLE, posProjectee2D.getX(), posProjectee2D.getY(), distCentreVehicule, inDanger ? "OUI" : "NON");
        
        envoyerMiseAJourTagsRuntime(ids, xs, ys, distsScreen, alarmes, 1);
}

void executer_HUB_STATE_DETECTING_TAGS_FOR_INIT() {
    Serial.println("[Hub] Lancement de l'auto-calibration matérielle des ancres...");
   
    
    if (aDesDistancesManuelles) {
        initTestHardcodedAnchorsPosition(vAnchors);
    } else {
        // Cette fonction gère son propre timeout/blocage CAN dans InitAnchorPosition.cpp
        initAnchorsPosition(vAnchors);
        // /!\ La fonction ne fonctionnant pas, on va hardcoder les positions des ancres à la place
    }
    
    Serial.println("[Hub INIT ANCRES] Voici les coordonnées de base des ancres : " + String(vAnchors.toString().c_str()));
   
    // Changement de repère officiel
    alignAnchorsCoordinatesWithGridOrigin(vAnchors);
    Serial.println("[Hub INIT ANCRES] Coordonnées après translation de l'épicentre vers (0, 0, 0) : " + String(vAnchors.toString().c_str()));
    calibManager.viderPoints();

    AskUserForTag();

    Serial.println("[Hub] Initialisation terminée. Passage en collecte de points pour géométrie.");
    afficherCoordonneesAncres();
}

void executer_HUB_STATE_COLLECTING_POINTS() {
    V3 pos3D = getCoordonnesTag(TAG_CIBLE);
    calibManager.ajouterPoint(pos3D);
   /*  static unsigned long lastPollTimeCalib = 0;
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
    } */
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
    Serial.println("[Machine Etats] Géométrie transmise. Retour en IDLE.");
    afficherCoordonneesAncres();
    etatActuelHub = HUB_STATE_IDLE;
}

void executer_HUB_STATE_DEBUG_3D_MEASURE() {
    clearSerialMonitor();

    V3 pos3D = getCoordonnesTag(TAG_CIBLE);

    afficherCoordonneesAncres();
    delay(500);
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
        case HUB_STATE_DEBUG_3D_MEASURE:
            executer_HUB_STATE_DEBUG_3D_MEASURE();
            break;
    }
}