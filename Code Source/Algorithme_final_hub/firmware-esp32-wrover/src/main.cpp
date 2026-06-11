#include <HubDataStorage.hpp>

#include "../../Algorithme-trilateration/Trilateration.h"
#include "../../Algorithme_final_hub/librairie_hub/GridLibrary.hpp"
#include "../../Algorithme_final_hub/librairie_hub/Polygone.hpp"

void setup() {
  // Initialisation du port série à la vitesse configurée dans platformio.ini
  Serial.begin(115200);
  delay(2000); 

  // On vérifie que la PSRAM est bien active
  initHub();

  // On crée les variables qui vont stocker toutes les informations concernant les ancres, les tags et la zone de sécurité
  // On commence par les ancres
  UWBModuleList vAnchors = initAnchors();

  // Puis on crée la liste des tags (vide pour le moment)
  UWBModuleList vTags;

  // On déclare la variable stockant la zone de sécurité
  Polygone vSafeZone;


  // PARTIE DE HUGUES en dessous de cette ligne de code

}

void loop()
{

  /*
  Serial.println("L'ESP32 WROVER communique parfaitement avec PlatformIO.");
  delay(5000); // Attend 5 secondes
  */
}