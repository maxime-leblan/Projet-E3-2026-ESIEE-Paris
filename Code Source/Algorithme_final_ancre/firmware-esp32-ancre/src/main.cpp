#include <Arduino.h>

#include "OLEDManager.hpp"
#include "UWBModuleActions.hpp"
#include "CANMessageManager.hpp"
#include "UWBMessageManager.hpp"
#include "InitialisationProtocol.hpp"
#include "UWBDataManager.hpp"
#include "Messages.hpp" 

// IMPORTANT : Remplacer par 0, 1, 2 ou 3 selon la carte flashée
#define MY_ANCHOR_ID 0
#define MAX_SUPPORTED_TAGS 6 

struct MemoireTag {
    bool donneeValide = false;
    std::vector<uint16_t> distances = {0, 0, 0, 0};
    int pressionPa = 0; // +1Pa = -8,3cm
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
    while (receiveUWBMessage(UWBSerial, rawUWBMessage, Serial)) {
        // Log direct dès qu'une trame radio arrive
        Serial.println("[ANCRE RX DIAG] Trame UWB brute détectée : " + rawUWBMessage);
        
        if (rawUWBMessage.indexOf("AT+RANGE") != -1 || rawUWBMessage.indexOf("&") != -1) {
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
                    
                    // Log de contrôle du nombre d'entiers extraits pour les distances
                    Serial.printf("[ANCRE PARSE DIAG] Entiers trouvés par la Regex : %d (Attendu au moins %d)\n", 
                                parsedData.size(), (FIRST_TAG_DISTANCE_INDEX + 4));

                    if (parsedData.size() >= (FIRST_TAG_DISTANCE_INDEX + 4)) {
                        uint8_t tid = (uint8_t)getTagIdFromTagData(parsedData);
                        
                        if (tid < MAX_SUPPORTED_TAGS) {
                            memoiresDesTags[tid].distances.clear();
                            
                            for (int i = 0; i < 4; i++) {
                                memoiresDesTags[tid].distances.push_back((uint16_t)getDistanceFromAnchor(parsedData, i));
                            }
                            
                            // Sauvegarde
                            memoiresDesTags[tid].pressionPa = decodedMsg.pressionPa;
                            memoiresDesTags[tid].donneeValide = true;
                            
                            // --- TRACE FINALE COMPLÈTE ---
                            Serial.printf("[SUCCÈS STOCKAGE ANCRE] Tag ID: %d | Pression: %d Pa | Distances: [%d, %d, %d, %d]\n", 
                                        tid, 
                                        memoiresDesTags[tid].pressionPa,
                                        memoiresDesTags[tid].distances[0],
                                        memoiresDesTags[tid].distances[1],
                                        memoiresDesTags[tid].distances[2],
                                        memoiresDesTags[tid].distances[3]);
                            
                            updateUWBData(tid, 
                                        memoiresDesTags[tid].distances[0], 
                                        memoiresDesTags[tid].distances[1], 
                                        memoiresDesTags[tid].distances[2], 
                                        memoiresDesTags[tid].distances[3]);
                        } else {
                            Serial.printf("[ANCRE ERROR] ID Tag hors limites (%d)\n", tid);
                        }
                    } else {
                        Serial.println("[ANCRE ERROR] Structure de trame AT+RANGE incomplète, impossible de mapper les distances.");
                    }
                }
            }
        } else {
            Serial.println("[ANCRE ERROR] Échec complet du décodage par decodeUWBMessage.");
        }
        dernierMessageValide = ""; // Nettoyage
        while (UWBSerial.available()) { UWBSerial.read(); }
    }

    // ====================================================================
    // ÉTAPE 2 : GESTION DU BUS CAN (Filtrée selon le rôle de l'ancre)
    // ====================================================================
    if (receiveCanMessage(vCanMessage)) {
        Serial.println("\n\n[CAN RX] Message reçu sur le bus CAN.");
        if (decodeCanMessage(vCanMessage, vDecodedCanData)) {
            Serial.printf("[CAN RX] Type : %d, Ancre : %d, Tag : %d, Order : %d, Distance : %.2f\n", 
                          vDecodedCanData.type, 
                          vDecodedCanData.id_ancre, 
                          vDecodedCanData.id_tag, 
                          vDecodedCanData.aOrderType, 
                          vDecodedCanData.distance);
            
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
                    
                    Serial.printf("\n[CAN RX] Polling -> Le Hub interroge ma memoire pour le Tag %d.\n", tagCible);
                    // Log de traçabilité : le Hub a bien fait une demande
                    
                    if (tagCible < MAX_SUPPORTED_TAGS && memoiresDesTags[tagCible].donneeValide) {
                        sendCanDistanceFromAnchorToHub(MY_ANCHOR_ID, tagCible, memoiresDesTags[tagCible].distances);
                        
                        // Log de confirmation de renvoi réussi
                        Serial.printf("[CAN TX] Succes : Distances du Tag %d expediees au Hub.\n", tagCible);
                        updateCANAction("TX CAN", "Distances OK");
                    } 
                    else {
                        // Log d'alerte critique : met en évidence si le Hub interroge le mauvais Tag (ex: 0 au lieu de 4)
                        Serial.printf("[CAN TX REFUS] Le Hub demande le Tag %d, mais ma memoire est vide ou invalide pour ce Tag !\n", tagCible);
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