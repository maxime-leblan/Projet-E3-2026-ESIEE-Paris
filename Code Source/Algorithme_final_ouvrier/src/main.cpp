#include <Arduino.h>
#include "CapteurPression.h"

// Sur la XIAO, on a branché le CS sur la broche D3
#define BMP_CS 3 

// On crée l'objet capteur
CapteurPression monCapteur(BMP_CS); 

void setup() {
  Serial.begin(115200);
  
  // Les XIAO démarrent très vite, on attend que le port série soit prêt
  while (!Serial) delay(10);
  
  Serial.println("\nDémarrage sur carte XIAO...");

  if (!monCapteur.initialiser()) {
    Serial.println("Erreur : Capteur introuvable ! Vérifiez le câblage SPI.");
    while(1) delay(10);
  }
  
  Serial.println("Capteur détecté avec succès !");
}

void loop() {
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