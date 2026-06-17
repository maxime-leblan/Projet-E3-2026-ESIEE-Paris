#include <Arduino.h>
#include "CapteurPression.h" // On inclut TA bibliothèque

#define BMP_CS 10

// On crée l'objet à partir de ta bibliothèque
CapteurPression monCapteur(BMP_CS); 

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
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