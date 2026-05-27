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
    UWBModule vTag1 = UWBModule("5", V3(9, 6, 3));
    vector<V3> vSensorsPosition = {V3(3, 0, 1), V3(0, 3, 0), V3(3, 6, 0), V3(6, 3, 0)};

    // on les ajoute dans la liste des capteurs
    for (int i = 1; i <= NUMBER_OF_SENSORS; i++)
    {
        UWBModule vTemp = UWBModule(to_string(i), vSensorsPosition[i - 1]);
        vSensors.addModule(vTemp.getName(), vTemp);
    }

    // on calcule les distances réelles entre chaque capteur
    unordered_map<string, float> vDistances;

    for (int i = 1; i <= NUMBER_OF_SENSORS; i++)
    {
        vDistances[to_string(i)] = (vSensors.getModule(to_string(i)).getPosition() - vTag1.getPosition()).norm();
    }

    // on récupère les coordonnées du vTag1 avec notre algorithme de triangulation
    initMatrixA(vSensors);
    V3 vRealTag1 = trilateration3D(vSensors, vDistances);

    V3 vTag1Position = vTag1.getPosition();

    cout << "Véritable position tag1 : " << vTag1Position << "\n";
    cout << "Position tag1 calculée par trilatération : " << vRealTag1 << "\n";
    
}