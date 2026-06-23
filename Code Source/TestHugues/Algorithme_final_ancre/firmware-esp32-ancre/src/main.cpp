#include <Arduino.h>

#include "OLEDManager.hpp"
#include "UWBModuleActions.hpp"
#include "CANMessageManager.hpp"
#include "UWBMessageManager.hpp"
#include "InitialisationProtocol.hpp"
#include "UWBDataManager.hpp"
#include "Messages.hpp"

#define MY_ANCHOR_ID 1
#define MAX_SUPPORTED_TAGS 6

// Mémoire partagée (utilisée par InitialisationProtocol via extern)
struct MemoireTag {
    bool donneeValide = false;
    std::vector<uint16_t> distances = {0, 0, 0, 0};
};
MemoireTag memoiresDesTags[MAX_SUPPORTED_TAGS];

void setup() {
    Serial.begin(115200);
    
    initOLED(MY_ANCHOR_ID);
    updateCANAction("DEMARRAGE", "Initialisation UWB...");

    initUWBModule(MY_ANCHOR_ID);
    initCan(ANCHOR_RX_PIN, ANCHOR_TX_PIN);

    Serial.printf("\n[SETUP] === ANCRE %d CONFIGUREE ===\n", MY_ANCHOR_ID);
    
    if (MY_ANCHOR_ID == 0) {
        updateCANAction("ANCRE 0 (MASTER)", "Prete (CAN actif)");
    } else {
        updateCANAction("CAN", "En attente d'ordre");
    }
}

void loop() {
    twai_message_t vCanMessage;
    DecodedData vDecodedCanData;
    String rawUWBMessage = "";
    String dernierMessageValide = "";

    // 1. ECOUTE UWB
    while (receiveUWBMessage(UWBSerial, rawUWBMessage, Serial)) {
        if (rawUWBMessage.startsWith("AT+RANGE") || rawUWBMessage.startsWith("AT+RDATA")) {
            dernierMessageValide = rawUWBMessage;
        }
    }

    if (dernierMessageValide != "") {
        UWBMessage decodedMsg;
        if (decodeUWBMessage(dernierMessageValide, decodedMsg, Serial)) {
            if (decodedMsg.aIsStandardDistanceMessage) {
                int rangeStart = dernierMessageValide.indexOf("AT+RANGE");
                if (rangeStart != -1) {
                    String cleanRangeMsg = dernierMessageValide.substring(rangeStart);
                    std::string stdRawMsg(cleanRangeMsg.c_str());
                    std::vector<int> parsedData = getDataFromString(stdRawMsg, "[0-9]+");
                    
                    if (parsedData.size() >= (FIRST_TAG_DISTANCE_INDEX + 4)) {
                        uint8_t tid = (uint8_t)getTagIdFromTagData(parsedData);
                        
                        if (tid < MAX_SUPPORTED_TAGS) {
                            memoiresDesTags[tid].distances.clear();
                            for (int i = 0; i < 4; i++) {
                                memoiresDesTags[tid].distances.push_back((uint16_t)getDistanceFromAnchor(parsedData, i));
                            }
                            memoiresDesTags[tid].donneeValide = true;
                            
                            updateUWBData(tid,
                                          memoiresDesTags[tid].distances[0],
                                          memoiresDesTags[tid].distances[1],
                                          memoiresDesTags[tid].distances[2],
                                          memoiresDesTags[tid].distances[3]);
                        }
                    }
                }
            }
        }
        while (UWBSerial.available()) { UWBSerial.read(); }
    }

    // 2. ECOUTE CAN
    if (receiveCanMessage(vCanMessage)) {
        if (decodeCanMessage(vCanMessage, vDecodedCanData)) {
            
            // Interception de l'ordre d'initialisation globale de géométrie (Idem pour Master et Slaves)
            if (vDecodedCanData.type == MESSAGE_HUB_ORDER &&
                vDecodedCanData.id_ancre == MY_ANCHOR_ID &&
                vDecodedCanData.aOrderType == HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL)
            {
                updateCANAction("INIT ANCRE", "Calibration...");
                runCompleteInitialisationPhase(MY_ANCHOR_ID);
                updateCANAction("ANCRE " + String(MY_ANCHOR_ID), "Prete (Surveillance)");
                return; // On skip le reste pour ce cycle
            }

            // Gestion spécifique au rôle
            if (MY_ANCHOR_ID == 0) {
                if (vDecodedCanData.type == MESSAGE_TAG_ID_AND_DISTANCE) {
                    sendDistanceToTag(UWBSerial, Serial, MY_ANCHOR_ID, vDecodedCanData.id_tag, vDecodedCanData.distance);
                    updateCANAction("TX UWB", "Alerte relayee");
                }
                else if (vDecodedCanData.type == MESSAGE_HUB_ORDER &&
                         vDecodedCanData.id_ancre == MY_ANCHOR_ID &&
                         vDecodedCanData.aOrderType == HUB_ORDER_REQUEST_DISTANCES)
                {
                    uint8_t tagCible = vDecodedCanData.id_tag;
                    if (tagCible < MAX_SUPPORTED_TAGS && memoiresDesTags[tagCible].donneeValide) {
                        sendCanDistanceFromAnchorToHub(MY_ANCHOR_ID, tagCible, memoiresDesTags[tagCible].distances);
                        updateCANAction("TX CAN", "Distances OK");
                    }
                }
            }
        }
    }
}

