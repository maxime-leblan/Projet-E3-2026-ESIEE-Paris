#pragma once

#include <esp_now.h>
/*
Lien pour le code de communication Wifi entre 2 ESP32 : 
https://www.aranacorp.com/fr/creer-un-reseau-desp32-avec-esp-now/
*/

#include <esp_now.h>// https://github.com/espressif/esp-idf/blob/master/components/esp_wifi/include/esp_now.h
#include <WiFi.h>
#include "HubDataStorage.hpp"

// --- TYPES DE MESSAGES ---
enum MessageType {
    MSG_INIT = 0,
    MSG_DISTANCES = 1,
};

// --- MESSAGE 1 : ANCRE -> HUB (Réception) ---
typedef struct __attribute__((packed)) MessageAncreHub {
    uint8_t type;           // MSG_INIT ou MSG_DISTANCES
    int tag_id;             // L'ID du Tag (ex: 0, 1)
    float distances[4];     // Les distances en mètres
} MessageAncreHub;

// --- MESSAGE 2 : HUB -> ANCRE (Envoi d'ordres) ---
typedef struct __attribute__((packed)) MessageHubAncre {
    uint8_t command;        // 0 = Reste Ancre, 1 = Deviens un Tag, 2 = Initiale ton ID
    int uwb_id;             // Le numéro UWB à utiliser (0 à 3)
} MessageHubAncre;

// --- STRUCTURE DU REGISTRE --- (Contient l'ensemble des adresses MAC des ancres )
struct RegistreAncre {
    uint8_t mac[6];      
    int uwb_id;          
    bool estEnregistree;
};

// --- PROTOTYPES DES FONCTIONS ---
void initWifi();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

// Permet d'envoyer un ordre à une ancre spécifique (ex: deviens un Tag)
void envoyerOrdreChangementRole(int pUwbId, uint8_t pCommand);




