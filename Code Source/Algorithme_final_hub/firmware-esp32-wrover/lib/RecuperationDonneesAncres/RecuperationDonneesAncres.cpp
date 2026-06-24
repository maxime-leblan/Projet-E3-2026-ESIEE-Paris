#include "RecuperationDonneesAncres.hpp"

RecuperationDonneesAncres::RecuperationDonneesAncres() {
    for (int i = 0; i < MAX_TAGS; i++) {
        memset(accumulateurs[i].sommeDistances, 0, sizeof(accumulateurs[i].sommeDistances));
        memset(accumulateurs[i].nbEchantillons, 0, sizeof(accumulateurs[i].nbEchantillons));
        accumulateurs[i].actif = false;
    }
}

void RecuperationDonneesAncres::effacerDonneesTag(int tagId) {
    if (tagId >= 0 && tagId < MAX_TAGS) { // Suppression du ID_DEPART_TAGS
        memset(accumulateurs[tagId].sommeDistances, 0, sizeof(accumulateurs[tagId].sommeDistances));
        memset(accumulateurs[tagId].nbEchantillons, 0, sizeof(accumulateurs[tagId].nbEchantillons));
        accumulateurs[tagId].actif = false;
    }
}

void RecuperationDonneesAncres::injecterDonnee(int tagId, float d0, float d1, float d2, float d3) {
    if (tagId < 0 || tagId >= MAX_TAGS) return; // Sécurité directe

    float dists[4] = {d0, d1, d2, d3};
    for (int i = 0; i<4; i++) {
        if (dists[i] > 0.0f) {
            accumulateurs[tagId].sommeDistances[i] += dists[i];
            accumulateurs[tagId].nbEchantillons[i]++;
            accumulateurs[tagId].actif = true;
        }
    }
}

bool RecuperationDonneesAncres::getDonneesLisseesPourTag(int tagId, DistanceMoyennes& resultat_out) {
    if (tagId < 0 || tagId >= MAX_TAGS || !accumulateurs[tagId].actif) {
        return false;
    }

    resultat_out.tag_id = tagId;
    int ancresValides = 0;

    for (int i = 0; i<4; i++) {
        if (accumulateurs[tagId].nbEchantillons[i] > 0) {
            resultat_out.distances[i] = accumulateurs[tagId].sommeDistances[i] / (float)accumulateurs[tagId].nbEchantillons[i];
            ancresValides++;
        } else {
            resultat_out.distances[i] = 0.0f;
        }
    }

    memset(accumulateurs[tagId].sommeDistances, 0, sizeof(accumulateurs[tagId].sommeDistances));
    memset(accumulateurs[tagId].nbEchantillons, 0, sizeof(accumulateurs[tagId].nbEchantillons));
    accumulateurs[tagId].actif = false;

    return (ancresValides > 0);
}

bool RecuperationDonneesAncres::getToutesDonneesLissees(std::vector<DistanceMoyennes>& resultats_out) {
    resultats_out.clear();
    for (int idx = 0; idx < MAX_TAGS; idx++) {
        if (!accumulateurs[idx].actif) continue;
        DistanceMoyennes resultatTag;
        if (getDonneesLisseesPourTag(idx, resultatTag)) {
            int ancresValides = 0;
            for(int i=0; i<4; i++) if(resultatTag.distances[i] > 0.0f) ancresValides++;
            if(ancresValides >= 3) {
                resultats_out.push_back(resultatTag);
            }
        }
    }
    return !resultats_out.empty();
}

