#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>

#ifdef ARDUINO
    #include <UWBModuleList.h>
    #include <V3.h>
#else
    #include "../../Algorithme-trilateration/UWBModuleList.h"
    #include "../../Algorithme-trilateration/V3.h"
#endif

namespace ListeDistance {

    // Structure pour lier un Tag à sa distance calculée par rapport au centre des ancres
    struct TagTrie {
        int id;
        float distanceAuCentre;
        V3 position;
    };

    /**
     * Calcule le centre géométrique des 4 ancres en faisant la moyenne de leurs coordonnées.
     */
    V3 obtenirCentreAncres(UWBModuleList& pAncres);

    /**
     * Calcule la distance de chaque tag par rapport au centre des ancres,
     * trie la liste du plus proche au plus lointain, et l'envoie via UART (Serial).
     */
    void envoyerDistancesTrieesUART(
        UWBModuleList& pAncres, 
        const std::unordered_map<int, V3>& pPositionsTags
    );

}