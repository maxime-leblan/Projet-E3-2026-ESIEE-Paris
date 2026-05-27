#pragma once

#include <string>

#include "V3.h"

/*
Classe chargée d'implémenter un item plaçable dans le repère virtuel de la zone de sécurité
*/

class UWBModule
{
    private:
    
    /*
    Nom du module
    */
    string aName;

    /*
    Position du module
    */
    V3 aPosition;

    public:

    /*
    Constructeur naturel (ne fait rien)
    */
    UWBModule();

    /*
    Constructeur par défaut de la classe. Attribut une position du module
    pPosition - position du module dans le repère
    pName - nom du module
    */
    UWBModule(string pName, V3 pPosition);

    /*
    Renvoie le nom du module
    */
    string getName() const;

    /*
    Renvoie la position du module
    */
    V3 getPosition() const;

    /*
    Modifie la position du module
    */
    void setPosition(V3 pNewPosition);
};