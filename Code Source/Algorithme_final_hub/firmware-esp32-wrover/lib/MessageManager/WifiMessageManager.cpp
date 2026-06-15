#include "WifiMessageManager.hpp"

void initWifi()
{
    // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("Erreur d'initialisation ESP-NOW"));
    return;
  }
  Serial.print(F("Receveur initialisé : "));
  Serial.println(WiFi.macAddress());
  
  // Define callback functions
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  for (int i = 0; i < ANCHORS_NUMBER; i++)
  {
    memcpy(peerInfo.peer_addr, broadcastAddress[i], 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println(("Echec de l'ajout de l'adresse MAC " + to_string(i)).c_str());
      return;
    }
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print(F("\r\nPaquet maître envoyé :\t"));
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Délivré avec succès" : "Echec de transmission");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&dataRcv, incomingData, sizeof(dataRcv));
  Serial.print("\r\nOctets reçus : ");
  Serial.println(len);
  Serial.print("De l'esclave : ");
  Serial.println(dataRcv.aMessage);
  Serial.println();
}