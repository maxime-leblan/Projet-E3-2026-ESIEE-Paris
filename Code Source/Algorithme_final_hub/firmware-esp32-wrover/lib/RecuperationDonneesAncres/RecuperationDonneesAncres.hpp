#pragma once

#include <Arduino.h>
#include <vector>
#include "Config.hpp"

struct DistanceMoyennes {
    int tag_id;
    float distances[4];
};

class RecuperationDonneesAncres {
    
    private: 
        struct Accumulateur {
            float sommeDistances[4];
            int nbEchantillons[4];
            bool actif;
        };

        Accumulateur accumulateurs[MAX_TAGS];

        unsigned long chronoFenetre;
    
    public:
        RecuperationDonneesAncres();

        //Envoyer les données brutes.
        void injecterDonnee(int tagId, float d0, float d1, float d2, float d3);

        bool getDonneesLissees(std::vector<DistanceMoyennes>& resultats_out);
};