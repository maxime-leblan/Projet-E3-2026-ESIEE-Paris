#include <Arduino.h>
#include "CANMessageManager.hpp" 
#include "driver/twai.h" 

// --- PARAMÈTRES DE LA CARTE ---
// Changez cette valeur à 2 pour la deuxième carte !
const uint8_t ID_ANCRE = 1; 

unsigned long chronoEnvoi = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n====================================");
    Serial.printf("[BOOT] ANCRE %d (Testeur CAN RX/TX)\n", ID_ANCRE);
    Serial.println("====================================");

    initCan(ANCHOR_RX_PIN, ANCHOR_TX_PIN);
    Serial.println("[INIT] Driver TWAI demarre. Pret a ping-pong !");
}

void loop() {
    // ==========================================
    // 1. TRANSMISSION (Toutes les 2 secondes)
    // ==========================================
    if (millis() - chronoEnvoi > 2000) {
        Serial.printf("\n[TX] L'Ancre %d envoie sa trame...\n", ID_ANCRE);
        
        // L'Ancre 1 envoie une info sur le Tag 99, l'Ancre 2 sur le Tag 88
        uint8_t id_tag_cible = (ID_ANCRE == 1) ? 99 : 88;
        float distance_fictive = (ID_ANCRE == 1) ? 3.14f : 6.28f;

        // Utilisation de votre fonction métier
        sendCanDistance(ID_ANCRE, id_tag_cible, distance_fictive);

        chronoEnvoi = millis();
    }

    // ==========================================
    // 2. RÉCEPTION (En continu, indispensable pour vider le buffer)
    // ==========================================
    twai_message_t messageRecu;
    
    // On boucle pour vider TOUS les messages en attente (très important !)
    while (receiveCanMessage(messageRecu)) {
        Serial.println("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        Serial.printf("[RX] TRAME RECUE (ID Brut : 0x%X)\n", messageRecu.identifier);
        
        DecodedData donnees;
        if (decodeCanMessage(messageRecu, donnees)) {
            Serial.printf("  |- Emetteur (Ancre) : %d\n", donnees.id_ancre);
            
            if (donnees.type == MESSAGE_TAG_ID_AND_DISTANCE) {
                Serial.printf("  |- Format reconnu   : Distance Tag\n");
                Serial.printf("  |- Tag concerne     : %d\n", donnees.id_tag);
                Serial.printf("  |- Distance lue     : %.2f m\n", donnees.distance);
            } else {
                Serial.printf("  |- Format recu      : Type 0x%02X\n", donnees.type);
            }
        } else {
            Serial.println("[DECODE] ECHEC : Trame illisible ou type inconnu.");
        }
        Serial.println("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    }

    // ==========================================
    // 3. DIAGNOSTIC MATÉRIEL (Toutes les 5 secondes)
    // ==========================================
    static unsigned long chronoDiag = 0;
    if (millis() - chronoDiag > 5000) {
        twai_status_info_t status_info;
        if (twai_get_status_info(&status_info) == ESP_OK) {
            // Si le buffer n'est pas vide (msgs_to_rx > 0), c'est qu'on ne lit pas assez vite
            if (status_info.msgs_to_tx > 0 || status_info.msgs_to_rx > 0 || status_info.tx_error_counter > 0) {
                Serial.println("\n[DIAG] --- ANOMALIE DETECTEE ---");
                Serial.printf("En attente d'envoi (TX) : %d\n", status_info.msgs_to_tx);
                Serial.printf("En attente de lect (RX) : %d\n", status_info.msgs_to_rx);
                Serial.printf("Erreurs materielles TX  : %d\n", status_info.tx_error_counter);
                Serial.println("--------------------------------");
            }
        }
        chronoDiag = millis();
    }
}

