#include <Arduino.h>

#include "UWBModuleActions.hpp"
#include "CANMessageManager.hpp"
#include "UWBMessageManager.hpp"
#include "InitialisationProtocol.hpp"
#include "UWBDataManager.hpp"

// Identifiant statique de cette ancre (0, 1, 2 ou 3)
// Idéalement, à configurer via un dip-switch physique, ou codé en dur pour chaque carte avant le flash
#define MY_ANCHOR_ID 0 

void setup() {
    Serial.begin(115200);
    
    // 1. Initialisation de l'écran et du module UWB en mode Ancre
    initUWBModule(MY_ANCHOR_ID);

    // 2. Initialisation du bus CAN avec les broches dédiées à l'Ancre
    initCan(ANCHOR_RX_PIN, ANCHOR_TX_PIN);

    Serial.printf("\n=== ANCRE %d INITIALISEE ET PRETE (CAN) ===\n", MY_ANCHOR_ID);
    updateScreen("ANCRE " + String(MY_ANCHOR_ID), "Prete (CAN actif)");
}

void loop() {
    twai_message_t vCanMessage;
    DecodedData vDecodedCanData;
    String rawUWBMessage = "";

    // ====================================================================
    // ÉTAPE 1 : ÉCOUTE DU BUS CAN (Messages provenant du Hub)
    // ====================================================================
    if (receiveCanMessage(vCanMessage)) {
        if (decodeCanMessage(vCanMessage, vDecodedCanData)) {
            
            // Cas A : Le Hub demande le début de l'initialisation des positions
            if (vDecodedCanData.type == MESSAGE_HUB_ORDER && 
                vDecodedCanData.id_ancre == MY_ANCHOR_ID && 
                vDecodedCanData.aOrderType == HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL) 
            {
                Serial.println("[MAIN] Ordre du Hub : Lancement de l'initialisation des positions !");
                updateScreen("INIT ANCRE", "Calibration...");
                
                // Cette fonction gère toute la boucle d'initialisation de son côté
                runCompleteInitialisationPhase(MY_ANCHOR_ID);
                
                Serial.println("[MAIN] Initialisation terminee. Retour au mode surveillance.");
                updateScreen("ANCRE " + String(MY_ANCHOR_ID), "Surveillance active");
            }
            
            // Cas B : Le Hub transmet la distance de sécurité calculée vers un Tag
            else if (vDecodedCanData.type == MESSAGE_TAG_ID_AND_DISTANCE) 
            {
                // On relaie immédiatement la distance de sécurité au Tag via les ondes UWB
                sendDistanceToTag(UWBSerial, MY_ANCHOR_ID, vDecodedCanData.id_tag, vDecodedCanData.distance);
                
                Serial.printf("[MAIN] Relais CAN -> UWB : Tag %d est a %.2fm de la zone.\n", 
                              vDecodedCanData.id_tag, vDecodedCanData.distance);
            }
        }
    }

    // ====================================================================
    // ÉTAPE 2 : ÉCOUTE DU MODULE UWB (Messages provenant des Tags)
    // ====================================================================
    if (receiveUWBDistanceMessage(UWBSerial, rawUWBMessage)) {
        
        updateScreen("ANCRE " + String(MY_ANCHOR_ID), "reception (CAN actif)");
        Serial.println("[MAIN] Reception d'une trame UWB. Extraction des distances et relais vers le Hub via CAN...");
        // 1. Conversion de la String Arduino en std::string (requis par UWBDataManager)
        std::string stdRawMsg(rawUWBMessage.c_str());
        
        // 2. Extraction des entiers de la trame
        std::vector<int> parsedData = getDataFromString(stdRawMsg, "[0-9]+");
        
        // 3. Sécurité : on vérifie qu'on a extrait suffisamment de données 
        // (tid, mask, seq + au moins les 4 distances = 7 valeurs minimum)
        if (parsedData.size() >= (FIRST_TAG_DISTANCE_INDEX + 4)) {
            
            // Récupération de l'ID du Tag
            uint8_t tagId = (uint8_t)getTagIdFromTagData(parsedData);
            
            // Extraction des 4 distances
            std::vector<uint16_t> distancesArray;
            for (int i = 0; i < 4; i++) {
                distancesArray.push_back((uint16_t)getDistanceFromAnchor(parsedData, i));
            }
            
            // 4. Envoi de ce tableau au Hub via le bus CAN
            sendCanDistanceFromAnchorToHub(MY_ANCHOR_ID, tagId, distancesArray);
            
            // Affichage debug optionnel
            // Serial.printf("[MAIN] Relais UWB -> CAN : Distances du Tag %d envoyees au Hub.\n", tagId);
        } else {
            Serial.println("[MAIN] Erreur : Trame UWB recue mais incomplete ou mal formatee.");
        }
    }
}