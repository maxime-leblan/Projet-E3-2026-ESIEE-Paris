#include <Arduino.h>

#include "UWBModuleActions.hpp"
#include "CANMessageManager.hpp"
#include "UWBMessageManager.hpp"
#include "InitialisationProtocol.hpp"
#include "UWBDataManager.hpp"

// Identifiant statique de cette ancre (0, 1, 2 ou 3)
#define MY_ANCHOR_ID 0

// ========================================================
// MEMOIRE TAMPON DE L'ANCRE (Partitionnée par Tag)
// ========================================================
#define MAX_SUPPORTED_TAGS 5 // L'ancre peut retenir la position de 5 ouvriers en même temps

struct MemoireTag {
    bool donneeValide = false;
    std::vector<uint16_t> distances = {0, 0, 0, 0};
};

MemoireTag memoiresDesTags[MAX_SUPPORTED_TAGS];

void setup() {
    Serial.begin(115200);
    
    initUWBModule(MY_ANCHOR_ID);

    initCan(ANCHOR_RX_PIN, ANCHOR_TX_PIN);

    Serial.printf("\n[SETUP] === ANCRE %d INITIALISEE ET PRETE (POLLING CAN) ===\n", MY_ANCHOR_ID);
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
        
        // --- NETTOYAGE VITAL DE L'EN-TÊTE AT+RDATA ---
        // On supprime la taille du paquet injectée par le module radio (ex: "AT+RDATA=29,")
        int commaIndex = rawUWBMessage.indexOf(',');
        if (rawUWBMessage.startsWith("AT+RDATA") && commaIndex != -1) {
            rawUWBMessage = rawUWBMessage.substring(commaIndex + 1);
        }

        // Affichage Série Temps Réel
        Serial.println("\n[UWB RX] *** Trame recue *** : " + rawUWBMessage);
        
        // Affichage OLED Temps Réel (On limite à 20 caractères pour que ça rentre)
        updateScreen("RX UWB", rawUWBMessage.substring(0, 20)); 

        std::string stdRawMsg(rawUWBMessage.c_str());
        std::vector<int> parsedData = getDataFromString(stdRawMsg, "[0-9]+");
        
        if (parsedData.size() >= (FIRST_TAG_DISTANCE_INDEX + 4)) {
            // Récupération de l'ID du Tag qui a envoyé cette trame
            uint8_t currentTagId = (uint8_t)getTagIdFromTagData(parsedData);
            
            // Sécurité de la mémoire
            if (currentTagId < MAX_SUPPORTED_TAGS) {
                memoiresDesTags[currentTagId].distances.clear();
                for (int i = 0; i < 4; i++) {
                    memoiresDesTags[currentTagId].distances.push_back((uint16_t)getDistanceFromAnchor(parsedData, i));
                }
                memoiresDesTags[currentTagId].donneeValide = true;
                
                Serial.printf("[MEMOIRE] MAJ Tag %d : [%d, %d, %d, %d] cm\n", 
                              currentTagId, 
                              memoiresDesTags[currentTagId].distances[0], 
                              memoiresDesTags[currentTagId].distances[1], 
                              memoiresDesTags[currentTagId].distances[2], 
                              memoiresDesTags[currentTagId].distances[3]);
            }
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
                Serial.println("\n[CAN RX] Ordre CAN : Lancement calibration Ancre");
                updateScreen("INIT ANCRE", "Calibration...");
                runCompleteInitialisationPhase(MY_ANCHOR_ID);
                updateScreen("ANCRE " + String(MY_ANCHOR_ID), "Surveillance");
            }
                        
            // --- CAS B : Relais d'une alerte vers le Tag (Inchangé) ---
            else if (vDecodedCanData.type == MESSAGE_TAG_ID_AND_DISTANCE) 
            {
                sendDistanceToTag(UWBSerial, MY_ANCHOR_ID, vDecodedCanData.id_tag, vDecodedCanData.distance);
                Serial.printf("\n[CAN->UWB] Relais Radio : Alerte envoyee au Tag %d (Distance: %.2f)\n", 
                              vDecodedCanData.id_tag, vDecodedCanData.distance);
                updateScreen("TX UWB", "Alerte envoyee");
            }

            // --- CAS C (NOUVEAU) : Le Hub demande (poll) les distances ---
            else if (vDecodedCanData.type == MESSAGE_HUB_ORDER && 
                     vDecodedCanData.id_ancre == MY_ANCHOR_ID && 
                     vDecodedCanData.aOrderType == HUB_ORDER_REQUEST_DISTANCES) 
            {
                // On récupère le Tag que le Hub a mis dans sa requête
                uint8_t tagCible = vDecodedCanData.id_tag; 
                Serial.printf("\n[CAN RX] Le Hub interroge la memoire pour le Tag %d.\n", tagCible);
                
                // Si on a des données valides pour CE tag en particulier
                if (tagCible < MAX_SUPPORTED_TAGS && memoiresDesTags[tagCible].donneeValide) {
                    
                    // L'ancre répond à la requête en envoyant son paquet stocké pour ce Tag
                    sendCanDistanceFromAnchorToHub(MY_ANCHOR_ID, tagCible, memoiresDesTags[tagCible].distances);
                    
                    Serial.printf("[CAN TX] Succes : Distances du Tag %d expédiees au Hub.\n", tagCible);
                    updateScreen("TX CAN", "Reponse Hub OK");
                } else {
                    Serial.printf("[CAN TX] Avertissement : Requete du Hub ignoree (Memoire vide pour le Tag %d).\n", tagCible);
                    updateScreen("TX CAN", "Memoire Vide");
                }
            }
        }
    }
}