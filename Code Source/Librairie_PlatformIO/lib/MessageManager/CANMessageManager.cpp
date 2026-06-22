#ifdef ARDUINO_ARCH_ESP32  // N'exécute ce code que si on est sur une ESP32
#include "CANMessageManager.hpp"

bool decodeCanMessage(const twai_message_t &message, DecodedData &output) 
{

    // Par défaut, on initialise les autres champs à 0
    output.id_tag = 0;
    output.aOrderType = 0;
    output.aStaticAnchorIdDuringToggle = 0;
    output.distance = 0.0f;
    output.aDistances = {};

    // 1. Extraction du type grâce aux masques binaires
    if (message.extd == 1) {
        // C'est un message CAN Étendu (29 bits), on le découpe (>> décale, & 0xFF isole 1 octet)
        output.id_tag = readThirdHexaNumber(message.identifier);
        output.id_ancre = readSecondHexaNumber(message.identifier);
    }

    output.type = readFirstHexaNumber(message.identifier);

    // 2. Traitement différencié selon le type de message détecté
    switch (output.type) {
        
        case MESSAGE_ID_ONLY:
            output.id_ancre = readSecondHexaNumber(message.identifier);
            return true;

        case MESSAGE_TAG_ID_AND_DISTANCE:
            if (message.data_length_code == sizeof(MsgDistance)) {
                MsgDistance payload;
                memcpy(&payload, message.data, sizeof(MsgDistance));
                
                output.id_tag = payload.tag_id;
                output.id_ancre = readSecondHexaNumber(message.identifier);
                output.distance = payload.distance;
                return true;
            }
            Serial.println("Erreur decodage : Taille incorrecte pour MESSAGE_TAG_ID_AND_DISTANCE");
            return false;
        
        case MESSAGE_TAG_ID_AND_ALL_DISTANCES:
            if (message.data_length_code == sizeof(MsgAllDistances))
            {
                MsgAllDistances payload;
                memcpy(&payload, message.data, sizeof(MsgAllDistances));

                // on récupère les distances depuis le message reçu vers la structure de sortie
                for (int i = 0; i < ANCHORS_NUMBER; i++)
                {
                    output.aDistances.push_back(payload.aDistances[i]);
                }
                return true;
            }

        case MESSAGE_HUB_ORDER:
            output.id_ancre = readSecondHexaNumber(message.identifier);
            // Sécurité : On vérifie qu'on a au moins reçu l'octet de l'ordre (index 0)
            if (message.data_length_code >= 1) {
                // On extrait d'abord le sous-type d'ordre
                output.aOrderType = message.data[0];

                // On oriente le décodage de la structure selon le sous-type d'ordre
                switch (output.aOrderType) {
                    
                    case HUB_ORDER_TOGGLE_MODULE_MODE:
                        // La taille attendue de la trame totale est : 1 octet (ordre) + taille de la structure
                        if (message.data_length_code == (1 + sizeof(MsgToggleHubOrder))) {
                            MsgToggleHubOrder payload;
                            // Copie des données à partir de l'index 1 (juste après l'octet d'ordre)
                            memcpy(&payload, &message.data[1], sizeof(MsgToggleHubOrder));
                            // On stocke les données dans la variable de sortie
                            output.aStaticAnchorIdDuringToggle = payload.staticAnchorId;
                            return true;
                        }
                        Serial.println("Erreur decodage : Taille incorrecte pour HUB_ORDER_TOGGLE_MODULE_MODE");
                        return false;

                    case HUB_ORDER_START_TAG_CALIBRATION:
                        // La taille attendue de la trame totale est : 1 octet (ordre) + taille de la structure
                        if (message.data_length_code == (1 + sizeof(MsgTagCalibHubOrder))) {
                            MsgTagCalibHubOrder payload;
                            // Copie des données à partir de l'index 1 (juste après l'octet d'ordre)
                            memcpy(&payload, &message.data[1], sizeof(MsgTagCalibHubOrder));
                            // On stocke les données dans la variable de sortie
                            output.id_tag = payload.aTagId;
                            return true;
                        }
                        Serial.println("Erreur decodage : Taille incorrecte pour HUB_ORDER_START_TAG_CALIBRATION");
                        return false;
                    case HUB_ORDER_END_TAG_CALIBRATION:
                        // La taille attendue de la trame totale est : 1 octet (ordre) + taille de la structure
                        if (message.data_length_code == (1 + sizeof(MsgTagCalibHubOrder))) {
                            MsgTagCalibHubOrder payload;
                            // Copie des données à partir de l'index 1 (juste après l'octet d'ordre)
                            memcpy(&payload, &message.data[1], sizeof(MsgTagCalibHubOrder));
                            // On stocke les données dans la variable de sortie
                            output.id_tag = payload.aTagId;
                            return true;
                        }
                        Serial.println("Erreur decodage : Taille incorrecte pour HUB_ORDER_END_TAG_CALIBRATION");
                        return false;
                    
                    case HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL:
                        // La taille attendue de la trame totale est : 1 octet (ordre) + taille de la structure
                        if (message.data_length_code == (1 + sizeof(MsgAnchorCalibHubOrder))) {
                            MsgAnchorCalibHubOrder payload;
                            // Copie des données à partir de l'index 1 (juste après l'octet d'ordre)
                            memcpy(&payload, &message.data[1], sizeof(MsgAnchorCalibHubOrder));
                            // On a juste besoin de l'id_ancre qu'on a déjà stocké plus haut
                            return true;
                        }
                        Serial.println("Erreur decodage : Taille incorrecte pour HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL");
                        return false;

                    case HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL:
                        // La taille attendue de la trame totale est : 1 octet (ordre) + taille de la structure
                        if (message.data_length_code == (1 + sizeof(MsgAnchorCalibHubOrder))) {
                            MsgAnchorCalibHubOrder payload;
                            // Copie des données à partir de l'index 1 (juste après l'octet d'ordre)
                            memcpy(&payload, &message.data[1], sizeof(MsgAnchorCalibHubOrder));
                            // On a juste besoin de l'id_ancre qu'on a déjà stocké plus haut
                            return true;
                        }
                        Serial.println("Erreur decodage : Taille incorrecte pour HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL");
                        return false;

                    case HUB_ORDER_REQUEST_DISTANCES:
                        // La taille attendue de la trame totale est : 1 octet (ordre) + taille de la structure
                        if (message.data_length_code == (1 + sizeof(MsgRequestDistancesHubOrder))) {
                            MsgRequestDistancesHubOrder payload;
                            // Copie des données à partir de l'index 1 (juste après l'octet d'ordre)
                            memcpy(&payload, &message.data[1], sizeof(MsgRequestDistancesHubOrder));
                            // On stocke le Tag demandé dans la variable de sortie
                            output.id_tag = payload.aTagId; 
                            return true;
                        }
                        Serial.println("Erreur decodage : Taille incorrecte pour HUB_ORDER_REQUEST_DISTANCES");
                        return false;

                    // Tu pourras ajouter tes futurs ordres ici très facilement :
                    // case DEUXIEME_ORDRE_FUTUR:
                    //     if (message.data_length_code == (1 + sizeof(MsgFutur))) { ... }

                    default:
                        Serial.printf("Ordre Hub inconnu recu : %d\n", output.aOrderType);
                        return false;
                }
            }
            Serial.println("Erreur decodage : Trame MESSAGE_HUB_ORDER vide");
            return false;

        default:
            Serial.println("Type de message CAN inconnu.");
            return false;
    }
}

bool receiveCanMessage(twai_message_t &messageRecu)
{
    esp_err_t result = twai_receive(&messageRecu, pdMS_TO_TICKS(DATA_RECEPTION_TIME));
    if (result == ESP_OK)
    {
        Serial.printf(("[CAN] Message received, <identifier> = " + String(messageRecu.identifier) + "\n").c_str());
        return true;
    }
    /*
    else if (result == ESP_ERR_TIMEOUT)
    {
        Serial.printf("[CAN] Failed to receive the message : Timed out waiting for message\n");
    }
    */
    else if (result == ESP_ERR_INVALID_ARG)
    {
        Serial.printf("[CAN] Failed to receive the message : Arguments are invalid\n");
    }
    else if (result == ESP_ERR_INVALID_STATE)
    {
        Serial.printf("[CAN] Failed to receive the message : TWAI driver is not installed\n");
    }

    return false;
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

void sendCanDistanceFromAnchorToHub(uint8_t pAnchorId, uint8_t pTagId, std::vector<uint16_t> pAllTagDistances)
{
    twai_message_t message;
    
    // Création de l'ID Étendu (29 bits)
    message.identifier = (pTagId << 8) | (MESSAGE_TAG_ID_AND_ALL_DISTANCES + pAnchorId);
    
    message.extd = 1; // INDISPENSABLE : Active le mode CAN étendu 29 bits
    message.rtr = 0;
    message.data_length_code = sizeof(MsgAllDistances); // Vaut exactement 8

    MsgAllDistances payload;
    // On copie de manière sécurisée les valeurs du vecteur vers le tableau fixe
    for(int i = 0; i < 4 && i < pAllTagDistances.size(); i++) {
        payload.aDistances[i] = pAllTagDistances[i];
    }

    memcpy(message.data, &payload, sizeof(MsgAllDistances));

    esp_err_t vErr = twai_transmit(&message, pdMS_TO_TICKS(DATA_TRANSMISSION_TIME));
    if (vErr != ESP_OK) {
        if (vErr == ESP_ERR_INVALID_ARG)
        {
            Serial.println("Echec transmission CAN (Type 4) : Arguments invalides");
        }
        else if (vErr == ESP_ERR_TIMEOUT)
        {
            Serial.println("Echec transmission CAN (Type 4) : Timed out waiting for space on TX queue");
        }
        else if (vErr == ESP_FAIL)
        {
            Serial.println("Echec transmission CAN (Type 4) : TX queue is disabled and another message is currently transmitting");
        }
        else if (vErr == ESP_ERR_INVALID_STATE)
        {
            Serial.println("Echec transmission CAN (Type 4) : TWAI driver is not in running state, or is not installed");
        }
        else if (vErr == ESP_ERR_NOT_SUPPORTED)
        {
            Serial.println("Echec transmission CAN (Type 4) : Listen Only Mode does not support transmissions");
        }
    }
}

void sendCanSignal(uint8_t pModuleId)
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

void sendCanRequestDistances(uint8_t pAnchorId, uint8_t pTagId) {
    // 1. On encapsule la donnée dans la nouvelle structure
    MsgRequestDistancesHubOrder payload = { pTagId };

    // 2. On utilise ton template générique qui gère tout (Taille, Octet d'ordre, et memcpy)
    sendCanOrderFromHubTo(pAnchorId, HUB_ORDER_REQUEST_DISTANCES, payload);
    
    // Le print de succès/échec est déjà géré à l'intérieur de sendCanOrderFromHubTo, 
    // mais on peut rajouter un petit log de suivi :
    Serial.printf("[CAN TX] Requete envoyee a l'Ancre %d pour le Tag %d.\n", pAnchorId, pTagId);
}

void initCan(int pRXPin, int pTXPin)
{
    driver_installed = false;
    // Initialize configuration structures using macro initializers
    // mettre TWAI_MODE_NORMAL dans g_config pour le mode normal, sinon mettre TWAI_MODE_NO_ACK pour le déboguage sur une seule carte
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

#endif // ARDUINO_ARCH_ESP32