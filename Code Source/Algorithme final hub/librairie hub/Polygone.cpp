#include "Polygone.hpp"

// Constructeurs
Polygone::Polygone() : aId(0) {}

Polygone::Polygone(int pId, const std::vector<V3>& pPoints) : aId(pId), aPoints(pPoints) {}

// Vérification d'inclusion 2D (Ray-Casting) via les accesseurs
bool Polygone::isInside(V3 pPoint) 
{
    if (aPoints.size() < 3) return false;

    bool inside = false;
    size_t numPoints = aPoints.size();

    for (size_t i = 0, j = numPoints - 1; i < numPoints; j = i++) {
        const V3& pI = aPoints[i];
        const V3& pJ = aPoints[j];

        // Utilisation directe des accesseurs au moment du calcul pour éviter le déballage de variables primitives
        if (((pI.getY() > pPoint.getY()) != (pJ.getY() > pPoint.getY())) &&
            (pPoint.getX() < (pJ.getX() - pI.getX()) * (pPoint.getY() - pI.getY()) / (pJ.getY() - pI.getY()) + pI.getX())) {
            inside = !inside;
        }
    }

    return inside;
}

// Calcul de la distance minimale en exploitant les vecteurs et les opérateurs surchargés
float Polygone::getDistanceFrom(V3 pPoint) 
{
    if (aPoints.empty()) return 0.0f;
    
    // Cas dégénéré : le polygone n'est qu'un seul point
    if (aPoints.size() == 1) {
        V3 diff = pPoint - aPoints[0]; // Utilisation de l'opérateur - surchargé
        return std::sqrt(diff.getX() * diff.getX() + diff.getY() * diff.getY());
    }

    // 1. Si le point est à l'intérieur de la surface, la distance est nulle
    if (isInside(pPoint)) {
        return 0.0f;
    }

    // 2. Sinon, on cherche la distance minimale par rapport aux segments du contour
    float minEdgeDist = -1.0f;
    size_t numPoints = aPoints.size();

    for (size_t i = 0; i < numPoints; ++i) {
        const V3& a = aPoints[i];
        const V3& b = aPoints[(i + 1) % numPoints];

        // Stockage des coordonnées du vecteur dans une seule variable V3 grâce aux opérateurs
        V3 ab = b - a; 
        V3 ap = pPoint - a;

        // Calculs de longueur en 2D via les accesseurs getX() et getY()
        float abLenSq = ab.getX() * ab.getX() + ab.getY() * ab.getY();
        float t = 0.0f;

        if (abLenSq > 0.00001f) {
            // Produit scalaire 2D restreint à X et Y
            t = (ap.getX() * ab.getX() + ap.getY() * ab.getY()) / abLenSq;
            t = std::max(0.0f, std::min(1.0f, t));
        }

        // Calcul du point le plus proche sur le segment grâce aux opérateurs + et * de V3
        V3 closestPoint = a + ab * t;

        // Distance 2D finale entre pPoint et le point le plus proche
        V3 diffClosest = pPoint - closestPoint;
        float dist = std::sqrt(diffClosest.getX() * diffClosest.getX() + diffClosest.getY() * diffClosest.getY());

        if (minEdgeDist < 0.0f || dist < minEdgeDist) {
            minEdgeDist = dist;
        }
    }

    return minEdgeDist;
}