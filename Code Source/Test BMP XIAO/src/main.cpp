#include <Arduino.h>
#include "CapteurPression.h"
#include <Adafruit_TinyUSB.h>

// D'après ton schéma de câblage, le Chip Select (CS) est sur D3
#define BMP_CS D3 

CapteurPression monCapteur(BMP_CS); 

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Démarrage du système BMP581...");
  SPI.begin(); 

  while(!monCapteur.initialiser()) {
    delay(1000); 
    Serial.println("Erreur : Capteur BMP581 introuvable. Vérifiez le câblage SPI !");
  }
  Serial.println("Capteur initialisé avec succès.");
}

void loop() {
  float pressionActuelle = monCapteur.getPression();

  if (pressionActuelle != -1.0) {
    Serial.print("Pression : ");
    Serial.print(pressionActuelle, 2); // 2 décimales
    Serial.println(" hPa");
  } else {
    Serial.println("Erreur de lecture SPI.");
  }

  delay(1000);
}