#pragma once

#include <string>
#include <vector>
#include <regex>
#include <iostream>

#define BUFFER 100
#define DISTANCE_REGEX "AT\\+RANGE=tid:3,mask:01,seq:228,range:\\(269,[0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]+\\),ancid:\\(0,-1,-1,-1,-1,-1,-1,-1\\)"
#define TAG_ID_REGEX "AT\\+RANGE=tid:[0-9]+,mask:01,seq:228,range:\\(269,0,0,0,0,0,0,0\\),ancid:\\(0,-1,-1,-1,-1,-1,-1,-1\\)"

/*
Renvoie un tableau contenant les entiers à extraire d'une chaîne de caractères suivant le pattern regex
*/
std::vector<int> getDataFromString(const std::string& texte, const std::string& patternRegex);



/*
Renvoie la distance qui sépare le tag, d'où proviennent les données passées en parmètre, de l'ancre dont l'id est passée en paramètre
pTagData - données provenant du tag dont on veut connaître la distance par rapport à l'ancre
==> les données sont de la forme : ...range:(<distance_à_ancre_0>, <distance_à_ancre_1>, ... , <distance_à_ancre_7>) ...
pAnchorId - indice (compris entre 0 et 7 inclus) correspondant à la place de la distance par rapport au tag dans les données du tag
Return: une distance (pas d'unité) ou -1 si l'on est pas parvenu à trouver une distance

*/
// int getDistanceFromAnchor(string pTagData, int pAnchorId);

/*
Renvoie l'id du tag à partir des données envoyées par ce même tag
pTagData - données provenant du tag
*/
// int getTagIdFromTagData(string pTagData);