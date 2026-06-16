#include <Arduino.h>
#include "gravity_IMU.h" // Inclusion de ton nouveau module

void setup() {
  Serial.begin(115200);
  while(!Serial) {}
  delay(1000);

  // 1. Démarrage de la boîte noire IMU (Configure et lance le Cœur 0 en arrière-plan)
  initIMUSystem();

  Serial.println("--- Systeme Central Demarre ---");
}

void loop() {
  static unsigned long lastPrintTime = 0;

  // Boucle de traitement du Cœur 1 à 20 Hz (toutes les 50 millisecondes)
  if (millis() - lastPrintTime >= 50) {
    lastPrintTime = millis();
    
    FusionVector currentGravity;
    
    // 2. Appel de l'API de notre module pour récupérer le vecteur
    if (getGravityVector(&currentGravity)) {
      Serial.printf("X: % .3f, Y: % .3f, Z: % .3f\n", 
                    currentGravity.axis.x, 
                    currentGravity.axis.y, 
                    currentGravity.axis.z);
    } else {
      Serial.println("Ressource IMU temporairement indisponible...");
    }
  }

  // Ici viendra la logique de traitement du bus CAN pour les ancres UWB.
}