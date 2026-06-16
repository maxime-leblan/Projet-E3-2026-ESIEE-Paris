#pragma once
#include <Arduino.h>

// --- ADRESSE MAC DU HUB ---
// L'Ancre doit connaître cette adresse pour savoir vers qui envoyer ses paquets
const uint8_t HUB_MAC_ADDRESS[] = {0x68, 0xFE, 0x71, 0x92, 0x6F, 0x10};

// --- TYPES DE MESSAGES ---
enum MessageType {
    MSG_INIT = 0,
    MSG_DISTANCES = 1,
};

// --- STRUCTURES DES PAQUETS (Partagées) ---

// Le colis que l'Ancre ENVOIE au Hub
typedef struct __attribute__((packed)) MessageAncreHub {
    uint8_t type;           // MSG_INIT ou MSG_DISTANCES
    int tag_id;             // L'ID du Tag détecté (ex: 0 ou 1)
    float distances[4];     // Les 4 distances converties en mètres
} MessageAncreHub;

// Le ordre que l'Ancre REÇOIT du Hub
typedef struct __attribute__((packed)) MessageHubAncre {
    uint8_t command;        // 0 = Reste Ancre, 1 = Deviens un Tag, 2 = Initialise ton ID
    int uwb_id;             // L'ID UWB que le Hub attribue à cette Ancre (0, 1, 2 ou 3)
} MessageHubAncre;

// --- PROTOTYPE DE LA FONCTION DE PARSING ---
// Spécifique à l'Ancre pour traduire la String UWB en structure MessageAncreHub
bool parseUWBMessage(String rawData, MessageAncreHub& outMessage);

