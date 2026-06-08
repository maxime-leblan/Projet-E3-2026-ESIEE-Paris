#pragma once

#ifdef ARDUINO
    #include <ArduinoEigenDense.h>
#else
    #include <Eigen/Core>
    #include <Eigen/Dense>
#endif

#include <string>

#include "UWBModuleList.h"

using namespace Eigen;

/*
Matrice contenant les coeffs du membre gauche des équations du système linéaire des équations des sphères de triangulations avec 3 inconnues
*/
extern Matrix<float, 6, 3> gA;
/*
Matrice contenant les coeffs du membre gauche des équations du système linéaire des équations des sphères de triangulations avec 2 inconnues
*/
extern Matrix<float, 3, 2> gA2D;

/*
Matrice contenant les coeffs du membre droit des équations du système linéaire des équations des sphères de triangulations avec 3 inconnues
*/
extern Matrix<float, 6, 1> gB;
/*
Matrice contenant les coeffs du membre droit des équations du système linéaire des équations des sphères de triangulations avec 2 inconnues
*/
extern Matrix<float, 3, 1> gB2D;

/*
Calcule et renvoie l'altitude en mètre par rapport au niveau de la mer à partir de la pression atmosphérique
pAtmospheriqPression - pression atmosphérique en hPa
*/
float giveAltitude(float pAtmospheriqPression);

/*
Calcule et renvoie les coordonnées d'un module en 3D en utilisant l'algorithme de trilatération avec 4 ancres
avec en paramètre la liste des ancres (pSensors) et la liste des distances entre chaque ancre avec le module
dont on souhaite déterminer les coordonnées.
pSensors - liste des modules jouants le rôle de capteurs
pDistances - liste des distances de chaque capteur avec le module de position indéterminée
Return : les coordonnées du module dont on ne connaît pas la position
*/
V3 trilateration3D(UWBModuleList pSensors, unordered_map<int, float> pDistances);

/*
Calcule et renvoie les coordonnées d'un module en 3D en utilisant l'algorithme de trilatération avec seulement 3 ancres + l'altitude déjà connue
avec en paramètre la liste des ancres (pSensors) et la liste des distances entre chaque ancre avec le module
dont on souhaite déterminer les coordonnées.
pSensors - liste des modules jouants le rôle de capteurs
pDistances - liste des distances de chaque capteur avec le module de position indéterminée (tag)
pAltitude - altitude du tag calculée préalablement avec la pression atmosphérique mesurée par les capteurs sur l'engin et le tag
Return : les coordonnées du module dont on ne connaît pas la position
*/
V3 trilateration3D(UWBModuleList pSensors, unordered_map<int, float> pDistances, float pAltitude);

/*
Initialise les valeurs de la matrice A avec les positions des capteurs UWB
pSensors - liste des modules jouants le rôle de capteurs
*/
void initMatrixA(UWBModuleList pSensors);

/*
Initialise les valeurs de la matrice A2D avec les positions des capteurs UWB
pSensors - liste des modules jouants le rôle de capteurs
*/
void initMatrixA2D(UWBModuleList pSensors);

/*
Initialise les valeurs de la matrice B avec les positions des capteurs UWB et les distances de chaque capteur par rapport au tag
pSensors - liste des modules jouants le rôle de capteurs
pDistances - liste des distances de chaque capteur avec le module de position indéterminée
*/
void initMatrixB(UWBModuleList pSensors, unordered_map<int, float> pDistances);

/*
Initialise les valeurs de la matrice B2D avec les positions des capteurs UWB et les distances de chaque capteur par rapport au tag
pSensors - liste des modules jouants le rôle de capteurs
pDistances - liste des distances de chaque capteur avec le module de position indéterminée
pAltitude - coordonnée z du module dont la position est indéterminée
*/
void initMatrixB2D(UWBModuleList pSensors, unordered_map<int, float> pDistances, float pAltitude);