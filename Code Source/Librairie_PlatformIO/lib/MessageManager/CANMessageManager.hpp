#pragma once

#include <Arduino.h>
#include "driver/twai.h"

// Pins used to connect to CAN bus
#define HUB_RX_PIN 34
#define HUB_TX_PIN 25
#define ANCHOR_RX_PIN 15
#define ANCHOR_TX_PIN 16

// Durée de transmission d'un message en ms
#define DATA_TRANSMISSION_TIME 1000
// Durée de réception d'un message en ms
#define DATA_RECEPTION_TIME 1

// Convention de nommage des différents types de messages
#define MESSAGE_ID_ONLY ((uint8_t)0x10)
#define MESSAGE_TAG_ID_AND_DISTANCE ((uint8_t)0x20)
#define MESSAGE_HUB_ORDER ((uint8_t)0x30)

// Convention de nommage des différents types d'ordre du Hub
#define HUB_ORDER_TOGGLE_MODULE_MODE 1

// Macro pour extraire des chiffres de nombres en hexadécimal
#define readFirstHexaNumber(H) (H & 0xF0)
#define readSecondHexaNumber(H) (H & 0x0F)

// Interval:
#define TRANSMIT_RATE_MS 1000

#define POLLING_RATE_MS 1000

static bool driver_installed = false;

// Structure pour le message de type 2 (Id Tag + Distance)
struct __attribute__((packed)) MsgDistance {
    uint8_t tag_id;
    float distance;
};

// Structure pour le message de type 3 pour l'ordre HUB_ORDER_TOGGLE_MODULE_MODE
struct __attribute__((packed)) MsgToggleHubOrder {
    uint8_t staticAnchorId; // L'ID de l'ancre qui reste fixe en mode Ancre
};

/**
 * Structure contenant le résultat du décodage. Elle contient les champs suivants :
 * type, id_ancre, id_tag, distance
 */
struct DecodedData {
    uint32_t type;      // MESSAGE_ID_ONLY, MESSAGE_TAG_ID_AND_DISTANCE, etc.
    uint8_t id_ancre;   // L'émetteur du message
    uint8_t id_tag;     // ID du Tag (uniquement pour le type MESSAGE_TAG_ID_AND_DISTANCE)
    uint8_t aStaticAnchorIdDuringToggle; // ID de l'Ancre qui reste en mode Ancre pendant la phase d'initialisation des positions des Ancres (uniquement pour le type HUB_ORDER_TOGGLE_MODULE_MODE)
    uint8_t aOrderType; // Type d'ordre envoyé par le Hub
    float distance;     // La distance (uniquement pour le type MESSAGE_TAG_ID_AND_DISTANCE)
};

/**
 * Lit le message passé en paramètre et extrait les informations utiles en les stockant dans la variable output passée en paramètre
 * @param message Référence de la variable contenant le message précédemment reçu
 * @param output Référence de la variable chargée de stocker les données extraites
 * @return Renvoie true si le message a bien été décodé, false sinon
 */
bool decodeCanMessage(const twai_message_t &message, DecodedData &output);

/**
 * Vérifie si un message a été reçu dans l'intervalle de temps DATA_RECEPTION_TIME et si oui, le stocke dans la variable passée en paramètre
 * @param messageRecu Référence de la variable qui contiendra le message reçu
 * @return Renvoie true si un message a bien été reçu, false sinon
 */
bool receiveCanMessage(twai_message_t &messageRecu);

/**
 * Permet d'envoyer un message depuis une Ancre au Hub contenant l'identifiant d'un Tag avec sa distance par rapport à l'Ancre qui a calculé cette distance
 * @param id_ancre Identifiant de l'Ancre par rapport à laquelle la distance au Tag a été calculée
 * @param id_tag Identifiant du Tag dont on veut envoyer la distance par rapport à l'Ancre passée en paramètre
 * @param dist Distance entre le Tag et l'Ancre passée en paramètre
 */
void sendCanDistance(uint8_t id_ancre, uint8_t id_tag, float dist);

/**
 * Permet d'envoyer un message, contenant un identifiant, entre le Hub et l'Ancre dans les 2 sens.
 * @param pModuleId Identifiant d'une Ancre ou d'un Tag que l'on veut envoyer
 */
void sendCanSignal(uint8_t pModuleId);

/**
 * Permet d'envoyer un message contenant un ordre et une structure de données quelconque
 * depuis le Hub à l'Ancre dont l'identifiant est passé en paramètre.
 * * @tparam T Le type de la structure passée en paramètre (déduit automatiquement)
 * @param id_ancre Identifiant de l'Ancre à qui le Hub veut envoyer un ordre
 * @param pOrderType Spécifie le type d'ordre (ex: HUB_ORDER_TOGGLE_MODULE_MODE)
 * @param pOrderData La structure contenant les données spécifiques à cet ordre
 */
template <typename T>
void sendCanOrderFromHubTo(uint8_t id_ancre, uint8_t pOrderType, const T& pOrderData)
{
    twai_message_t message;
    
    // L'identifiant CAN est construit à partir du type d'ordre général + l'ID de l'ancre cible
    message.identifier = MESSAGE_HUB_ORDER + id_ancre;
    message.extd = 0;
    message.rtr = 0;
    
    // La taille s'adapte automatiquement à la structure injectée + 1 octet pour le sous-type d'ordre
    message.data_length_code = sizeof(pOrderType) + sizeof(T); 

    // Sécurité : Le bus CAN classique est limité à 8 octets de données max par trame
    if (message.data_length_code > 8) {
        Serial.printf("Erreur : La structure d'ordre est trop grande (%d octets) pour le bus CAN\n", message.data_length_code);
        return;
    }

    // On copie d'abord le type d'ordre spécifique au premier octet (index 0)
    message.data[0] = pOrderType;

    // On copie ensuite la structure de données juste après (à partir de l'index 1)
    memcpy(&(message.data[1]), &pOrderData, sizeof(T));

    // Envoi sur le bus CAN
    if (twai_transmit(&message, pdMS_TO_TICKS(DATA_TRANSMISSION_TIME)) == ESP_OK) {
        Serial.printf("Ordre de type %d envoye avec succes a l'Ancre %d\n", pOrderType, id_ancre);
    } else {
        Serial.printf("Echec de l'envoi de l'ordre a l'Ancre %d\n", id_ancre);
    }
}

/**
 * Initialise le canal de communication CAN en mode bidirectionnelle
 * @param pRXPin Pin utilisé pour recevoir des données avec le CAN
 * @param pTXPin Pin utilisé pour envoyer des données avec le CAN
 */
void initCan(int pRXPin, int pTXPin);