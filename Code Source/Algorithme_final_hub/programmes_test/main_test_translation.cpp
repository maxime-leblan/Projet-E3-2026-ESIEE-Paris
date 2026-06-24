#include <iostream>

#include "3DVisualisation.hpp"
#include "../librairie_hub/GridLibrary.hpp"

/*
Commande de compilation : 
g++ -std=c++17 \
  -I . -I /usr/include/eigen3 -I /usr/local/include -I "../../Algorithme-trilateration" \
  -L/usr/local/lib \
  main_test_translation.cpp \
  3DVisualisation.cpp \
  "../librairie_hub/GridLibrary.cpp" \
  "../../Algorithme-trilateration/UWBModuleList.cpp" \
  "../../Algorithme-trilateration/UWBModule.cpp" \
  "../../Algorithme-trilateration/V3.cpp" \
  -o main_test_translation \
  -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
*/

int main()
{
    // Translation des ancres
    cout << "\nPartie translation des ancres :\n";

    // liste des coordonnées des ancres
    vector<V3> vPointsList = {V3(0.000000, 0.000000, 0.000000), V3(0.167643, 0.000000, 0.000000), V3(0.229456, 0.104456, 0.000000), V3(0.315397, 0.185018, 0.063392)};
    UWBModuleList vAnchors = UWBModuleList();

    // on ajoute les ancres dans la liste des capteurs
    for (int i = 1; i <= vPointsList.size(); i++)
    {
        UWBModule vTemp = UWBModule(i, vPointsList[i - 1]);
        vAnchors.addModule(vTemp.getId(), vTemp);
    }

    V3 vFirstEpicenterPosition = giveEpicenterPosition(vAnchors);

    // on applique la translation sur les ancres
    alignAnchorsCoordinatesWithGridOrigin(vAnchors);
    // on enregistre les coordonnées de chaque ancre après translation
    vector<V3> vNewPointsList;
    map<int, V3> vAnchorsPositionList = vAnchors.giveModulePositionList();

    for (auto it = vAnchorsPositionList.begin(); it != vAnchorsPositionList.end(); it++)
    {
        vNewPointsList.push_back(it->second);
    }

    // on crée les listes en concaténant tous les points et les vecteurs
    vector<vector<V3>> vVectorsListA = buildVectorsListFor3DEnvironment(vPointsList);
    vector<vector<V3>> vVectorsListB = buildVectorsListFor3DEnvironment(vNewPointsList);

    vNewPointsList.push_back(giveEpicenterPosition(vAnchors));
    vPointsList.push_back(vFirstEpicenterPosition);

    vector<vector<V3>> vFinalVectorsList = concatenateVectorsList(vVectorsListA, vVectorsListB);

    vector<V3> vFinalPointsList = concatenatePointsList(vPointsList, vNewPointsList);
    
    // on affiche nos résultat dans l'environnement 3D
    view3DEnvironment(convertPointsListAxisToRayLib(vFinalPointsList), convertVectorsListAxisToRayLib(vFinalVectorsList));

    for (int i = 0; i < vFinalPointsList.size(); i++)
    {
      cout << "Point " << i << " : " << vFinalPointsList[i] << "\n";
    }
    
    // --------------------------------------------------------------------
    // Changement de base
    cout << "\nPartie changement de base :\n";
    cout << "\nPoints dans la base de départ :\n";

    vNewPointsList.pop_back();
    vPointsList = vNewPointsList;
    vNewPointsList.clear();

    vector<V3> vSafeZonePoints = {V3(-1, 0, 0), V3(-2, 0, 1), V3(-3, -2, 2), V3(-4, -3, 3), V3(-5, -4, 4)};

    vVectorsListA = buildVectorsListFor3DEnvironment(vPointsList);
    vVectorsListB = buildVectorsListFor3DEnvironment(vSafeZonePoints);

    vFinalVectorsList = concatenateVectorsList(vVectorsListA, vVectorsListB);

    vFinalPointsList = concatenatePointsList(vPointsList, vSafeZonePoints);
    
    // on affiche nos résultat dans l'environnement 3D
    view3DEnvironment(convertPointsListAxisToRayLib(vFinalPointsList), convertVectorsListAxisToRayLib(vFinalVectorsList));

    // on applique maintenant le changement de base
    V3 vFirstAxisVector = -prodVect((vSafeZonePoints[0] - vSafeZonePoints[1]),(vSafeZonePoints[2] - vSafeZonePoints[1])); 
    vPointsList = changeCoordinateSystem(vPointsList, vFirstAxisVector);
    vSafeZonePoints = changeCoordinateSystem(vSafeZonePoints, vFirstAxisVector);

    vVectorsListA = buildVectorsListFor3DEnvironment(vPointsList);
    vVectorsListB = buildVectorsListFor3DEnvironment(vSafeZonePoints);

    vFinalVectorsList = concatenateVectorsList(vVectorsListA, vVectorsListB);

    vFinalPointsList = concatenatePointsList(vPointsList, vSafeZonePoints);
    
    // on affiche nos résultat dans l'environnement 3D
    view3DEnvironment(convertPointsListAxisToRayLib(vFinalPointsList), convertVectorsListAxisToRayLib(vFinalVectorsList));

    for (int i = 0; i < vFinalPointsList.size(); i++)
    {
      cout << "Point " << i << " : " << vFinalPointsList[i] << "\n";
    }
}