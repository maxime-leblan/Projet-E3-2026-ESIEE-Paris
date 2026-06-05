#pragma once

#include <vector>
#include <algorithm>

#include "../../Algorithme trilatération/V3.h"

class Polygone 
{
    private:

    /*
    Identifiant du Polygone
    */
    int aId;

    /*
    Liste contenant tous les points formant le Polygone
    */
    vector<V3> aPoints;

    public:

    /*
    Constructeur naturel de la classe (ne fait rien)
    */
    Polygone();

    /*
    Constructeur par défaut de la classe
    pId - identifiant du Polygone
    pPoints - liste des points formant le polygone
    */
    Polygone(int pId, const vector<V3>& pPoints);

    /*
    Renvoie l'identifiant du polygone
    */
    int getId() const;

    /*
    Renvoie la liste des points composants le polygone
    */
    const vector<V3>& getPoints() const;

    /*
    Détermine si pPoint est à l'intérieur du Polygone
    pPoint - point dont on veut vérifier la position par rapport au polygone
    */
    bool isInside(V3 pPoint);

    /*
    Donne la distance entre le point et le polygone
    pPoint - point dont on veut obtenir la distance par rapport au polygone
    */
    float getDistanceFrom(V3 pPoint);
};