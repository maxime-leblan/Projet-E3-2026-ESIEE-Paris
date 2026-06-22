
// ---- AJOUT DE LA LIBRAIRIE CAN ----
#include "CANMessageManager.hpp" 

#include <Arduino.h>


#define HAUTEUR_MAX_TAG_METRES 5.0f
#define BUZZER_GPIO_PIN 4
#define BUZZER_FREQUENCY 600

void setup() {
    Serial.begin(115200);

    // 2. Initialisation du bus CAN avec les broches dédiées à l'Ancre
    initCan(HUB_RX_PIN, HUB_TX_PIN);

    Serial.println("\n=== HUB INITIALISEE ET PRET (CAN) ===\n");
}

void loop() {
    twai_message_t vCanMessage;
    DecodedData vDecodedCanData;
    String rawUWBMessage = "";

    Serial.println("On essaie de lire un message de l'ancre par CAN");

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
            Serial.println("Erreur de décodage du message provenant de l'ancre");
        }
    }
    
}