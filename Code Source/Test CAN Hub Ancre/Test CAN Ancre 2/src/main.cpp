
#include <Arduino.h>

#include "UWBModuleActions.hpp"
#include "CANMessageManager.hpp"
#include "UWBMessageManager.hpp"
#include "InitialisationProtocol.hpp"
#include "UWBDataManager.hpp"

// Identifiant statique de cette ancre (0, 1, 2 ou 3)
// Idéalement, à configurer via un dip-switch physique, ou codé en dur pour chaque carte avant le flash
#define MY_ANCHOR_ID 2

#define HAUTEUR_MAX_TAG_METRES 5.0f
#define BUZZER_GPIO_PIN 4
#define BUZZER_FREQUENCY 600

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

    Serial.println("On essaie de lire un message de l'ancre 1 par CAN");

    delay(2000);

    // on vérifie si une ancre nous a envoyé un message
    if (receiveCanMessage(vCanMessage))
    {
        if (decodeCanMessage(vCanMessage, vDecodedCanData))
        {
            Serial.println("Message reçu de l'ancre " + String(vDecodedCanData.id_ancre) + ", tag " + String(vDecodedCanData.id_tag));
            Serial.println("On répond...");
            sendCanDistance(0, vDecodedCanData.id_tag, 10.25);
        }
        else
        {
            Serial.println("Erreur de décodage du message provenant de l'ancre 1");
        }
    }
    
}