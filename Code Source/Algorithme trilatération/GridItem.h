#pragma once

#include "V2.h"

/*
Classe chargée d'implémenter un item plaçable dans une grille
*/

class GridItem
{
    private:
    
    /*
    Position de l'item sur la grille
    */
    V2 aPosition;

    public:

    /*
    Constructeur par défaut de la classe. Attribut une position à l'item
    pPosition - position de l'item sur la grille
    */
    GridItem(V2 pPosition);
};