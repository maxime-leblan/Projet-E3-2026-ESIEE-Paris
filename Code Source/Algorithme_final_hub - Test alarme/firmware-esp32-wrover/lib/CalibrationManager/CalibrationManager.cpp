#include "CalibrationManager.hpp"
#include "ListeDistanceLibrary.hpp"

CalibrationManager::CalibrationManager() : centreEpicentre(0, 0, 0) {}

void CalibrationManager::initialiserEpicentre(UWBModuleList& ancres) {
    centreEpicentre = ListeDistance::obtenirCentreAncres(ancres);
}

V3 CalibrationManager::getEpicentre() const {
    return centreEpicentre;
}

void CalibrationManager::ajouterPoint(V3 pointCalcule) {
    pointsTemporaires.push_back(pointCalcule);
}

const std::vector<V3>& CalibrationManager::getPoints() const {
    return pointsTemporaires;
}

int CalibrationManager::getNombrePoints() const {
    return pointsTemporaires.size();
}

void CalibrationManager::viderPoints() {
    pointsTemporaires.clear();
}
