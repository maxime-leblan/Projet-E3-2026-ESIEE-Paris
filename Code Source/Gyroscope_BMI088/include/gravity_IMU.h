#ifndef GRAVITY_IMU_H
#define GRAVITY_IMU_H

#include "Fusion.h"

// Fonction d'initialisation du système IMU (BMI088)
void initIMUSystem();

// Fonction de récupération du vecteur de gravité (Thread-Safe).
// Renvoie true si la récupération a réussi, false sinon (en cours d'écriture).
bool getGravityVector(FusionVector* outGravity);

// Fonctions d'étalonnage du vecteur de gravité (tare)
void setTareCalibration();
void clearTareCalibration();

#endif