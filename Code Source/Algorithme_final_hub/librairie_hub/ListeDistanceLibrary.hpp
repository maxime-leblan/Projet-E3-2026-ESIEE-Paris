#pragma once

#include <unordered_map>
#include <string>

#ifdef ARDUINO
    #include <Trilateration.h>
#else
    #include "../../Algorithme-trilateration/Trilateration.h" // Contient V3, UWBModuleList, etc.
#endif

#include "Polygone.hpp"      // Ta classe Polygone

namespace ListeDistance {

    /*
     * Calcule la distance de chaque tag par rapport au rectangle formé par les capteurs
     * et génère la chaîne JSON.
     * pTagsPositions - Map contenant l'ID du tag et sa position 3D (V3)
     * pRectangleCapteurs - Le polygone 2D construit avec les positions des capteurs
     * Return : Une String contenant le JSON ex: [{"id":101, "dist":3.25}, {"id":102, "dist":0.00}]
     */
    String generateTagsDistanceJson(const std::unordered_map<int, V3>& pTagsPositions, Polygone& pRectangleCapteurs);

    /*
     * Construit le polygone à partir de la liste des capteurs, calcule les distances,
     * et envoie le résultat sur le port Série pour l'écran.
     */
    void sendDistancesToScreen(const std::unordered_map<int, V3>& pTagsPositions, const UWBModuleList& pSensors);

}

