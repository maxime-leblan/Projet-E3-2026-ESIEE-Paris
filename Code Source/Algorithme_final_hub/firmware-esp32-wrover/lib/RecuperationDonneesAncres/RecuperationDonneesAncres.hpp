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
   
    public:
        RecuperationDonneesAncres();

        // Ajoute les données brutes lues par l'ancre en temps réel
        void injecterDonnee(int tagId, float d0, float d1, float d2, float d3);

        // NOUVEAU : Récupère la moyenne calculée pour UN SEUL tag (idéal pour le polling du Hub)
        // Renvoie true si des données étaient disponibles, false sinon.
        bool getDonneesLisseesPourTag(int tagId, DistanceMoyennes& resultat_out);

        // OPTIONNEL : Si tu as besoin de tout vider/récupérer d'un coup
        bool getToutesDonneesLissees(std::vector<DistanceMoyennes>& resultats_out);

        void effacerDonneesTag(int tagId);
};

