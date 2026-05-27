#pragma once

#include <unordered_map>
#include <vector>

#include "UWBModule.h"

class UWBModuleList
{
    private:

    /*
    Dictionnaire contenant la liste des modules UWB
    */
    unordered_map<string, UWBModule> aUWBModuleList;

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
    Renvoie le module associé au nom passé en paramètre
    */
    UWBModule getModule(string pModuleName);

    /*
    Renvoie la liste des modules qui lorsqu'ils sont passé en paramètre de la fonction pFunction, celle-ci renvoie vrai
    */
    vector<string> findAll(bool (&pFunction)(UWBModule));

    /*
    Ajoute un module à la liste des modules
    */
    void addModule(string pModuleName, UWBModule pModule);


};