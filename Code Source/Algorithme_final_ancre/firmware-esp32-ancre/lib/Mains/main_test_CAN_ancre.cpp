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

    Serial.println("On essaie d'envoyer un message au hub par CAN");
    sendCanDistanceFromAnchorToHub(MY_ANCHOR_ID, 4, {1, 2, 3, 4});

    delay(2000);

    // on vérifie si le hub nous a répondu
    if (receiveCanMessage(vCanMessage))
    {
        if (decodeCanMessage(vCanMessage, vDecodedCanData))
        {
            Serial.println("Message reçu du hub : tag " + String(vDecodedCanData.id_tag) + ", distance " + String(vDecodedCanData.distance));
        }
        else
        {
            Serial.println("Erreur de décodage du message provenant du Hub");
        }
    }
    
}