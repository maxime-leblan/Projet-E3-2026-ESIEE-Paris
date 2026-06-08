#include <iostream>

#include "Trilateration.h"

#define NUMBER_OF_SENSORS 4

/*
Commande de compilation : 
g++ -O3 *.cpp -o main_test -I /usr/include/eigen3
*/

int main()
{
    // on instancie les capteurs
    UWBModuleList vSensors = UWBModuleList();
    UWBModule vTag1 = UWBModule(1, V3(9, 6, 3));
    vector<V3> vSensorsPosition = {V3(3, 0, 1), V3(0, 3, 0), V3(3, 6, 0), V3(6, 3, 0)};

    // on les ajoute dans la liste des capteurs
    for (int i = 1; i <= NUMBER_OF_SENSORS; i++)
    {
        UWBModule vTemp = UWBModule(i, vSensorsPosition[i - 1]);
        vSensors.addModule(vTemp.getId(), vTemp);
    }

    // on calcule les distances réelles entre chaque capteur
    unordered_map<int, float> vDistances;

    for (int i = 1; i <= NUMBER_OF_SENSORS; i++)
    {
        vDistances[i] = (vSensors.getModule(i).getPosition() - vTag1.getPosition()).norm();
    }

    // on récupère les coordonnées du vTag1 avec notre algorithme de triangulation
    initMatrixA(vSensors);
    V3 vRealTag1 = trilateration3D(vSensors, vDistances);

    V3 vTag1Position = vTag1.getPosition();

    cout << "Véritable position tag1 : " << vTag1Position << "\n";
    cout << "Position tag1 calculée par trilatération : " << vRealTag1 << "\n";

    // ------------------------------------------------------------------------------------------------------------------------

    // On veut maintenant calculer les coordonnées du Tag avec l'algorithme de trilatération utilisant 3 ancres et 2 baromètres
    // on crée une nouvelle liste de capteurs
    UWBModuleList vSensorsBis = UWBModuleList();
    vector<V3> vSensorsPositionBis = {V3(0, 3, 0), V3(3, 6, 0), V3(6, 3, 0)};

    // on les ajoute dans la liste des capteurs
    for (int i = 1; i <= NUMBER_OF_SENSORS-1; i++)
    {
        UWBModule vTemp = UWBModule(i, vSensorsPositionBis[i - 1]);
        vSensorsBis.addModule(vTemp.getId(), vTemp);
    }

    // on calcule les distances réelles entre chaque capteur
    unordered_map<int, float> vDistancesBis;

    for (int i = 1; i <= NUMBER_OF_SENSORS-1; i++)
    {
        vDistancesBis[i] = (vSensorsBis.getModule(i).getPosition() - vTag1.getPosition()).norm();
    }

    // on récupère les coordonnées du vTag1 avec notre algorithme de triangulation
    initMatrixA2D(vSensorsBis);

    float vBar1Mesure = 1013.25;
    float vBar2Mesure = 1012.89;

    float vAltitudeTag1 = abs(giveAltitude(vBar1Mesure) - giveAltitude(vBar2Mesure));

    V3 vRealTag1Bis = trilateration3D(vSensorsBis, vDistancesBis, vAltitudeTag1);
    cout << "Position tag1 calculée par trilatération avec baromètres : " << vRealTag1Bis << "\n";
}