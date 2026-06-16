#ifndef IMU_SYSTEM_H
#define IMU_SYSTEM_H

#include "Fusion.h"

// Configure le bus SPI, les capteurs BMI088 et lance la tâche FreeRTOS sur le Cœur 0
void initIMUSystem();

// Tente de récupérer le dernier vecteur gravité de manière sécurisée.
// Retourne 'true' si la lecture a réussi, 'false' si la ressource était occupée.
bool getGravityVector(FusionVector* outGravity);

#endif