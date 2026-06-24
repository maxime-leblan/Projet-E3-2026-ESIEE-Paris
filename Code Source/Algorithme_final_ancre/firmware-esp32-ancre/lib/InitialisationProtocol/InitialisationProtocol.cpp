#include "InitialisationProtocol.hpp"
#include "CANMessageManager.hpp"
#include "UWBModuleActions.hpp"
#include "OLEDManager.hpp"
#include "UWBMessageManager.hpp"
#include "UWBDataManager.hpp"
#include "Messages.hpp"

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
    LocalRole currentRole = ROLE_ANCHOR; 

    Serial.printf("\n[INIT ANCRE %d] === DEBUT PHASE CALIBRATION ===\n", pMyAnchorId);
    updateCANAction("INIT POSITION", "Attente de role...");

    for (int i = 0; i < 6; i++) {
        memoiresDesTags[i].donneeValide = false;
        memoiresDesTags[i].distances.clear();
    }

    while (!vHasReceivedEndMessage)
    {
        // =================================================================
        // 1. ÉCOUTE DU BUS CAN
        // =================================================================
        while (receiveCanMessage(vMessage))
        {
            if (decodeCanMessage(vMessage, vMessageData))
            {
                if (vMessageData.type == MESSAGE_HUB_ORDER && vMessageData.id_ancre == pMyAnchorId)
                {
                    if (vMessageData.aOrderType == HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL) {
                        Serial.println("[CAN RX] Ordre de fin de calibration recu !");
                        vHasReceivedEndMessage = true;
                        break;
                    }
                    else if (vMessageData.aOrderType == HUB_ORDER_SET_AS_TAG) {
                        updateCANAction("CALIBRATION", "Mode TAG Actif");
                        setUWBModeTag(pMyAnchorId);
                        currentRole = ROLE_TAG;
                    }
                    else if (vMessageData.aOrderType == HUB_ORDER_SET_AS_ANCHOR) {
                        updateCANAction("CALIBRATION", "Mode ANCRE Actif");
                        setUWBModeAnchor(pMyAnchorId);
                        currentRole = ROLE_ANCHOR;
                        while(UWBSerial.available()) { UWBSerial.read(); }
                    }
                    else if (vMessageData.aOrderType == HUB_ORDER_REQUEST_DISTANCES) {
                        uint8_t tagCible = vMessageData.id_tag;
                        if (currentRole == ROLE_ANCHOR && tagCible < 6 && memoiresDesTags[tagCible].donneeValide) {
                            sendCanDistanceFromAnchorToHub(pMyAnchorId, tagCible, memoiresDesTags[tagCible].distances);
                        }
                    }
                }
            }
        }

        if (vHasReceivedEndMessage) break;

        // NOTE : LE PING MANUEL (AT+RANGE) A ETE TOTALEMENT SUPPRIME POUR NE PAS CRASHER L'UART.

        // =================================================================
        // 2. ÉCOUTE UWB ET PARSING INCASSABLE (sscanf)
        // =================================================================
        String rawUWBMessage = "";
        
        while (receiveUWBMessage(UWBSerial, rawUWBMessage, Serial)) {
            
            // Le [UART BRUT] s'affiche ici automatiquement grâce à ton UWBMessageManager.cpp
            
            if (rawUWBMessage.startsWith("AT+RANGE")) {
                int tid_val, mask_val, seq_val;
                int dists[8];

                int parsedItems = sscanf(rawUWBMessage.c_str(), 
                    "AT+RANGE=tid:%d,mask:%x,seq:%d,range:(%d,%d,%d,%d,%d,%d,%d,%d)",
                    &tid_val, &mask_val, &seq_val,
                    &dists[0], &dists[1], &dists[2], &dists[3],
                    &dists[4], &dists[5], &dists[6], &dists[7]
                );

                if (parsedItems == 11) {
                    uint8_t tid = (uint8_t)tid_val;

                    if (currentRole == ROLE_TAG) {
                        if (tid == pMyAnchorId) {
                            std::vector<uint16_t> mesDistances;
                            for (int i = 0; i < 4; i++) {
                                mesDistances.push_back((uint16_t)dists[i]);
                            }
                            
                            sendCanDistanceFromAnchorToHub(pMyAnchorId, pMyAnchorId, mesDistances);
                            Serial.printf("[BYPASS CAN] SUCCES ! Distances injectees au Hub : [%d, %d, %d, %d]\n",
                                          mesDistances[0], mesDistances[1], mesDistances[2], mesDistances[3]);
                        }
                    }
                    else if (currentRole == ROLE_ANCHOR) {
                        if (tid < 4) {
                            memoiresDesTags[tid].distances.clear();
                            for (int i = 0; i < 4; i++) {
                                memoiresDesTags[tid].distances.push_back((uint16_t)dists[i]);
                            }
                            memoiresDesTags[tid].donneeValide = true;
                        }
                    }
                } else {
                    Serial.printf("[UWB ERREUR] sscanf a echoue. Elements trouves : %d/11\n", parsedItems);
                }
            }
        }
        
        delay(5);
    }

    Serial.println("[INIT ANCRE] Ordre final recu. Restauration du role par defaut (ANCRE).");
    setUWBModeAnchor(pMyAnchorId);
    updateCANAction("INIT POSITION", "Phase terminee OK");
}

