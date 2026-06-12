#pragma once

#include <esp_now.h>// https://github.com/espressif/esp-idf/blob/master/components/esp_wifi/include/esp_now.h
#include <WiFi.h>

const char nom[10]="Master"; 
uint8_t broadcastAddress[4][6] = {
  {0x2C, 0xF4, 0x32, 0x15, 0x52, 0x22}, //station0
  {0xA0, 0x20, 0xA6, 0x08, 0x20, 0xD9},  //station1
  {0xA0, 0x20, 0xA6, 0x08, 0x20, 0xD9},  //station2
  {0xA0, 0x20, 0xA6, 0x08, 0x20, 0xD9},  //station3
};// REPLACE WITH RECEIVER MAC ADDRESS

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  String d;
  bool e;
} struct_message;
struct_message myData;
struct_message dataRcv;

unsigned long previousTime=0;

void initWifi();

// callbacks for sending and receiving data
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);