#include "HubDataStorage.hpp"
#include "WifiMessageManager.hpp"
#include <ArduinoEigenDense.h>
#include <ArduinoJson.h>

//Hugues : Test communications via WiFi.
#include <WiFi.h>
#include <HTTPClient.h>
//\Hugues

#include "Trilateration.h"
#include "GridLibrary.hpp"
#include "Polygone.hpp"

//MODE DE COMMUNICATION :
enum ModeCommunication {
  MODE_WIFI, // Sans fils
  MODE_WIRED // UART / CAN
};
// --- MODE ACTUEL DE COMMUNICATION !!! ---
ModeCommunication modeActuel = MODE_WIFI;

void setup() {
  // Initialisation du port série à la vitesse configurée dans platformio.ini
  Serial.begin(115200);
  delay(2000); 

  // On vérifie que la PSRAM est bien active
  initHub();

  // On crée les variables qui vont stocker toutes les informations concernant les ancres, les tags et la zone de sécurité
  // On commence par les ancres
  UWBModuleList vAnchors = initAnchors("Anchors");
  Serial.println(("Contenu de la liste des ancres :\n" + vAnchors.toString()).c_str());

  // Puis on crée la liste des tags (vide pour le moment)
  UWBModuleList vTags;

  // On déclare la variable stockant la zone de sécurité
  Polygone vSafeZone = Polygone(0, initSafeZone("SafeZone"));
  Serial.println(("Contenu de la zone de sécurité :\n" + vSafeZone.toString()).c_str());
  /*
  // PARTIE DE HUGUES en dessous de cette ligne de code
  WiFi.begin("MaTouch_Radar", "12345678");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  */
  Serial.println("Adresse MAC :");
  Serial.println(WiFi.macAddress());
  initWifi();
  //\Hugues
  
}

void loop() {
  /*
  // Code de test pour communiquer avec les modules UWB
  // Toute les 5 secondes on envoie un message
  if((millis() -previousTime)>5000){
    // Set values to send
    strcpy(myData.aSenderName, nom);
    
  
    esp_err_t result;
    // Send message via ESP-NOW
    for (int i = 0; i < ANCHORS_NUMBER; i++)
    {
      myData.aModuleId = i;
      myData.aMessage = "Slave" + String(i);
      sendData(i);
    }
    previousTime=millis();
  }
  */
}