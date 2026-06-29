#include <iostream>
#include "3DVisualisation.hpp"
#include "../librairie_hub/GridLibrary.hpp"

/*
Commande de compilation : 
g++ -std=c++17 \
  -I . -I /usr/include/eigen3 -I /usr/local/include -I "../../Algorithme-trilateration" \
  -L/usr/local/lib \
  main_test_poster.cpp \
  3DVisualisation.cpp \
  "../librairie_hub/GridLibrary.cpp" \
  "../../Algorithme-trilateration/UWBModuleList.cpp" \
  "../../Algorithme-trilateration/UWBModule.cpp" \
  "../../Algorithme-trilateration/V3.cpp" \
  -o main_test_poster \
  -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
*/

int main()
{
    // Translation des ancres
    cout << "\nPartie translation des ancres :\n";

    // liste des coordonnées des ancres
    vector<V3> vPointsList = {V3(-2, -4, 3), V3(-3, 3, 3), V3(-1, 2, 3), V3(1, 2, 4)};
    UWBModuleList vAnchors = UWBModuleList();

    // on ajoute les ancres dans la liste des capteurs
    for (int i = 1; i <= vPointsList.size(); i++)
    {
        UWBModule vTemp = UWBModule(i, vPointsList[i - 1]);
        vAnchors.addModule(vTemp.getId(), vTemp);
    }

    // on applique la translation sur les ancres
    alignAnchorsCoordinatesWithGridOrigin(vAnchors);
    // on enregistre les coordonnées de chaque ancre après translation
    vector<V3> vNewPointsList;
    map<int, V3> vAnchorsPositionList = vAnchors.giveModulePositionList();

    for (auto it = vAnchorsPositionList.begin(); it != vAnchorsPositionList.end(); it++)
    {
        vNewPointsList.push_back(it->second);
    }

    // on crée la zone de sécurité
    const int numPoints = 64;
    const float radius = 8.0f;
    std::vector<V3> pointsCercle;
    pointsCercle.reserve(numPoints); // Optimisation de la mémoire

    for (int i = 0; i < numPoints; ++i) {
        // Calcul de l'angle pour chaque point (répartition uniforme de 0 à 2*PI)
        float angle = 2.0f * M_PI * i / numPoints;

        // Calcul des coordonnées X et Y
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);
        float z = 0.0f; // Cercle à plat sur le plan X, Y

        // Ajout du point au vecteur en utilisant le constructeur de V3
        pointsCercle.push_back(V3(x, y, z));
    }

    // on crée les listes en concaténant tous les points et les vecteurs
    vector<vector<V3>> vVectorsListA = buildVectorsListFor3DEnvironment(vNewPointsList);
    vector<vector<V3>> vVectorsListB = buildVectorsListFor3DEnvironment(pointsCercle);

    vector<vector<V3>> vFinalVectorsList = concatenateVectorsList(vVectorsListA, vVectorsListB);

    vector<V3> vFinalPointsList = concatenatePointsList(pointsCercle, vNewPointsList);
    
    // on affiche nos résultat dans l'environnement 3D
    view3DEnvironment(convertPointsListAxisToRayLib(vFinalPointsList), convertVectorsListAxisToRayLib(vFinalVectorsList));
}