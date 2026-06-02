#pragma once

#include <vector>
#include <algorithm>

#include "../../Algorithme trilatération/V3.h"

class Polygone 
{
private:
    std::vector<V3> aPoints;
    int aId;

public:
    // Constructeurs
    Polygone();
    Polygone(int pId, const std::vector<V3>& pPoints);

    // Getters et Setters
    int getId() const { return aId; }
    void setId(int pId) { aId = pId; }
    const std::vector<V3>& getPoints() const { return aPoints; }
    void setPoints(const std::vector<V3>& pPoints) { aPoints = pPoints; }

    /*
    * Détermine si pPoint est à l'intérieur du Polygone (en 2D, via x et y).
    */
    bool isInside(V3 pPoint);

    /*
    * Donne la distance entre le point et le polygone (surface comprise).
    */
    float getDistanceFrom(V3 pPoint);
};