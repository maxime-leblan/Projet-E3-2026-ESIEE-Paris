#pragma once

#include <vector>
#include <unordered_map>

#include "UWBModule.h"

class UWBModuleList
{
    private:

    /*
    Dictionnaire contenant la liste des modules UWB
    */
    unordered_map<int, UWBModule> aUWBModuleList;

    public:

    /*
    Constructeur naturel de la classe (ne fait rien)
    */
    UWBModuleList();

    /*
    Renvoie le nombre d'éléments stockés dans la liste
    */
    int size();

    /*
    Renvoie le module associé à l'identifiant passé en paramètre
    pId - identifiant du module que l'on veut récupérer
    */
    UWBModule getModule(int pId);

    /*
    Renvoie la liste des modules qui lorsqu'ils sont passé en paramètre de la fonction pFunction, celle-ci renvoie vrai
    */
    vector<int> findAll(bool (&pFunction)(UWBModule));

    /*
    Ajoute un module à la liste des modules
    pId - identifiant du module que l'on veut 
    */
    void addModule(int pId, UWBModule pModule);


};