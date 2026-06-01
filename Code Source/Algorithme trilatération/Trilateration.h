#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>
#include <string>

#include "UWBModuleList.h"

using namespace Eigen;

/*
Matrice contenant les coeffs du membre gauche des équations du système linéaire des équations des sphères de triangulations
*/
extern Matrix<float, 6, 3> gA;

/*
Matrice contenant les coeffs du membre droit des équations du système linéaire des équations des sphères de triangulations
*/
extern Matrix<float, 6, 1> gB;

/*
Calcule et renvoie les coordonnées d'un module en 3D en utilisant l'algorithme de trilatération
avec en paramètre la liste des ancres (pSensors) et la liste des distances entre chaque ancre avec le module
dont on souhaite déterminer les coordonnées.
pSensors - liste des modules jouants le rôle de capteurs
pDistances - liste des distances de chaque capteur avec le module de position indéterminée
Return : les coordonnées du module dont on ne connaît pas la position
*/
V3 trilateration3D(UWBModuleList pSensors, unordered_map<int, float> pDistances);

/*
Initialise les valeurs de la matrice A avec les positions des capteurs UWB
pSensors - liste des modules jouants le rôle de capteurs
*/
void initMatrixA(UWBModuleList pSensors);

/*
Initialise les valeurs de la matrice B avec les positions des capteurs UWB et les distances de chaque capteur par rapport au tag
pSensors - liste des modules jouants le rôle de capteurs
pDistances - liste des distances de chaque capteur avec le module de position indéterminée
*/
void initMatrixB(UWBModuleList pSensors, unordered_map<int, float> pDistances);