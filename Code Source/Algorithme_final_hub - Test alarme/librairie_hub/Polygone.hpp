#pragma once

#include <vector>
#include <algorithm>

#ifdef ARDUINO
    #include <V3.h>
#else
    #include "../../Algorithme-trilateration/V3.h"
#endif


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
    Détermine si pPoint, qui sera considéré comme projeté sur le plan du polygone, est à l'intérieur du Polygone
    pPoint - point dont on veut vérifier la position par rapport au polygone
    */
    bool isInside(V3 pPoint);

    /*
    Donne la distance entre le point passé en paramètre, qui sera considéré comme projeté sur le plan du polygone, et le polygone
    pPoint - point dont on veut obtenir la distance par rapport au polygone
    */
    float getDistanceFrom(V3 pPoint);

    /**
     * Renvoie une chaîne de caractères représentant textuellement la liste des points formant le polygone
     * @return [(x_0, y_0, z_0), (x_1, y_1, z_1),...,(x_n, y_n, z_n)] ou [] si elle est vide
     */
    string toString();
};