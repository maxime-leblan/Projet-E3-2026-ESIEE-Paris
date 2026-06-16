#include <Arduino.h>
#include "gravity_IMU.h"

void setup() {
  Serial.begin(115200);
  while(!Serial) {}
  delay(1000);

  initIMUSystem();

  Serial.println("--- Systeme Central Demarre ---");
}

void loop() {
  FusionVector currentGravity;

  if (getGravityVector(&currentGravity)) {
    Serial.printf("X: % .3f, Y: % .3f, Z: % .3f\n", 
                  currentGravity.axis.x, 
                  currentGravity.axis.y, 
                  currentGravity.axis.z);
  } else {
    Serial.println("Ressource IMU temporairement indisponible...");
  }
}