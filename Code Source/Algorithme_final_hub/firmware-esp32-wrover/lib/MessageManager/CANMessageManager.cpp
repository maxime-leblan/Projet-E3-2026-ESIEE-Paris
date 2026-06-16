#include "CANMessageManager.hpp"

bool decodeCanMessage(const twai_message_t &message, DecodedData &output) 
{
    // 1. Extraction du type et de l'ID de l'ancre grâce aux masques binaires
    output.type = readFirstHexaNumber(message.identifier);
    output.id_ancre = readSecondHexaNumber(message.identifier);

    // Par défaut, on initialise les autres champs à 0
    output.id_tag = 0;
    output.distance = 0.0f;

    // 2. Traitement différenciée selon le type de message détecté
    switch (output.type) {
        
        case MESSAGE_ID_ONLY:
            // Ce message ne contient pas de données dans le tableau data (data_length_code == 0)
            // Les informations utiles (type et id_ancre) sont déjà extraites
            return true;

        case MESSAGE_TAG_ID_AND_DISTANCE:
            // Sécurité : On vérifie que la taille des données reçues correspond bien à notre structure MsgDistance
            if (message.data_length_code == sizeof(MsgDistance)) {
                MsgDistance payload;
                // On copie les données brutes du tableau de la trame CAN vers notre structure locale
                memcpy(&payload, message.data, sizeof(MsgDistance));
                
                // On remplit la structure de sortie
                output.id_tag = payload.tag_id;
                output.distance = payload.distance;
                return true;
            }
            Serial.println("Erreur décodage: Taille de données incorrecte pour MESSAGE_TAG_ID_AND_DISTANCE");
            return false;

        case MESSAGE_HUB_ORDER:
            // Si vos ordres possèdent un octet de commande dans message.data[0], vous pouvez l'extraire ici
            return true;

        default:
            Serial.printf("Erreur décodage: Type de message inconnu (0x%X)\n", output.type);
            return false;
    }
}

bool receiveCanMessage(twai_message_t &messageRecu)
{
    bool result = (twai_receive(&messageRecu, pdMS_TO_TICKS(DATA_RECEPTION_TIME)) == ESP_OK);
    if (result) {
    Serial.printf(("Message received, <identifier> = " + String(messageRecu.identifier) + "\n").c_str());
    } else {
    Serial.printf("Failed to receive the message.\n");
    }

    return result;
}

void sendCanDistance(uint8_t id_ancre, uint8_t id_tag, float dist) {
    twai_message_t message;
    message.identifier = MESSAGE_TAG_ID_AND_DISTANCE + id_ancre;
    message.extd = 0;
    message.rtr = 0;
    message.data_length_code = sizeof(MsgDistance); // 5 octets

    MsgDistance payload = { id_tag, dist };
    memcpy(message.data, &payload, sizeof(MsgDistance));

    // Queue message for transmission
    if (twai_transmit(&message, pdMS_TO_TICKS(DATA_TRANSMISSION_TIME)) == ESP_OK) {
    Serial.printf(("Message queued for transmission, <identifier> = " + String(message.identifier) + "\n").c_str());
    } else {
    Serial.printf("Failed to queue message for transmission\n");
    }
}

void sendCanSignal(uint16_t pModuleId)
{
    twai_message_t message;
    message.identifier = MESSAGE_ID_ONLY + pModuleId;
    message.extd = 0;
    message.rtr = 0;
    message.data_length_code = 0; // Pas de données, l'ID suffit (ex: signal de vie)

    // Queue message for transmission
    if (twai_transmit(&message, pdMS_TO_TICKS(DATA_TRANSMISSION_TIME)) == ESP_OK) {
    Serial.printf(("Message queued for transmission, <identifier> = " + String(message.identifier) + "\n").c_str());
    } else {
    Serial.printf("Failed to queue message for transmission\n");
    }
}

void initCan(int pRXPin, int pTXPin)
{
    driver_installed = false;
    // Initialize configuration structures using macro initializers
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)pTXPin, (gpio_num_t)pRXPin, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  //Look in the api-reference for other speed sets.
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
    } else {
    Serial.println("Failed to install driver");
    return;
    }

    // Start TWAI driver
    if (twai_start() == ESP_OK) {
    Serial.println("Driver started");
    } else {
    Serial.println("Failed to start driver");
    return;
    }

    // Reconfigure alerts to detect TX alerts and Bus-Off errors
    uint32_t alerts_to_enable = TWAI_ALERT_TX_IDLE | TWAI_ALERT_TX_SUCCESS | TWAI_ALERT_TX_FAILED | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR;
    if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    Serial.println("CAN Alerts reconfigured");
    } else {
    Serial.println("Failed to reconfigure alerts");
    return;
    }

    // TWAI driver is now successfully installed and started
    driver_installed = true;
}