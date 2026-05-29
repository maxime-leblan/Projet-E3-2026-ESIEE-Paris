#pragma once

#include <string>
#include <vector>
#include <regex>
#include <iostream>

#define BUFFER 100
#define TAG_DATA_REGEX "AT\\+RANGE=tid:[0-9]+,mask:[0-9]+,seq:[0-9]+,range:\\([0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]+\\),ancid:\\([0-9]+,([+-]?(?=\\.\\d|\\d)(?:\\d+)?(?:\\.?\\d*))(?:[Ee]([+-]?\\d+))?,([+-]?(?=\\.\\d|\\d)(?:\\d+)?(?:\\.?\\d*))(?:[Ee]([+-]?\\d+))?,([+-]?(?=\\.\\d|\\d)(?:\\d+)?(?:\\.?\\d*))(?:[Ee]([+-]?\\d+))?,([+-]?(?=\\.\\d|\\d)(?:\\d+)?(?:\\.?\\d*))(?:[Ee]([+-]?\\d+))?,([+-]?(?=\\.\\d|\\d)(?:\\d+)?(?:\\.?\\d*))(?:[Ee]([+-]?\\d+))?,([+-]?(?=\\.\\d|\\d)(?:\\d+)?(?:\\.?\\d*))(?:[Ee]([+-]?\\d+))?,([+-]?(?=\\.\\d|\\d)(?:\\d+)?(?:\\.?\\d*))(?:[Ee]([+-]?\\d+))?\\)"
#define TAG_ID_INDEX 0
#define FIRST_TAG_DISTANCE_INDEX 3

using namespace std;

/*
Renvoie un tableau contenant les entiers à extraire d'une chaîne de caractères suivant le pattern regex
*/
std::vector<int> getDataFromString(const std::string& texte, const std::string& patternRegex);

/*
Renvoie la distance qui sépare le tag, d'où proviennent les données passées en parmètre, de l'ancre dont l'id est passée en paramètre
pTagData - données provenant du tag dont on veut connaître la distance par rapport à l'ancre, présentées sous forme de tableau d'entier
==> les données sont de la forme : ...range:(<distance_à_ancre_0>, <distance_à_ancre_1>, ... , <distance_à_ancre_7>) ...
pAnchorId - indice (compris entre 0 et 7 inclus) correspondant à la place de la distance par rapport au tag dans les données du tag
Return: une distance (pas d'unité) ou -1 si l'on est pas parvenu à trouver une distance
*/
int getDistanceFromAnchor(vector<int> pTagData, int pAnchorId);

/*
Renvoie l'id du tag à partir des données envoyées par ce même tag
pTagData - données provenant du tag, sous forme de tableau d'entiers
*/
int getTagIdFromTagData(vector<int> pTagData);