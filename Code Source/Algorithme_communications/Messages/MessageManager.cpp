#include "MessageManager.hpp"

void initWifi()
{
    // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("Error initializing ESP-NOW"));
    return;
  }
  Serial.print(F("Reciever initilized : "));
  Serial.println(WiFi.macAddress());
  
  // Define callback functions
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
    
  memcpy(peerInfo.peer_addr, broadcastAddress[0], 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress[1], 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print(F("\r\nMaster packet sent:\t"));
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&dataRcv, incomingData, sizeof(dataRcv));
  Serial.print("\r\nBytes received: ");
  Serial.println(len);
  Serial.print("From slave: ");
  Serial.println(dataRcv.a);
  Serial.println();
}