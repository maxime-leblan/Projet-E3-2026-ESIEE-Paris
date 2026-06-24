#include "InitAnchorPosition.hpp"
#include "Config.hpp"
#include "RecuperationDonneesAncres.hpp"
#include <Arduino.h>
#include <algorithm>

extern RecuperationDonneesAncres recupDonnees;
extern void ecouterReseauFilaire();

void setAnchorRole(uint8_t pAnchorId, uint8_t pRoleOrder)
{
    MsgAnchorCalibHubOrder vMessage = {pAnchorId};
    sendCanOrderFromHubTo(pAnchorId, pRoleOrder, vMessage);
    Serial.printf("[HUB CALIB] Ordre de role (Ordre: %d) envoye a l'ancre %d\n", pRoleOrder, pAnchorId);
}

void sendToAnchorsInitialisationPhaseSignal(UWBModuleList & pAnchors, uint8_t pSignalType)
{
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();
    for (uint8_t vCurrentId : vAnchorsId)
    {
        MsgAnchorCalibHubOrder vMessage = {vCurrentId};
        sendCanOrderFromHubTo(vCurrentId, pSignalType, vMessage);
    }
}

void initAnchorsPosition(UWBModuleList & pAnchors)
{
    Serial.println("\n[HUB CALIB] === DEBUT DU PROTOCOLE DE CALIBRATION SPATIALE ===");
    sendToAnchorsInitialisationPhaseSignal(pAnchors, HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL);
    delay(500);

    std::unordered_map<std::string, float> vAnchorDistances;
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();

    // Application stricte de la règle : on présume que les 4 modules sont déjà en Ancres.
    Serial.println("[HUB CALIB] Les 4 modules sont par defaut des ANCRES. Demarrage de la rotation.");

    for (int idCibleTag : vAnchorsId)
    {
        Serial.printf("\n[HUB CALIB] --- Phase: L'Ancre %d devient le TAG temporaire ---\n", idCibleTag);

        // 1. ISOLEMENT : On ne modifie QUE la cible
        setAnchorRole(idCibleTag, HUB_ORDER_SET_AS_TAG);
        
        Serial.println("[HUB CALIB] Attente du reboot complet de la puce UWB (5.0s)...");
        delay(5000); 

        recupDonnees.effacerDonneesTag(idCibleTag);

        Serial.println("[HUB CALIB] Purge des anciens messages CAN en attente...");
        twai_message_t dummyMsg;
        while(twai_receive(&dummyMsg, 0) == ESP_OK) {
            //Vidange
        }

        // 2. COLLECTE RALENTIE : Boucle de 5 secondes avec espacement massif des requêtes
        unsigned long startTime = millis();
        while (millis() - startTime < 5000)
        {
            // Interrogation des 3 modules restés Ancres
            for (int idAncre : vAnchorsId)
            {
                if (idAncre != idCibleTag) {
                    sendCanRequestDistances(idAncre, idCibleTag);
                    delay(100); // 100ms de pause stricte entre chaque requête filaire
                }
            }

            ecouterReseauFilaire();

            DistanceMoyennes tagMoyenne;
            if (recupDonnees.getDonneesLisseesPourTag(idCibleTag, tagMoyenne))
            {
                for (int idAncre : vAnchorsId)
                {
                    if (idAncre != idCibleTag)
                    {
                        float dist = tagMoyenne.distances[idAncre];
                        if (dist > 0.0f)
                        {
                            Serial.printf("[HUB DEBUG] En direct : Ancre %d <---> Tag Temporaire %d = %.2f cm\n", idAncre, idCibleTag, dist);

                            int minId = std::min(idAncre, idCibleTag);
                            int maxId = std::max(idAncre, idCibleTag);
                            std::string vKey = std::to_string(minId + 1) + std::to_string(maxId + 1);

                            if (vAnchorDistances.find(vKey) == vAnchorDistances.end() || vAnchorDistances[vKey] == 0) {
                                vAnchorDistances[vKey] = dist;
                                Serial.printf("[HUB CALIB] Succes : Distance validee %s -> %.2f cm\n", vKey.c_str(), dist);
                            }
                        }
                    }
                }
            }
            delay(200); // Pause globale supplémentaire pour laisser respirer le microcontrôleur
        }

        // 3. RESTAURATION : On ne restaure QUE la cible
        Serial.printf("[HUB CALIB] Fin de collecte. L'Ancre %d redevient ANCRE.\n", idCibleTag);
        setAnchorRole(idCibleTag, HUB_ORDER_SET_AS_ANCHOR);
        
        Serial.println("[HUB CALIB] Attente du retour physique en mode Ancre (5.0s)...");
        delay(5000);
    }

    Serial.println("\n[HUB CALIB] Fin de la collecte des distances inter-ancres.");
    delay(500);

    sendToAnchorsInitialisationPhaseSignal(pAnchors, HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL);

    if (vAnchorDistances.size() < 6) {
        Serial.printf("[HUB ERREUR CRITIQUE] Calibration avortee ! Seulement %d distances captees sur les 6 requises.\n", vAnchorDistances.size());
        return; 
    }

    Serial.println("[HUB CALIB] Traitement mathématique (Descente de Gradient) en cours...");
    initAnchorsCoordinatesWithGD(pAnchors, vAnchorDistances, ITERATIONS, LEARNING_RATE);
    
    Serial.println("[HUB CALIB] === FIN DU PROTOCOLE DE CALIBRATION ===");
}

