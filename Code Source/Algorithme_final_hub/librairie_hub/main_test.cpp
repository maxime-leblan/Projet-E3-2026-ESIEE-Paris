#include "GridLibrary.hpp"
#include <iostream>

/*
Commande de compilation : 
g++ -std=c++17 \
  -I . \
  -I "../../Algorithme trilatération" \
  main_test.cpp \
  GridLibrary.cpp \
  "../../Algorithme trilatération/UWBModuleList.cpp" \
  "../../Algorithme trilatération/UWBModule.cpp" \
  "../../Algorithme trilatération/V3.cpp" \
  -o main_test
*/

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
    A2 = (2, 0, 0)
    A3 = (1, -2, 0)
    A4 = (-2, 3, 3)
    La fonction initAnchorsCoordinates devrait attribuer ces coordonnées à chaque ancre après son exécution,
    mais SANS TENIR COMPTE DES SIGNES. En effet, la figure peut être orientée dans différentes directions,
    l'algorithme ne permet pas de l'orienter dans une direction précise
    */
    // on stocke les distances entre chaque ancre
    unordered_map<string, float> vDistances;
    vDistances["12"] = 2; // au lieu de 2 (Erreur +15cm)
    vDistances["13"] = 2.24; // au lieu de 2.24 (Erreur -10cm)
    vDistances["14"] = 4.69; // au lieu de 4.69 (Erreur +20cm)
    vDistances["23"] = 2.24; // au lieu de 2.24 (Erreur +10cm)
    vDistances["24"] = 5.83; // au lieu de 5.83 (Erreur -10cm)
    vDistances["34"] = 6.56; // au lieu de 6.56 (Erreur -15cm)

    // on lance l'initialisation des ancres
    initAnchorsCoordinates(vSensors, vDistances);

    // on affiche les coordonnées données lors de l'initialisation à chaque ancre
    for (int i = 1; i <= 4; i++)
    {
        V3 vCoordAnchor = vSensors.getModule(i).getPosition();
        cout << "Ancre " << i << " : " << vCoordAnchor << "\n";
    }

    // ------------------------------------------------------------------------------
    // Partie avec utilisation de la descente de gradient

    vDistances["12"] = 2.15; // au lieu de 2.0 (Erreur +15cm)
    vDistances["13"] = 2.10; // au lieu de 2.2 (Erreur -10cm)
    vDistances["14"] = 4.80; // au lieu de 4.6 (Erreur +20cm)
    vDistances["23"] = 2.30; // au lieu de 2.2 (Erreur +10cm)
    vDistances["24"] = 5.70; // au lieu de 5.8 (Erreur -10cm)
    vDistances["34"] = 6.45; // au lieu de 6.6 (Erreur -15cm)

    // on lance l'initialisation des ancres
    initAnchorsCoordinatesWithGD(vSensors, vDistances, 100, 0.01);

    cout << "\nPartie avec descente de gradient : " << "\n";

    // on affiche les coordonnées données lors de l'initialisation à chaque ancre
    for (int i = 1; i <= 4; i++)
    {
        V3 vCoordAnchor = vSensors.getModule(i).getPosition();
        cout << "Ancre " << i << " : " << vCoordAnchor << "\n";
    }
}