#pragma once

#include <vector>
#include <cmath>

#ifdef ARDUINO
    #include <ArduinoEigenDense.h>
    #include <V3.h>
#else
    #include <Eigen/Core>
    #include <Eigen/Dense>
    #include "../../Algorithme-trilateration/V3.h"
#endif

// Ajout de la classe Polygone
#include "Polygone.hpp"

namespace LissageVehicule {

    // Représente le repère local de notre "Plan Moyen"
    struct PlanLocal {
        V3 centre;      // Le centre de gravité de tous les points
        V3 normale;     // Le vecteur perpendiculaire au plan
        V3 axeU;        // Le nouvel axe X local du plan
        V3 axeV;        // Le nouvel axe Y local du plan
        
        // Les coefficients de l'équation cartésienne : ax + by + cz + d = 0
        float a, b, c, d;
    };

    /**
     * FONCTION 1 : Trouve le plan moyen 2D dans un espace 3D 
     * à partir d'un nuage de points très dense.
     */
    PlanLocal calculerPlanMoyen(const std::vector<V3>& points3D);

    /**
     * FONCTION 2 : Projette les points sur le plan moyen, calcule le périmètre,
     * génère 64 points espacés uniformément, les repasse en 3D et crée le Polygone !
     */
    Polygone echantillonner64Points(int pId, const PlanLocal& plan, const std::vector<V3>& points3D);

}