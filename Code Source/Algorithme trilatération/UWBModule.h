#pragma once

#include "V3.h"

/*
Classe chargée d'implémenter un item plaçable dans le repère virtuel de la zone de sécurité
*/

class UWBModule
{
    private:
    
    /*
    Nom de l'item
    */
    string aName;

    /*
    Position de l'item sur la grille
    */
    V3 aPosition;

    public:

    /*
    Constructeur par défaut de la classe. Attribut une position à l'item
    pPosition - position de l'item dans le repère
    pName - nom de l'item
    */
    UWBModule(string pName, V3 pPosition);

    /*
    Renvoie la position de l'item
    */
    V3 getPosition() const;

    /*
    Modifie la position de l'item
    */
    void setPosition(V3 pNewPosition);
};