#include "GridLibrary.hpp"

void initAnchorsCoordinates(UWBModuleList & pAnchors, unordered_map<string, float> pDistances)
{
    // on récupère les véritables identifiants des ancres, auquels on accédera par les indices de la liste de 0 à 3
    vector<int> vAnchorIdList = pAnchors.giveModuleIdList();
    // on trie la liste pour ensuite pouvoir obtenir les bonnes distances dans pDistances car les clés sont de la forme : "12"
    sort(vAnchorIdList.begin(), vAnchorIdList.end());

    // on fixe les coordonnées de la première ancre à (0, 0, 0)
    pAnchors.setModulePosition(vAnchorIdList[0], V3(0, 0, 0));

    // on fixe les coodonnées de la deuxième ancre à (d1_2, 0, 0) avec d1_2 la distance entre l'ancre 1 et l'ancre 2
    string vDistanceBtwA1andA2Id = to_string(vAnchorIdList[0]) + to_string(vAnchorIdList[1]);
    float vD1to2 = pDistances[vDistanceBtwA1andA2Id];
    pAnchors.setModulePosition(vAnchorIdList[1], V3(vD1to2, 0, 0));

    // on fixe les coordonnées de la troisième ancre à (x3, y3, 0) en déterminant x3 et y3 à l'aide du théorème de Pythagore
    string vDistanceBtwA1andA3Id = to_string(vAnchorIdList[0]) + to_string(vAnchorIdList[2]);
    float vD1to3 = pDistances[vDistanceBtwA1andA3Id];
    string vDistanceBtwA2andA3Id = to_string(vAnchorIdList[1]) + to_string(vAnchorIdList[2]);
    float vD2to3 = pDistances[vDistanceBtwA2andA3Id];

    float vX3 = (vD1to2 * vD1to2 + vD1to3 * vD1to3 - vD2to3 * vD2to3) / (2 * vD1to2);
    float vY3 = sqrt(vD1to3 * vD1to3 - vX3 * vX3);

    pAnchors.setModulePosition(vAnchorIdList[2], V3(vX3, vY3, 0));

    // on fixe les coordonnées de la quatrième ancre à (x4, y4, z4) avec un système d'équation prenant en compte toutes les distances
    string vDistanceBtwA1andA4Id = to_string(vAnchorIdList[0]) + to_string(vAnchorIdList[3]);
    float vD1to4 = pDistances[vDistanceBtwA1andA4Id];
    string vDistanceBtwA2andA4Id = to_string(vAnchorIdList[1]) + to_string(vAnchorIdList[3]); 
    float vD2to4 = pDistances[vDistanceBtwA2andA4Id];
    string vDistanceBtwA3andA4Id = to_string(vAnchorIdList[2]) + to_string(vAnchorIdList[3]); 
    float vD3to4 = pDistances[vDistanceBtwA3andA4Id];

    float vX4 = (vD1to2 * vD1to2 + vD1to4 * vD1to4 - vD2to4 * vD2to4) / (2 * vD1to2);
    float vY4 = (vD1to4 * vD1to4 - vD3to4 * vD3to4 + vX3 * vX3 + vY3 * vY3 - 2 * vX3 * vX4) / (2 * vY3);
    float vZ4 = sqrt(vD1to4 * vD1to4 - vX4 * vX4 - vY4 * vY4);

    pAnchors.setModulePosition(vAnchorIdList[3], V3(vX4, vY4, vZ4));
}

void initAnchorsCoordinatesWithGD(UWBModuleList & pAnchors, unordered_map<string, float> pDistances, int pIter, float pAlpha)
{
    // on utilise l'algorithme standard pour que les ancres ait déjà une position proche de la réalité
    initAnchorsCoordinates(pAnchors, pDistances);

    // on récupère les véritables identifiants des ancres, auquels on accédera par les indices de la liste de 0 à 3
    vector<int> vAnchorIdList = pAnchors.giveModuleIdList();
    // on trie la liste pour ensuite pouvoir obtenir les bonnes distances dans pDistances car les clés sont de la forme : "12"
    sort(vAnchorIdList.begin(), vAnchorIdList.end());

    for (int vIter = 0; vIter < pIter; vIter++)
    {
        // enregistrement des positions actuelles des ancres dans une liste
        vector<V3> vPos(4);
        for (int i = 0; i < 4; i++) {
            vPos[i] = pAnchors.getModule(vAnchorIdList[i]).getPosition();
        }

        // Tableau pour accumuler les gradients de chaque ancre
        vector<V3> vGradients = { V3(0,0,0), V3(0,0,0), V3(0,0,0), V3(0,0,0) };

        // Calcul des gradients pour chaque noeud
        // La fonction de coût est la somme des (d_ij_mesurée - d_ij_calculée)^2
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (i == j) continue;

                // Récupération de la clé de distance ("12", "23", etc.) avec l'ID plus petit en premier
                string vKey = (i < j) ? (to_string(vAnchorIdList[i]) + to_string(vAnchorIdList[j])) 
                                      : (to_string(vAnchorIdList[j]) + to_string(vAnchorIdList[i]));
                
                float vDistMesuree = pDistances[vKey];
                
                V3 vVectDiff = vPos[i] - vPos[j];
                float vDistCalculee = vVectDiff.norm();

                // Éviter la division par zéro si deux points se superposent par erreur
                if (vDistCalculee > 1e-4f) 
                {
                    V3 vVectUnitaire = vVectDiff.getNormalized();
                    
                    // Dérivée partielle du Stress par rapport à la position P_i :
                    // Grad = -2 * (d_mesurée - d_calculée) * (Vecteur_Unitaire_j_vers_i)
                    vGradients[i] = vGradients[i] - 2.0f * (vDistMesuree - vDistCalculee) * vVectUnitaire;
                }
            }
        }

        // Mise à jour des positions (P = P - alpha * Gradient)
        // On commence la boucle à i = 1 pour laisser l'Ancre 1 fixe à (0,0,0) comme origine.
        for (int i = 1; i < 4; i++)
        {
            V3 vNewPos = vPos[i] - (vGradients[i] * pAlpha);
            pAnchors.setModulePosition(vAnchorIdList[i], vNewPos);
        }
    }
}