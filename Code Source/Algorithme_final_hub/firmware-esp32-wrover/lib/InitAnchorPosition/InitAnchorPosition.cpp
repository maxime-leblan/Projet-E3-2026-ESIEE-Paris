#include "InitAnchorPosition.hpp"
#include "Config.hpp"
#include "RecuperationDonneesAncres.hpp"
#include <Arduino.h>
#include <algorithm> // Pour std::remove

std::unordered_map<std::string, float> getAnchorDistances(int pStaticAnchorId, UWBModuleList pAnchors)
{
    Serial.println("[INIT ANCRES] (getAnchorDistances) On va calculer les distances relatives à l'ancre d'id " + String(pStaticAnchorId));
    std::unordered_map<std::string, float> vDistances;
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();
    bool vAreDistancesReceived = false;
    
    std::vector<int> vAnchorsIdWithoutStaticAnchorId = pAnchors.giveModuleIdList();
    // On supprime l'id pStaticAnchorId de vAnchorsIdWithoutStaticAnchorId
    vAnchorsIdWithoutStaticAnchorId.erase(std::remove(vAnchorsIdWithoutStaticAnchorId.begin(), vAnchorsIdWithoutStaticAnchorId.end(), pStaticAnchorId), vAnchorsIdWithoutStaticAnchorId.end());

    twai_message_t vMessage;
    DecodedData vMessageData;

    // Basculement de rôle : Les autres modules deviennent temporairement des Tags
    Serial.println("[INIT ANCRES] (getAnchorDistances) Premier Toggle");
    toggleAnchorsMode(vAnchorsId, pStaticAnchorId);

    // On laisse un peu de temps aux ancres pour qu'elles redémarrent et calculent des distances
    delay(10000);

    unsigned long startTime = millis();
    
    // Timeout de 5000ms pour garantir plusieurs cycles de polling
    while ((!vAreDistancesReceived) && (millis() - startTime < 5000)) 
    {
        // On vérifie si le hub a reçu les distances de la part de l'ancre statique
        if (receiveCanMessage(vMessage))
        {
            if (decodeCanMessage(vMessage, vMessageData))
            {
                if (vMessageData.type == MESSAGE_STATIC_ANCHOR_ID_AND_ALL_DISTANCES &&
                    vMessageData.aStaticAnchorIdDuringToggle == pStaticAnchorId)
                {
                    vAreDistancesReceived = true;
                    Serial.println("[INIT ANCRES] On enregistre les distances pour calculer les positions des ancres par rapport à l'ancre statique " + String(pStaticAnchorId));
                    Serial.println("[INIT ANCRES] Voici les distances reçues : ");
                    for (int i = 0; i < ANCHORS_NUMBER; i++)
                    {
                        uint16_t vCurrentDistance = vMessageData.aDistances[i];
                        Serial.println("[INIT ANCRES] Distance avec l'ancre " + String(i) + " : " + String(vCurrentDistance));
                    }

                    for (int vCurrentTagId : vAnchorsIdWithoutStaticAnchorId) 
                    {
                        // Règle d'unicité : on ne stocke que si l'ID demandeur est inférieur à l'ID cible (évite les doublons 1-2 et 2-1)
                        if (pStaticAnchorId < vCurrentTagId) 
                        {
                            if (vMessageData.aDistances[vCurrentTagId] > 0.0f) 
                            {
                                std::string vKey = std::to_string(pStaticAnchorId + 1) + std::to_string(vCurrentTagId + 1);
                                vDistances[vKey] = vMessageData.aDistances[vCurrentTagId];
                            }
                        }
                    }
                }
            }
        }
        // Sinon on envoie un message pour demander les distances à l'Ancre statique
        else
        {
            MsgRequestAnchorDistancesDuringCalibHubOrder vMessageToSend = {(uint8_t)pStaticAnchorId};
            sendCanOrderFromHubTo(pStaticAnchorId, HUB_ORDER_REQUEST_ANCHOR_DISTANCES_DURING_CALIB, vMessageToSend);
        }
    }

    // Rétablissement : on renvoie le même ordre pour que les modules redeviennent des Ancres
    Serial.println("[INIT ANCRES] (getAnchorDistances) Deuxième Toggle");
    toggleAnchorsMode(vAnchorsId, pStaticAnchorId);
    
    Serial.println("[INIT ANCRES] Fin du calcul des distances relatives à l'ancre d'id " + String(pStaticAnchorId));
    return vDistances;
}

void initAnchorsPosition(UWBModuleList & pAnchors)
{
    Serial.println("[INIT ANCRES] ---- DEMARRAGE INIT ANCRES PROTOCOLE ----");
    sendToAnchorsInitialisationPhaseSignal(pAnchors, HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL);

    std::unordered_map<std::string, float> vAnchorDistances;
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();

    for (int vCurrentId : vAnchorsId)
    {
        std::unordered_map<std::string, float> vCurrentAnchorDistances = getAnchorDistances(vCurrentId, pAnchors);

        for (auto it = vCurrentAnchorDistances.begin(); it != vCurrentAnchorDistances.end(); ++it)
        {
            vAnchorDistances[it->first] = it->second;
        }
    }

    sendToAnchorsInitialisationPhaseSignal(pAnchors, HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL);

    Serial.println("[INIT ANCRES] ---- REALIGNEMENT COORD ANCRES ----");
    // Calcul mathématique des coordonnées à partir de la matrice des distances
    initAnchorsCoordinatesWithGD(pAnchors, vAnchorDistances, ITERATIONS, LEARNING_RATE);
    Serial.println("[INIT ANCRES] ---- FIN INIT ANCRES PROTOCOLE ----");
}

void sendToAnchorsInitialisationPhaseSignal(UWBModuleList & pAnchors, int pSignalType)
{
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();

    Serial.println("[INIT ANCRES] On envoie à toutes les ancres le signal d'init : " + String(pSignalType));

    for (uint8_t vCurrentId : vAnchorsId)
    {
        MsgAnchorCalibHubOrder vMessage = {vCurrentId};
        sendCanOrderFromHubTo(vCurrentId, pSignalType, vMessage);
    }
}

void toggleAnchorsMode(std::vector<int> pAnchorsId, uint8_t pStaticAnchorId)
{
    Serial.println("[INIT ANCRES] On ordonne aux ancres de changer de mode (ID Ancre statique : " + String(pStaticAnchorId) + ")");
    for (size_t i = 0; i < pAnchorsId.size(); i++)
    {
        MsgToggleHubOrder vMessage;
        vMessage.staticAnchorId = pStaticAnchorId;
        sendCanOrderFromHubTo(pAnchorsId[i], HUB_ORDER_TOGGLE_MODULE_MODE, vMessage);
    }
}

void initTestHardcodedAnchorsPosition(UWBModuleList & pAnchors)
{
    // On hardcode les distances

    Serial.println("[Hub INIT ANCRES] On rentre dans initTestHardcodedAnchorsPosition");

    unordered_map<string, float> vMesuredDistances;

    vMesuredDistances["12"] = 3.58; // 3.58
    vMesuredDistances["13"] = 4.42; // 4.68
    vMesuredDistances["14"] = 3.24; // 3.15
    vMesuredDistances["23"] = 3.48; // 3.05
    vMesuredDistances["24"] = 4.74; // 5.15
    vMesuredDistances["34"] = 3.44; // 4.12

    Serial.println("[Hub INIT ANCRES] Coordonées des ancres avant 1er calcul :");
    Serial.println(pAnchors.toString().c_str());

    initAnchorsCoordinates(pAnchors, vMesuredDistances);

    Serial.println("[Hub INIT ANCRES] Coordonées des ancres avant 2e calcul :");
    Serial.println(pAnchors.toString().c_str());

    // On calcule les positions des ancres
    initAnchorsCoordinatesWithGD(pAnchors, vMesuredDistances, ITERATIONS, LEARNING_RATE);
    Serial.println("[Hub INIT ANCRES] On sort dans initTestHardcodedAnchorsPosition. Coordonées des ancres après 2e calcul :");
    Serial.println(pAnchors.toString().c_str());
}