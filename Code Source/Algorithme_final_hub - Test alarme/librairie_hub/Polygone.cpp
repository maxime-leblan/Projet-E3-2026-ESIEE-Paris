#include "Polygone.hpp"

Polygone::Polygone() : aId(0) {}

Polygone::Polygone(int pId, const std::vector<V3>& pPoints) : aId(pId), aPoints(pPoints) {}

int Polygone::getId() const
{
    return aId;
}

const vector<V3>& Polygone::getPoints() const
{
    return aPoints;
}

bool Polygone::isInside(V3 pPoint) 
{
    // on teste que l'on ait bien un polygone
    if (aPoints.size() < 3) return false;

    bool vInside = false;
    size_t vNumPoints = aPoints.size();

    // On utilise l'algorithme du lancer de rayons 
    for (size_t i = 0, j = vNumPoints - 1; i < vNumPoints; j = i++) {
        const V3& vPointI = aPoints[i];
        const V3& vPointJ = aPoints[j];

        // si pour chaque arête IJ, P est compris entre I et J ou J et I selon l'axe Y, alors on rentre dans la conditionnelle
        if (((vPointI.getY() > pPoint.getY()) != (vPointJ.getY() > pPoint.getY())) &&
            (pPoint.getX() < (vPointJ.getX() - vPointI.getX()) * (pPoint.getY() - vPointI.getY()) / (vPointJ.getY() - vPointI.getY()) + vPointI.getX())) {
            vInside = !vInside;
        }
    }

    return vInside;
}

// Calcul de la distance minimale avec norm2D() et prodScal2D()
float Polygone::getDistanceFrom(V3 pPoint) 
{
    if (aPoints.empty()) return 0.0f;
    
    // Cas dégénéré : le polygone n'est qu'un seul point
    if (aPoints.size() == 1) {
        V3 vDiff = pPoint - aPoints[0];
        return vDiff.norm2D();
    }

    // Si le point est à l'intérieur de la surface, la distance est nulle
    if (isInside(pPoint)) {
        return 0.0f;
    }

    // Sinon, on cherche la distance minimale par rapport aux segments du contour
    float vMinEdgeDist = -1.0f;
    size_t vNumPoints = aPoints.size();

    for (size_t i = 0; i < vNumPoints; ++i)
    {
        const V3& vA = aPoints[i];
        const V3& vB = aPoints[(i + 1) % vNumPoints];

        V3 vAB = vB - vA; 
        V3 vAP = pPoint - vA;

        // Le carré de la norme de AB correspond au produit scalaire de AB avec lui-même
        float vAbLengthSquared = prodScal2D(vAB, vAB);
        float vT = 0.0f;

        // On vérifie 
        if (vAbLengthSquared > 0.00001f)
        {
            // On calcule t avec t représentant l'emplacement du projeté de pPoint sur AB
            vT = prodScal2D(vAP, vAB) / vAbLengthSquared;
            vT = std::max(0.0f, std::min(1.0f, vT));
        }

        // Calcul du point le plus proche sur le segment
        V3 vClosestPoint = vA + vAB * vT;

        // Distance entre pPoint et le point le plus proche sur le segment
        V3 vVectorToClosest = pPoint - vClosestPoint;
        float vDist = vVectorToClosest.norm2D();

        if (vMinEdgeDist < 0.0f || vDist < vMinEdgeDist) {
            vMinEdgeDist = vDist;
        }
    }

    return vMinEdgeDist;
}

string Polygone::toString()
{
    string vString = "[";

    for (int i = 0; i < aPoints.size(); i++)
    {
        vString += aPoints[i].toString() + ", ";
    }

    if (vString.size() > 1)
    {
        vString.pop_back();
        vString.pop_back();
    }

    return vString + "]";
}