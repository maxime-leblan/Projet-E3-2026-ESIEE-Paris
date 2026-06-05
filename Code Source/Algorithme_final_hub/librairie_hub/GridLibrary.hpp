#pragma once

#include <algorithm>

#include "../../Algorithme trilatération/UWBModuleList.h"

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