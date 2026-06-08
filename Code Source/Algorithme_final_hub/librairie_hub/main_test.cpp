#include "GridLibrary.hpp"
#include <iostream>

/*
Commande de compilation : 
g++ -std=c++17 -I . -I /usr/include/eigen3 -I "../../Algorithme-trilateration" main_test.cpp GridLibrary.cpp "../../Algorithme-trilateration/UWBModuleList.cpp" "../../Algorithme-trilateration/UWBModule.cpp" "../../Algorithme-trilateration/V3.cpp" -o main_test
*/

unordered_map<string, float> giveErrors(unordered_map<string, float> pRealDistances, unordered_map<string, float> pComputedDistances)
{
    unordered_map<string, float> vResults;

    for (auto it = pRealDistances.begin(); it != pRealDistances.end(); it++)
    {
        vResults[it->first] = (abs(pRealDistances[it->first] - pComputedDistances[it->first])/(pRealDistances[it->first])) * 100.0f;
    }

    return vResults;
}

vector<V3> giveCoordinates(UWBModuleList pSensors)
{
    vector<V3> vCoord;
    vector<int> vModuleIdList = pSensors.giveModuleIdList();
    sort(vModuleIdList.begin(), vModuleIdList.end());
    int vCurrentId;

    for (int i = 0; i < vModuleIdList.size(); i++)
    {
        vCurrentId = vModuleIdList[i];
        vCoord.push_back(pSensors.getModule(vCurrentId).getPosition());
    }

    return vCoord;
}

unordered_map<string, float> makeDistanceTabFromCoordinates(vector<V3> pCoordinates)
{
    unordered_map<string, float> vDistances;
    string vKey;

    for (int i = 1; i <= 3; i++)
    {
        for (int j = i+1; j <= 4; j++)
        {
            vKey = to_string(i) + to_string(j);
            vDistances[vKey] = (pCoordinates[j-1] - pCoordinates[i-1]).norm();
        }
    }

    return vDistances;
}

void printTab(vector<V3> pSensors)
{
    for (int i = 1; i <= 4; i++)
    {
        V3 vCoordAnchor = pSensors[i];
        cout << "Ancre " << i << " : " << vCoordAnchor << "\n";
    }
}

void printTabUWB(UWBModuleList pSensors)
{
    for (int i = 1; i <= 4; i++)
    {
        V3 vCoordAnchor = pSensors.getModule(i).getPosition();
        cout << "Ancre " << i << " : " << vCoordAnchor << "\n";
    }
}

void printDict(unordered_map<string, float> pDict)
{
    for (auto it = pDict.begin(); it != pDict.end(); it++)
    {
        cout << "Distance [" << it->first << "] : " << it->second << "\n";
    }
}

int main()
{
    // on instancie les capteurs
    UWBModuleList vSensors = UWBModuleList();
    vector<V3> vSensorsPosition = {V3(-1, -1, -1), V3(-1, -1, -1), V3(-1, -1, -1), V3(-1, -1, -1)};

    // on les ajoute dans la liste des capteurs
    for (int i = 1; i <= 4; i++)
    {
        UWBModule vTemp = UWBModule(i, vSensorsPosition[i - 1]);
        vSensors.addModule(vTemp.getId(), vTemp);
    }

    /*
    Prenons le cas où les ancres ont les coordonnées suivantes : 
    A1 = (0, 0, 0)
    A2 = (4, 0, 0)
    A3 = (1.83, 3.99, 0)
    A4 = (2.33, 6.53, 4)
    La fonction initAnchorsCoordinates devrait attribuer ces coordonnées à chaque ancre après son exécution,
    mais SANS TENIR COMPTE DES SIGNES. En effet, la figure peut être orientée dans différentes directions,
    l'algorithme ne permet pas de l'orienter dans une direction précise
    */
    vector<V3> vRealCoordinates = {V3(0, 0, 0), V3(4, 0, 0), V3(1.83, 3.99, 0), V3(2.33, 6.53, 4)};

    unordered_map<string, float> vRealDistances = makeDistanceTabFromCoordinates(vRealCoordinates);

    // on stocke les distances entre chaque ancre
    unordered_map<string, float> vMesuredDistances;

    vMesuredDistances["12"] = 3.85; // au lieu de 4 (Erreur +15cm)
    vMesuredDistances["13"] = 4.55; // au lieu de 4.39 (Erreur -10cm)
    vMesuredDistances["14"] = 7.80; // au lieu de 8.01 (Erreur +20cm)
    vMesuredDistances["23"] = 4.25; // au lieu de 4.54 (Erreur +10cm)
    vMesuredDistances["24"] = 8.05; // au lieu de 7.84 (Erreur -10cm)
    vMesuredDistances["34"] = 4.5; // au lieu de 4.77 (Erreur -15cm)

    // on lance l'initialisation des ancres
    initAnchorsCoordinates(vSensors, vMesuredDistances);

    // on affiche les coordonnées données lors de l'initialisation à chaque ancre
    printTabUWB(vSensors);

    // on affiche les pourcentages d'erreurs pour chaque distance
    cout << "Erreurs algo classique (en \% par rapport à la vrai distance): \n";
    printDict(giveErrors(vRealDistances, vMesuredDistances));

    // ------------------------------------------------------------------------------
    // Partie avec utilisation de la descente de gradient

    vMesuredDistances["12"] = 3.85; // au lieu de 4 (Erreur +15cm)
    vMesuredDistances["13"] = 4.55; // au lieu de 4.39 (Erreur -10cm)
    vMesuredDistances["14"] = 7.80; // au lieu de 8.01 (Erreur +20cm)
    vMesuredDistances["23"] = 4.25; // au lieu de 4.54 (Erreur +10cm)
    vMesuredDistances["24"] = 8.05; // au lieu de 7.84 (Erreur -10cm)
    vMesuredDistances["34"] = 4.5; // au lieu de 4.77 (Erreur -15cm)

    // on lance l'initialisation des ancres
    initAnchorsCoordinatesWithGD(vSensors, vMesuredDistances, 1000, 0.01);

    cout << "\nPartie avec descente de gradient : " << "\n";

    // on affiche les coordonnées données lors de l'initialisation à chaque ancre
    printTabUWB(vSensors);
    cout << "Erreurs algo descente de gradient (en \% par rapport à la vrai distance): \n";
    printDict(giveErrors(vRealDistances, makeDistanceTabFromCoordinates(giveCoordinates(vSensors))));
}