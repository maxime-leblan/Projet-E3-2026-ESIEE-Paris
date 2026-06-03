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