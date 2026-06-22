#include "driver/twai.h"

// ⚠️ MODIFIEZ CES NUMÉROS AVEC LES BROCHES DE VOTRE CARTE MaUWB
#define CAN_TX_PIN   GPIO_NUM_4  // À remplacer par votre broche TX reliée au TCAN (D)
#define CAN_RX_PIN   GPIO_NUM_5  // À remplacer par votre broche RX reliée au TCAN (R)

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Initialisation du bus CAN (TWAI) en mode NO_ACK...");

    // 1. Configuration générale du pilote
    // On force le mode TWAI_MODE_NO_ACK pour le dépannage sur table
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)CAN_TX_PIN, 
        (gpio_num_t)CAN_RX_PIN, 
        TWAI_MODE_NO_ACK
    );

    // 2. Configuration de la vitesse (ici 500 kbps, classique en CAN)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

    // 3. Configuration des filtres (On accepte tout)
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // 4. Installation et démarrage du pilote
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        Serial.println("Pilote installé avec succès.");
    } else {
        Serial.println("Échec de l'installation du pilote.");
        while (1);
    }

    if (twai_start() == ESP_OK) {
        Serial.println("Périphérique CAN démarré.");
    } else {
        Serial.println("Échec du démarrage du périphérique.");
        while (1);
    }
}

void loop() {
    // Création d'une trame CAN standard de test
    twai_message_t tx_msg;
    tx_msg.identifier = 0x123;           // ID du message
    tx_msg.extd = 0;                     // 0 = ID Standard (11 bits)
    tx_msg.rtr = 0;                      // 0 = Trame de données
    tx_msg.data_length_code = 4;         // 4 octets de données
    tx_msg.data[0] = 0xDE;
    tx_msg.data[1] = 0xAD;
    tx_msg.data[2] = 0xBE;
    tx_msg.data[3] = 0xEF;

    // Envoi de la trame
    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(1000)) == ESP_OK) {
        Serial.println("Trame envoyée (génération de carrés sur le bus)");
    } else {
        Serial.println("Échec de l'envoi");
    }

    // Pause de 200ms entre chaque trame pour bien espacer les signaux à l'oscilloscope
    delay(200); 
}