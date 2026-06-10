#include <iostream>

#include "3DVisualisation.hpp"
#include "../librairie_hub/GridLibrary.hpp"

/*
Commande de compilation : 
g++ -std=c++17 \
  -I . -I /usr/include/eigen3 -I /usr/local/include -I "../../Algorithme-trilateration" \
  -L/usr/local/lib \
  main_test.cpp \
  3DVisualisation.cpp \
  "../librairie_hub/GridLibrary.cpp" \
  "../../Algorithme-trilateration/UWBModuleList.cpp" \
  "../../Algorithme-trilateration/UWBModule.cpp" \
  "../../Algorithme-trilateration/V3.cpp" \
  -o main_test \
  -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
*/

int main()
{
    // Application d'une matrice de rotation sur une liste de points
    cout << "\nPartie rotation de points :\n";

    // vecteurs représentant l'angle de la rotation
    V3 vStartVector = V3(-5, -1, 3);
    V3 vResultVector = V3(-5, 5, -2);

    // liste des points que l'on veut déplacer par rotation axiale
    vector<V3> vPointsList = {V3(-2, -4, 3), V3(-3, 3, 3), V3(-1, 2, 3)};

    // on applique la rotation sur chacun de ces points
    vector<V3> vNewPointsList = applyRotationOnPoints(vPointsList, giveRotationalMatrix(vStartVector, vResultVector));

    // on crée les listes en concaténant tous les points et les vecteurs
    vector<vector<V3>> vVectorsListA = buildVectorsListFor3DEnvironment(vPointsList);
    vector<vector<V3>> vVectorsListB = buildVectorsListFor3DEnvironment(vNewPointsList);

    vector<vector<V3>> vFinalVectorsList = concatenateVectorsList(vVectorsListA, vVectorsListB);

    vector<V3> vFinalPointsList = concatenatePointsList(vPointsList, vNewPointsList);
    
    // on affiche nos résultat dans l'environnement 3D
    // view3DEnvironment(vFinalPointsList, vFinalVectorsList);
    view3DEnvironment(convertPointsListAxisToRayLib(vFinalPointsList), convertVectorsListAxisToRayLib(vFinalVectorsList));

    for (int i = 0; i < vFinalPointsList.size(); i++)
    {
      cout << "Point " << i << " : " << vFinalPointsList[i] << "\n";
    }
}