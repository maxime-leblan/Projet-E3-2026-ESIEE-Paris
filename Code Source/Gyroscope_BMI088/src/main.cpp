#include <Arduino.h>
#include "gravity_IMU.h"

void setup() {
  Serial.begin(115200);
  while(!Serial) {}
  delay(1000);

  initIMUSystem();
  
  Serial.println("--- Systeme Central Demarre ---");
  Serial.println("Tapez 'etalon' pour calibrer l'assiette du vehicule.");
  Serial.println("Tapez 'annuler etalon' pour revenir a la valeur brute.");
}

void loop() {

  // 1. Écoute du Moniteur Série (Non bloquant)
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "etalon") {
      setTareCalibration();
      Serial.println("\n[SYSTEME] ---> VECTEUR DE REFERENCE APPLIQUE <---");
    } 
    else if (command == "annuler etalon") {
      clearTareCalibration();
      Serial.println("\n[SYSTEME] ---> ETALONNAGE ANNULE <---");
    }
  }

  // 2. Lecture du Vecteur de Gravité
  FusionVector currentGravity;

  if (getGravityVector(&currentGravity)) {
    Serial.printf("X: % .3f, Y: % .3f, Z: % .3f\n", 
                  currentGravity.axis.x, 
                  currentGravity.axis.y, 
                  currentGravity.axis.z);
  } else {
    Serial.println("Ressource IMU temporairement indisponible...");
  }
  delay(100);
}