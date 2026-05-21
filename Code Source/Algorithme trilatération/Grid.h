#pragma once

#include <vector>

#include "GridItem.h"

/*
Classe chargée de gérer le repère de coordonnées permettant de situer les tags par rapport aux capteurs
*/
class Grid
{
    private:
    /*
    Hauteur de la grille (axe Y)
    */
    int aHeight;

    /*
    Largeur de la grille (axe X)
    */
    int aWidth;

    /*
    Matrice 2D symbolisant la grille
    */
   vector<GridItem> aGrid;

    public:

    /*
    Constructeur par défaut de la classe, crée une grille vide dont la taille est passée en paramètre
    pHeight - hauteur de la grille (axe Y)
    pWidth - largeur de la grille (axe X)
    */
   Grid(int pHeight, int pWidth);
};