#include "InitialisationProtocol.hpp"
#include "CANMessageManager.hpp"
#include "UWBModuleActions.hpp"
#include "OLEDManager.hpp"
#include "UWBMessageManager.hpp"
#include "UWBDataManager.hpp" // Contient FIRST_TAG_DISTANCE_INDEX, etc.
#include "Messages.hpp"

// Accès à la mémoire globale déclarée dans le main.cpp
struct MemoireTag {
    bool donneeValide;
    std::vector<uint16_t> distances;
};
extern MemoireTag memoiresDesTags[];
extern HardwareSerial UWBSerial;

void runCompleteInitialisationPhase(uint8_t pMyAnchorId)
{
    bool vHasReceivedEndMessage = false;
    twai_message_t vMessage;
    DecodedData vMessageData;
    
    enum LocalRole { ROLE_ANCHOR, ROLE_TAG };
    LocalRole currentRole = ROLE_ANCHOR; // État par défaut au lancement

    Serial.printf("\n[INIT ANCRE %d] === DEBUT PHASE CALIBRATION ===\n", pMyAnchorId);
    updateCANAction("INIT POSITION", "Attente de role...");

    while (!vHasReceivedEndMessage)
    {
        // =================================================================
        // 1. ÉCOUTE ET TRAITEMENT DU BUS CAN
        // =================================================================
        if (receiveCanMessage(vMessage))
        {
            if (decodeCanMessage(vMessage, vMessageData))
            {
                // A. Ordre de clôture du Hub
                if (vMessageData.type == MESSAGE_HUB_ORDER &&
                    vMessageData.id_ancre == pMyAnchorId &&
                    vMessageData.aOrderType == HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL)
                {
                    Serial.println("[INIT ANCRE] Ordre de fin reçu. Clôture de la séquence.");
                    vHasReceivedEndMessage = true;
                    break;
                }
                
                // B. Ordre de transformation en TAG
                else if (vMessageData.type == MESSAGE_HUB_ORDER &&
                         vMessageData.id_ancre == pMyAnchorId &&
                         vMessageData.aOrderType == HUB_ORDER_SET_AS_TAG)
                {
                    Serial.println("[INIT ANCRE] Ordre matériel : Je deviens le TAG.");
                    updateCANAction("CALIBRATION", "Mode TAG Actif");
                    setUWBModeTag(pMyAnchorId);
                    currentRole = ROLE_TAG;
                }
                
                // C. Ordre de transformation en ANCRE
                else if (vMessageData.type == MESSAGE_HUB_ORDER &&
                         vMessageData.id_ancre == pMyAnchorId &&
                         vMessageData.aOrderType == HUB_ORDER_SET_AS_ANCHOR)
                {
                    Serial.println("[INIT ANCRE] Ordre matériel : Je deviens ANCRE.");
                    updateCANAction("CALIBRATION", "Mode ANCRE Actif");
                    setUWBModeAnchor(pMyAnchorId);
                    currentRole = ROLE_ANCHOR;
                }
                
                // D. Réponse au Polling du Hub (uniquement si l'on agit comme Ancre)
                else if (vMessageData.type == MESSAGE_HUB_ORDER &&
                         vMessageData.id_ancre == pMyAnchorId &&
                         vMessageData.aOrderType == HUB_ORDER_REQUEST_DISTANCES)
                {
                    uint8_t tagCible = vMessageData.id_tag;
                    
                    if (currentRole == ROLE_ANCHOR && tagCible < 6 && memoiresDesTags[tagCible].donneeValide)
                    {
                        sendCanDistanceFromAnchorToHub(pMyAnchorId, tagCible, memoiresDesTags[tagCible].distances);
                        Serial.printf("[INIT ANCRE CAN] Distances du pseudo-Tag %d envoyées au Hub.\n", tagCible);
                    }
                }
            }
        }

        // =================================================================
        // 2. ÉCOUTE ET TRAITEMENT DU RESEAU UWB (Uniquement en mode Ancre)
        // =================================================================
        if (currentRole == ROLE_ANCHOR)
        {
            String rawUWBMessage = "";
            String dernierMessageValide = "";

            while (receiveUWBMessage(UWBSerial, rawUWBMessage, Serial)) {
                if (rawUWBMessage.startsWith("AT+RANGE") || rawUWBMessage.startsWith("AT+RDATA")) {
                    dernierMessageValide = rawUWBMessage;
                }
            }

            if (dernierMessageValide != "")
            {
                UWBMessage decodedMsg;
                if (decodeUWBMessage(dernierMessageValide, decodedMsg, Serial))
                {
                    if (decodedMsg.aIsStandardDistanceMessage)
                    {
                        int rangeStart = dernierMessageValide.indexOf("AT+RANGE");
                        if (rangeStart != -1)
                        {
                            String cleanRangeMsg = dernierMessageValide.substring(rangeStart);
                            std::string stdRawMsg(cleanRangeMsg.c_str());
                            std::vector<int> parsedData = getDataFromString(stdRawMsg, "[0-9]+");
                            
                            if (parsedData.size() >= (FIRST_TAG_DISTANCE_INDEX + 4))
                            {
                                uint8_t tid = (uint8_t)getTagIdFromTagData(parsedData);
                                
                                if (tid < 6) // MAX_SUPPORTED_TAGS
                                {
                                    memoiresDesTags[tid].distances.clear();
                                    for (int i = 0; i < 4; i++) {
                                        memoiresDesTags[tid].distances.push_back((uint16_t)getDistanceFromAnchor(parsedData, i));
                                    }
                                    memoiresDesTags[tid].donneeValide = true;
                                    
                                    Serial.printf("[INIT ANCRE UWB] Télémétrie captée depuis le module %d\n", tid);
                                }
                            }
                        }
                    }
                }
            }
        }
        
        delay(5); // Protection Watchdog ESP32
    }

    // =================================================================
    // 3. NETTOYAGE POST-CALIBRATION
    // =================================================================
    Serial.println("[INIT ANCRE] Restauration du rôle par défaut (ANCRE).");
    setUWBModeAnchor(pMyAnchorId);
    updateCANAction("INIT POSITION", "Phase terminée OK");
}

