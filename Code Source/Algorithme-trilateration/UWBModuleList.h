#pragma once

#include <vector>
#include <unordered_map>
#include <map>

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
    Renvoie un dictionnaire contenant la liste des positions des modules de la liste avec chaque position d'un module
    associé à son identifiant
    */
    std::map<int, V3> giveModulePositionList() const;

    /**
     * Renvoie une chaîne de caractères permettant de représenter la liste de module textuellement
     * @return Une chaîne de type [(<id_0>, <position_0>), (<id_1>, <position_1>),...,(<id_n>, <position_n>)] ou [] si elle est vide
     */
    string toString();

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
    Renvoie la liste des identifiants des modules stockés dans la liste
    */
    vector<int> giveModuleIdList();

    /*
    Ajoute un module à la liste des modules
    pId - identifiant du module que l'on veut 
    */
    void addModule(int pId, UWBModule pModule);

    /*
    Modifie la position du module dont l'identifiant est passé en paramètre
    pModuleId - identifiant du module dont on veut modifier la position
    pNewPosition - nouvelle position que l'on veut donner au module
    */
    void setModulePosition(int pModuleId, V3 pNewPosition);


};