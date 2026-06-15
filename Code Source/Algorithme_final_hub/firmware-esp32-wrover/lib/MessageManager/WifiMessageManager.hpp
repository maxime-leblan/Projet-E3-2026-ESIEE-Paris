#pragma once

/*
Lien pour le code de communication Wifi entre 2 ESP32 : 
https://www.aranacorp.com/fr/creer-un-reseau-desp32-avec-esp-now/
*/

#include <esp_now.h>// https://github.com/espressif/esp-idf/blob/master/components/esp_wifi/include/esp_now.h
#include <WiFi.h>

#include "../hubDataStorage/HubDataStorage.hpp"

const char nom[10]="Master"; 
uint8_t broadcastAddress[ANCHORS_NUMBER][6] = {
  {0x2C, 0xF4, 0x32, 0x15, 0x52, 0x22}, //station0
  {0xA0, 0x20, 0xA6, 0x08, 0x20, 0xD9},  //station1
  {0xA0, 0x20, 0xA6, 0x08, 0x20, 0xD9},  //station2
  {0xA0, 0x20, 0xA6, 0x08, 0x20, 0xD9},  //station3
};// REPLACE WITH RECEIVER MAC ADDRESS

// Structure example to send data
// Must match the receiver structure
typedef struct Message {
  String aMessage;
  int aModuleId;
  char* aSenderName;
} Message;


// Variable contenant le message envoyé par le hub
Message myData;
// Variable contenant le dernier message reçu par le hub
Message dataRcv;


// Register peer
esp_now_peer_info_t peerInfo;

unsigned long previousTime=0;

void initWifi();

// callbacks for sending and receiving data
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

/**
 * Envoie le message stockée dans la variable globale myData au destinataire ayant l'adresse MAC associée à l'identifiant passé en paramètre.
 * @param pMACAdressId Identifiant du tableau global broadcastAddress correspondant à l'adresse MAC du destinataire
 */
void sendDataWifi(int pMACAdressId);