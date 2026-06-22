#include <Arduino.h>

#include "UWBModuleActions.hpp"
#include "CANMessageManager.hpp"
#include "UWBMessageManager.hpp"
#include "InitialisationProtocol.hpp"
#include "UWBDataManager.hpp"

// Identifiant statique de cette ancre (0, 1, 2 ou 3)
#define MY_ANCHOR_ID 2

// ========================================================
// MEMOIRE TAMPON DE L'ANCRE
// ========================================================
uint8_t memoireTagId = 0;
std::vector<uint16_t> memoireDistances = {0, 0, 0, 0};
bool donneeValideEnMemoire = false;

void setup() {
    Serial.begin(115200);
    
    initUWBModule(MY_ANCHOR_ID);

    initCan(ANCHOR_RX_PIN, ANCHOR_TX_PIN);

    Serial.printf("\n=== ANCRE %d INITIALISEE ET PRETE (POLLING CAN) ===\n", MY_ANCHOR_ID);
    updateScreen("ANCRE " + String(MY_ANCHOR_ID), "Prete (CAN actif)");
}

void loop() {
    twai_message_t vCanMessage;
    DecodedData vDecodedCanData;
    String rawUWBMessage = "";

    // ====================================================================
    // ÉTAPE 1 : ÉCOUTE UWB (Mise à jour silencieuse de la mémoire)
    // ====================================================================
    if (receiveUWBDistanceMessage(UWBSerial, Serial, rawUWBMessage)) {
        
        // Affichage Série Temps Réel
        Serial.println("[MAIN] *** RX UWB *** : " + rawUWBMessage);
        
        // Affichage OLED Temps Réel (On limite à 20 caractères pour que ça rentre)
        updateScreen("RX UWB", rawUWBMessage.substring(0, 20)); 

        std::string stdRawMsg(rawUWBMessage.c_str());
        std::vector<int> parsedData = getDataFromString(stdRawMsg, "[0-9]+");
        
        if (parsedData.size() >= (FIRST_TAG_DISTANCE_INDEX + 4)) {
            // Mise à jour de la mémoire interne de l'ancre
            memoireTagId = (uint8_t)getTagIdFromTagData(parsedData);
            
            memoireDistances.clear();
            for (int i = 0; i < 4; i++) {
                memoireDistances.push_back((uint16_t)getDistanceFromAnchor(parsedData, i));
            }
            donneeValideEnMemoire = true;
            
            Serial.printf("[MAIN] Memoire MAJ -> Tag %d : [%d, %d, %d, %d] cm\n", 
                          memoireTagId, memoireDistances[0], memoireDistances[1], memoireDistances[2], memoireDistances[3]);
        }
    }

    // ====================================================================
    // ÉTAPE 2 : ÉCOUTE DU BUS CAN (Ordres provenant du Hub)
    // ====================================================================
    if (receiveCanMessage(vCanMessage)) {
        if (decodeCanMessage(vCanMessage, vDecodedCanData)) {
            
            // --- CAS A : L'initialisation des positions (Inchangé) ---
            if (vDecodedCanData.type == MESSAGE_HUB_ORDER && 
                vDecodedCanData.id_ancre == MY_ANCHOR_ID && 
                vDecodedCanData.aOrderType == HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL) 
            {
                Serial.println("[MAIN] Ordre CAN : Lancement calibration");
                updateScreen("INIT ANCRE", "Calibration...");
                runCompleteInitialisationPhase(MY_ANCHOR_ID);
                updateScreen("ANCRE " + String(MY_ANCHOR_ID), "Surveillance");
            }
                        
            // --- CAS B : Relais d'une alerte vers le Tag (Inchangé) ---
            else if (vDecodedCanData.type == MESSAGE_TAG_ID_AND_DISTANCE) 
            {
                sendDistanceToTag(UWBSerial, MY_ANCHOR_ID, vDecodedCanData.id_tag, vDecodedCanData.distance);
                Serial.printf("[MAIN] Relais CAN->UWB : Alerte envoyee au Tag %d\n", vDecodedCanData.id_tag);
                updateScreen("TX UWB", "Alerte envoyee");
            }

            // --- CAS C (NOUVEAU) : Le Hub demande (poll) les distances ---
            else if (vDecodedCanData.type == MESSAGE_HUB_ORDER && 
                     vDecodedCanData.id_ancre == MY_ANCHOR_ID && 
                     vDecodedCanData.aOrderType == HUB_ORDER_REQUEST_DISTANCES) 
            {
                Serial.println("[MAIN] *** RX CAN *** : Le Hub interroge la memoire.");
                
                if (donneeValideEnMemoire) {
                    // L'ancre répond à la requête en envoyant son paquet stocké
                    sendCanDistanceFromAnchorToHub(MY_ANCHOR_ID, memoireTagId, memoireDistances);
                    
                    Serial.printf("[MAIN] *** TX CAN *** : Distances du Tag %d expédiees au Hub.\n", memoireTagId);
                    updateScreen("TX CAN", "Reponse Hub OK");
                } else {
                    Serial.println("[MAIN] Avertissement : Requete du Hub ignoree (Memoire vide).");
                    updateScreen("TX CAN", "Memoire Vide");
                }
            }
        }
    }
}