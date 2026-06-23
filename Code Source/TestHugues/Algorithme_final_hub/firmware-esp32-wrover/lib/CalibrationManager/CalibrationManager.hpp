#pragma once

#include <vector>
#include "V3.h"
#include "UWBModuleList.h"

class CalibrationManager {
private:
    std::vector<V3> pointsTemporaires;
    V3 centreEpicentre;

public:
    CalibrationManager();

    // Gestion de l'épicentre temporaire
    void initialiserEpicentre(UWBModuleList& ancres);
    V3 getEpicentre() const;

    // Gestion de la collecte de points (RAM)
    void ajouterPoint(V3 pointCalcule);
    const std::vector<V3>& getPoints() const;
    int getNombrePoints() const;
    void viderPoints();
};
