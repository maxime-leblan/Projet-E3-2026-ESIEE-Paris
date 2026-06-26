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

    // Les boucles parcourent les index de coordonnées (0 à 3)
    for (int i = 0; i <= 2; i++)
    {
        for (int j = i + 1; j <= 3; j++)
        {
            // Mais la clé générée applique un décalage de +1 ("12", "13", etc.)
            vKey = to_string(i + 1) + to_string(j + 1);
            vDistances[vKey] = (pCoordinates[j] - pCoordinates[i]).norm();
        }
    }

    return vDistances;
}

void printTab(vector<V3> pSensors)
{
    for (int i = 0; i < pSensors.size(); i++)
    {
        V3 vCoordAnchor = pSensors[i];
        cout << "Ancre " << (i+1) << " : " << vCoordAnchor << "\n";
    }
}

void printTabUWB(UWBModuleList pSensors)
{
    for (int i = 0; i < pSensors.size(); i++)
    {
        V3 vCoordAnchor = pSensors.getModule(i).getPosition();
        cout << "Ancre " << (i+1) << " : " << vCoordAnchor << "\n";
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
    for (int i = 0; i < 4; i++)
    {
        UWBModule vTemp = UWBModule(i, vSensorsPosition[i]);
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

    //unordered_map<string, float> vRealDistances = makeDistanceTabFromCoordinates(vRealCoordinates);

    // on stocke les distances entre chaque ancre
    unordered_map<string, float> vMesuredDistances;

    vMesuredDistances["12"] = 4.28; //dist01; // 3.58 // 3.58
    vMesuredDistances["13"] = 4.42; //dist02; // 4.42 // 4.68
    vMesuredDistances["14"] = 7.80; //dist03; // 3.24 // 3.15
    vMesuredDistances["23"] = 4.48; //dist12; // 3.48 // 3.05
    vMesuredDistances["24"] = 7.70; //dist13; // 4.74 // 5.15
    vMesuredDistances["34"] = 3.87; //dist23; // 3.44 // 4.12

    // on lance l'initialisation des ancres
    initAnchorsCoordinates(vSensors, vMesuredDistances);

    // on affiche les coordonnées données lors de l'initialisation à chaque ancre
    printTabUWB(vSensors);

    // on affiche les pourcentages d'erreurs pour chaque distance
    cout << "Erreurs algo classique (en \% par rapport à la vrai distance): \n";
    //printDict(giveErrors(vRealDistances, vMesuredDistances));

    // ------------------------------------------------------------------------------
    // Partie avec utilisation de la descente de gradient

    // Remplacement des index (1-4) par (0-3)
    vMesuredDistances["12"] = 4.28; //dist01; // 3.58 // 3.58
    vMesuredDistances["13"] = 4.42; //dist02; // 4.42 // 4.68
    vMesuredDistances["14"] = 7.80; //dist03; // 3.24 // 3.15
    vMesuredDistances["23"] = 4.48; //dist12; // 3.48 // 3.05
    vMesuredDistances["24"] = 7.70; //dist13; // 4.74 // 5.15
    vMesuredDistances["34"] = 3.87; //dist23; // 3.44 // 4.12

    // on lance l'initialisation des ancres
    initAnchorsCoordinatesWithGD(vSensors, vMesuredDistances, 1000, 0.01);

    cout << "\nPartie avec descente de gradient : " << "\n";

    // on affiche les coordonnées données lors de l'initialisation à chaque ancre
    cout << vSensors.toString();
    cout << "\nErreurs algo descente de gradient (en \% par rapport à la vrai distance): \n";
    //printDict(giveErrors(vRealDistances, makeDistanceTabFromCoordinates(giveCoordinates(vSensors))));

    // --------------------------------------------------------------------------------
    // Application d'une matrice de rotation sur une liste de points
    cout << "\nPartie rotation de points :\n";

    // vecteurs représentant l'angle de la rotation
    V3 vStartVector = V3(-5, -1, 3);
    V3 vResultVector = V3(-5, 5, -2);

    // liste des points que l'on veut déplacer par rotation axiale
    vector<V3> vPointsList = {V3(-2, -4, 3), V3(-3, 3, 3), V3(-1, 2, 3)};

    // on applique la rotation sur chacun de ces points
    vPointsList = applyRotationOnPoints(vPointsList, giveRotationalMatrix(vStartVector, vResultVector));
    printTab(vPointsList);
}