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
    Identifiant du module
    */
    int aId;

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
    Constructeur par défaut de la classe. Attribut un identifiant au module, et donne (0, 0, 0) comme
    position par défaut au module.
    pId - identifiant du module
    */
    UWBModule(int pId);

    /*
    Constructeur par défaut de la classe. Attribut une position et un identifiant au module
    pPosition - position du module dans le repère
    pId - identifiant du module
    */
    UWBModule(int pId, V3 pPosition);

    /*
    Renvoie l'identifiant du module
    */
    int getId() const;

    /**
     * Renvoie une chaîne de caractère permettant de représenter textuellement un module UWB
     * @return (<id>, <position>)
     */
    string toString();

    /*
    Renvoie la position du module
    */
    V3 getPosition() const;

    /*
    Modifie la position du module
    */
    void setPosition(V3 pNewPosition);
};