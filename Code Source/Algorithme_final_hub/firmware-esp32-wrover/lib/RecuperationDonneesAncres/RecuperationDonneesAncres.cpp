#include "RecuperationDonneesAncres.hpp"

RecuperationDonneesAncres::RecuperationDonneesAncres() {
    chronoFenetre = millis();

    //Initialisation des accumulateurs à zéro :

    for (int i = 0; i < MAX_TAGS; i++) {
        memset(accumulateurs[i].sommeDistances, 0, sizeof(accumulateurs[i].sommeDistances));
        memset(accumulateurs[i].nbEchantillons, 0, sizeof(accumulateurs[i].nbEchantillons));
        accumulateurs[i].actif = false;
    }
}

void RecuperationDonneesAncres::injecterDonnee(int tagId, float d0, float d1, float d2, float d3) {
    int idx = tagId - ID_DEPART_TAGS; // Les tags commencent à l'id ID_DEPART_TAGS
    if (idx < 0 || idx >= MAX_TAGS) return; // sécurité

    float dists[4] = {d0, d1, d2, d3};

    //On ajoute toutes les distances valides (>0, même les doublons)
    for (int i = 0; i<4; i++) {
        if (dists[i] > 0.0f) {
            accumulateurs[idx].sommeDistances[i] += dists[i];
            accumulateurs[idx].nbEchantillons[i]++;
            accumulateurs[idx].actif = true;
        }
    }
}

bool RecuperationDonneesAncres::getDonneesLissees(std::vector<DistanceMoyennes>& resultats_out) {
    //Si la fenêtre de temps n'est pas encore passé, on ne fait rien.
    if (millis() - chronoFenetre < FENETRE_MS) {
        return false;
    }

    //Le tmeps est écoulé :
    chronoFenetre = millis();
    resultats_out.clear();

    for (int idx = 0; idx < MAX_TAGS; idx++) {
        if (!accumulateurs[idx].actif) continue; // Pas de données dans ce tag

        DistanceMoyennes resultatTag;
        resultatTag.tag_id = idx + ID_DEPART_TAGS;
        int ancresValides = 0;

        for (int i = 0; i<4; i++) {
            if (accumulateurs[idx].nbEchantillons[i] > 0) {
                //calcul de la moyenne :
                resultatTag.distances[i] = accumulateurs[idx].sommeDistances[i] / (float)accumulateurs[idx].nbEchantillons[i];
                ancresValides++;
            } else {
                resultatTag.distances[i] = 0.0f;
            }
        }

        if (ancresValides >= 3) {
            resultats_out.push_back(resultatTag);
        }

        //Remise à zero :
        memset(accumulateurs[idx].sommeDistances, 0, sizeof(accumulateurs[idx].sommeDistances));
        memset(accumulateurs[idx].nbEchantillons, 0, sizeof(accumulateurs[idx].nbEchantillons));
        accumulateurs[idx].actif = false;
    }

    //renvoie true s'il y a eu des calculs, false si personne n'a bougé.
    return !resultats_out.empty();
}