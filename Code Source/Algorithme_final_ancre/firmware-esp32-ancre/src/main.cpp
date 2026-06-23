#include <Arduino.h>

#include "OLEDManager.hpp"
#include "UWBModuleActions.hpp"
#include "CANMessageManager.hpp"
#include "UWBMessageManager.hpp"
#include "InitialisationProtocol.hpp"
#include "UWBDataManager.hpp"
#include "Messages.hpp" 

// IMPORTANT : Remplacer par 0, 1, 2 ou 3 selon la carte flashée
#define MY_ANCHOR_ID 3
#define MAX_SUPPORTED_TAGS 6 

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
    
    // --- AFFICHAGE CONFORME AUX SPÉCIFICATIONS ---
    if (MY_ANCHOR_ID == 0) {
        updateCANAction("ANCRE 0 (MASTER)", "Prete (CAN actif)");
    } else {
        updateCANAction("CAN", "En attente d'ordre du hub");
    }
}

void loop() {
    twai_message_t vCanMessage;
    DecodedData vDecodedCanData;
    String rawUWBMessage = "";
    String dernierMessageValide = "";

    // ====================================================================
    // ÉTAPE 1 : ÉCOUTE UWB (Active sur TOUTES les ancres 0, 1, 2, 3)
    // ====================================================================
    // Toutes les cartes captent le broadcast radio et mettent à jour leur OLED
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
                            
                            // Rafraîchissement automatique de la zone basse (les 4 distances)
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

    // ====================================================================
    // ÉTAPE 2 : GESTION DU BUS CAN (Filtrée selon le rôle de l'ancre)
    // ====================================================================
    if (receiveCanMessage(vCanMessage)) {
        if (decodeCanMessage(vCanMessage, vDecodedCanData)) {
            
            // --- PROTECTION ET ROUTAGE SUR LE BUS CAN ---
            if (MY_ANCHOR_ID == 0) {
                // L'Ancre 0 exécute l'ensemble des tâches de communication avec le Hub
                
                if (vDecodedCanData.type == MESSAGE_HUB_ORDER && 
                    vDecodedCanData.id_ancre == MY_ANCHOR_ID && 
                    vDecodedCanData.aOrderType == HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL) 
                {
                    Serial.println("\n[CAN RX] Ordre CAN : Lancement calibration Ancre 0");
                    updateCANAction("INIT ANCRE 0", "Calibration...");
                    runCompleteInitialisationPhase(MY_ANCHOR_ID);
                    updateCANAction("ANCRE 0 (MASTER)", "Prete (CAN actif)");
                }
                else if (vDecodedCanData.type == MESSAGE_TAG_ID_AND_DISTANCE) 
                {
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
            else {
                // Les Ancres 1, 2, 3 IGNORENT les requêtes de polling ordinaires.
                // Elles ne répondent au CAN que si elles reçoivent un ordre d'initialisation global/spécifique.
                
                if (vDecodedCanData.type == MESSAGE_HUB_ORDER && 
                    vDecodedCanData.id_ancre == MY_ANCHOR_ID && 
                    vDecodedCanData.aOrderType == HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL) 
                {
                    updateCANAction("INIT ANCRE", "Calibration...");
                    runCompleteInitialisationPhase(MY_ANCHOR_ID);
                    
                    // Une fois la calibration finie, on restaure le message par défaut
                    updateCANAction("CAN", "En attente d'ordre du hub");
                }
            }
        }
    }
}