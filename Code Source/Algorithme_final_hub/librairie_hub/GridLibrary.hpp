#pragma once

#include <algorithm>
    
#ifdef ARDUINO
    #include <UWBModuleList.h>
    #include <ArduinoEigenDense.h>
#else
    #include "../../Algorithme-trilateration/UWBModuleList.h"
    #include <Eigen/Core> // taper dans le terminal "dpkg -L libeigen3-dev" pour trouver l'emplacement de la librairie
    #include <Eigen/Dense>
#endif

/*
Renvoie la matrice de rotation pour transformer le vecteur pStartVector en pResultVector
pStartVector - vecteur d'origine
pResultVector - vecteur final que l'on obtient après avoir multiplié pStartVector par la matrice de rotation renvoyée par la fonction
*/
Eigen::Matrix<float, 3, 3> giveRotationalMatrix(V3 pStartVector, V3 pResultVector);

/*
Renvoie une copie de la liste des points après application de la matrice de rotation
*/
vector<V3> applyRotationOnPoints(vector<V3> pPoints, Eigen::Matrix<float, 3, 3> pRotationalMatrix);

/*
Attribue aux 4 ancres virtuelles des coordonnées à partir des distances entre chaque ancre réelle. 
Pour cela, la fonction utilise l'algorithme MDS (Multidimensional Scaling).
pAnchors - liste des ancres virtuelles
pDistances - liste de toutes les distances entre chaque ancre
*/
void initAnchorsCoordinates(UWBModuleList & pAnchors, unordered_map<string, float> pDistances);

/*
Attribue aux 4 ancres virtuelles des coordonnées à partir des distances entre chaque ancre réelle en utilisant
l'algorithme de la descente de gradient.
pAnchors - liste des ancres virtuelles
pDistances - liste de toutes les distances entre chaque ancre
pIter - nombre d'itérations de la descente de gradient
pAlpha - vitesse d'apprentissage
*/
void initAnchorsCoordinatesWithGD(UWBModuleList & pAnchors, unordered_map<string, float> pDistances, int pIter, float pAlpha);