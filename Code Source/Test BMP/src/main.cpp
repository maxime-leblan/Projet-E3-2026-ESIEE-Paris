#include <Arduino.h>
#include <SPI.h> // <-- L'ajout crucial pour que la carte active ses broches 11, 12 et 13
#include "CapteurPression.h"

#define BMP_CS 10

// On crée l'objet à partir de ta bibliothèque
CapteurPression monCapteur(BMP_CS); 

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  // <-- On force le démarrage du bus matériel SPI avant tout le reste
  SPI.begin(); 
  
  Serial.println("Démarrage...");

  // On appelle ta fonction d'initialisation
  if (!monCapteur.initialiser()) {
    Serial.println("Erreur : Capteur introuvable !");
    while(1) delay(10);
  }
}

void loop() {
  // On appelle ta fonction de lecture
  float pressionActuelle = monCapteur.getPression();

  if (pressionActuelle != -1.0) {
    Serial.print("Pression : ");
    Serial.print(pressionActuelle, 4);
    Serial.println(" hPa");
  } else {
    Serial.println("Erreur de lecture.");
  }

  delay(1000);
}