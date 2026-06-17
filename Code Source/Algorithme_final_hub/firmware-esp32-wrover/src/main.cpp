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
#include "RecuperationDonneesAncres.hpp"

RecuperationDonneesAncres recupDonnees;

void ecouterReseauFilaire() {
  if (MODE_ACTUEL == MODE_WIRED) {
    // MAXIME
  }
}

void OnWifidataReceived(int tagId, float d0, float d1, float d2, float d3) {
  if (MODE_ACTUEL == MODE_WIFI) {
    recupDonnees.injecterDonnee(tagId, d0, d1, d2, d3);
  }
}


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

// Machine Etat Normal :
void gererFonctionnementNormal() {

  ecouterReseauFilaire(); // Ne fait rien si le mode wifi est choisi

  std::vector<DistanceMoyennes> tagsPretsPourMaths;

  if (recupDonnees.getDonneesLissees(tagsPretsPourMaths)) {
    for (const DistanceMoyennes& tag : tagsPretsPourMaths) {
      Serial.printf("[30Hz] Tag %d |D0:%.2f | D1:%2.f | D2:%2.f | D3:%2.f\n", tag.tag_id, tag.distances[0], tag.distances[1], tag.distances[2], tag.distances[3]);
    }
  }
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