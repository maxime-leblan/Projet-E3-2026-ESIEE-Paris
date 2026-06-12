#include "HubDataStorage.hpp"

//Hugues : Test communications via WiFi.
#include <WiFi.h>
#include <HTTPClient.h>
//\Hugues

#include "Trilateration.h"
#include "GridLibrary.hpp"
#include "Polygone.hpp"

void setup() {
  // Initialisation du port série à la vitesse configurée dans platformio.ini
  Serial.begin(115200);
  delay(2000); 

  // On vérifie que la PSRAM est bien active
  initHub();

  // On crée les variables qui vont stocker toutes les informations concernant les ancres, les tags et la zone de sécurité
  // On commence par les ancres
  UWBModuleList vAnchors = initAnchors("Anchors");

  // Puis on crée la liste des tags (vide pour le moment)
  UWBModuleList vTags;

  // On déclare la variable stockant la zone de sécurité
  Polygone vSafeZone = Polygone(0, initSafeZone("SafeZone"));


  // PARTIE DE HUGUES en dessous de cette ligne de code
  WiFi.begin("MaTouch_Radar", "12345678");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  //\Hugues

}

void loop()
{

  /*
  Serial.println("L'ESP32 WROVER communique parfaitement avec PlatformIO.");
  delay(5000); // Attend 5 secondes
  */
}