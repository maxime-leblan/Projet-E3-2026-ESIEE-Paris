#include "RecuperationDonneesAncres.hpp"

RecuperationDonneesAncres::RecuperationDonneesAncres() {
    // Plus de chronoFenetre ! Initialisation des accumulateurs à zéro :
    for (int i = 0; i < MAX_TAGS; i++) {
        memset(accumulateurs[i].sommeDistances, 0, sizeof(accumulateurs[i].sommeDistances));
        memset(accumulateurs[i].nbEchantillons, 0, sizeof(accumulateurs[i].nbEchantillons));
        accumulateurs[i].actif = false;
    }
}

void RecuperationDonneesAncres::injecterDonnee(int tagId, float d0, float d1, float d2, float d3) {
    int idx = tagId - ID_DEPART_TAGS; 
    if (idx < 0 || idx >= MAX_TAGS) return; 

    float dists[4] = {d0, d1, d2, d3};

    // On ajoute toutes les distances valides (>0)
    for (int i = 0; i<4; i++) {
        if (dists[i] > 0.0f) {
            accumulateurs[idx].sommeDistances[i] += dists[i];
            accumulateurs[idx].nbEchantillons[i]++;
            accumulateurs[idx].actif = true;
        }
    }
}

// Méthode appelée suite à la réception de HUB_ORDER_REQUEST_DISTANCES
bool RecuperationDonneesAncres::getDonneesLisseesPourTag(int tagId, DistanceMoyennes& resultat_out) {
    int idx = tagId - ID_DEPART_TAGS;
    if (idx < 0 || idx >= MAX_TAGS || !accumulateurs[idx].actif) {
        return false; // Pas de données disponibles pour ce Tag
    }

    resultat_out.tag_id = tagId;
    int ancresValides = 0;

    for (int i = 0; i<4; i++) {
        if (accumulateurs[idx].nbEchantillons[i] > 0) {
            // Calcul de la moyenne
            resultat_out.distances[i] = accumulateurs[idx].sommeDistances[i] / (float)accumulateurs[idx].nbEchantillons[i];
            ancresValides++;
        } else {
            resultat_out.distances[i] = 0.0f;
        }
    }

    // Remise à zero immédiate pour la prochaine requête de polling
    memset(accumulateurs[idx].sommeDistances, 0, sizeof(accumulateurs[idx].sommeDistances));
    memset(accumulateurs[idx].nbEchantillons, 0, sizeof(accumulateurs[idx].nbEchantillons));
    accumulateurs[idx].actif = false;

    // Tu peux ajuster cette condition si tu as besoin d'au moins 3 distances valides ou non
    return (ancresValides > 0); 
}

// Remplacement de l'ancienne méthode globale au cas où le Hub demande tout d'un coup
bool RecuperationDonneesAncres::getToutesDonneesLissees(std::vector<DistanceMoyennes>& resultats_out) {
    resultats_out.clear();

    for (int idx = 0; idx < MAX_TAGS; idx++) {
        if (!accumulateurs[idx].actif) continue;

        DistanceMoyennes resultatTag;
        // On réutilise la logique de l'autre fonction pour éviter la duplication
        if (getDonneesLisseesPourTag(idx + ID_DEPART_TAGS, resultatTag)) {
            // Vérification de sécurité (ex: si tu veux forcer 3 ancres minimum)
            int ancresValides = 0;
            for(int i=0; i<4; i++) if(resultatTag.distances[i] > 0.0f) ancresValides++;
            
            if(ancresValides >= 3) {
                resultats_out.push_back(resultatTag);
            }
        }
    }
    return !resultats_out.empty();
}

