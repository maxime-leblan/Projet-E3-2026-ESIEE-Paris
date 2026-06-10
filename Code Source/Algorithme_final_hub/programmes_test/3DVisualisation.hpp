#pragma once

#include <cmath> // Nécessaire pour les fonctions sinf() et cosf()
#include <vector>

#include "raylib.h"
#include "../../Algorithme-trilateration/V3.h"

using namespace std;

/*
Prend une liste de points et renvoie une liste de vecteurs reliant chaque point de la liste de manière circulaire (1 avec 2, 2 avec 3 ... n-1 avec n, n avec 1)
pPoints - liste des points correspondants aux extrémités des vecteurs
*/
vector<vector<V3>> buildVectorsListFor3DEnvironment(vector<V3> pPoints);

/*
Concatène les 2 listes de vecteurs passées en paramètre et renvoie le résultat
pList1 - liste 1 à concaténer
pList2 - liste 2 à concaténer
*/
vector<vector<V3>> concatenateVectorsList(vector<vector<V3>> pList1, vector<vector<V3>> pList2);

/*
Convertit l'ordre des axes de la liste de vecteurs passée en paramètre de la convention mathématique à la convention graphique d'OpenGL
pPointsList - liste de vecteurs dont l'on veut changer l'ordre des coordonnées
*/
vector<vector<V3>> convertVectorsListAxisToRayLib(vector<vector<V3>> pVectorsList);

/*
Concatène les 2 listes de points passées en paramètre et renvoie le résultat
pList1 - liste 1 à concaténer
pList2 - liste 2 à concaténer
*/
vector<V3> concatenatePointsList(vector<V3> pList1, vector<V3> pList2);

/*
Convertit l'ordre des axes de la liste de points passée en paramètre de la convention mathématique à la convention graphique d'OpenGL
pPointsList - liste de points dont l'on veut changer l'ordre des coordonnées
*/
vector<V3> convertPointsListAxisToRayLib(vector<V3> pPointsList);

/*
Convertit l'ordre des axes du point passé en paramètre de la convention mathématique à la convention graphique d'OpenGL
pPoint - point dont l'on veut changer l'ordre des coordonnées
*/
V3 convertPointAxisToRayLib(V3 pPoint);

/*
Prend en paramètre une liste de coordonnées de points et de vecteurs et les affiche dans une fenêtre représentant un repère orthonormé en 3D
pPoints - liste des coordonnées des points
pVectors - liste des coordonnées des vecteurs, qui sont chacuns représentés par une liste contenant 2 points qui correspondent aux 2 extremitées de chaque vecteur
*/
void view3DEnvironment(vector<V3> pPoints, vector<vector<V3>> pVectors);