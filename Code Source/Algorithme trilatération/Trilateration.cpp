#include "Trilateration.h"
#include <Eigen/Dense>
#include <string>
#include <unordered_map>

Matrix<float, 6, 3> gA;
Matrix<float, 6, 1> gB;

V3 trilateration3D(UWBModuleList pSensors, unordered_map<string, float> pDistances)
{
    initMatrixB(pSensors, pDistances);
    Matrix<float, 3, 1> vX = ((gA.transpose().eval() * gA).inverse().eval()) * (gA.transpose().eval() * gB);
    return V3(vX(0, 0), vX(1, 0), vX(2, 0));
}

void initMatrixA(UWBModuleList pSensors)
{
    int vNbSensors = pSensors.size();
    int vNbRowInCramerSystem = (vNbSensors * (vNbSensors - 1))/2;
    int vISensor;
    int vMinISensor;

    for (int i_coord = 1; i_coord <= NUMBER_OF_COORDONATES; i_coord++)
    {
        vMinISensor = 2;
        vISensor = vMinISensor;

        for (int i_line = 1; i_line <= vNbRowInCramerSystem; i_line++)
        {
            if (vISensor > vNbSensors)
            {
                vMinISensor++;
                vISensor = vMinISensor;
            }
            
            gA(i_line-1, i_coord-1) = 2 * (pSensors.getModule(to_string(vISensor)).getPosition().getCoordonateNumber(i_coord) - 
                                        pSensors.getModule(to_string(vMinISensor - 1)).getPosition().getCoordonateNumber(i_coord));

            vISensor++;
        }
    }
}

void initMatrixB(UWBModuleList pSensors, unordered_map<string, float> pDistances)
{
    int vNbSensors = pSensors.size();
    int vNbRowInCramerSystem = (vNbSensors * (vNbSensors - 1))/2;
    int vISensor;
    int vMinISensor;
    V3 vSensorPositionFromFirstPattern;
    V3 vSensorPositionFromSecondPattern;

    
    vMinISensor = 2;
    vISensor = vMinISensor;

    for (int i_line = 1; i_line <= vNbRowInCramerSystem; i_line++)
    {
        if (vISensor > vNbSensors)
        {
            vMinISensor++;
            vISensor = vMinISensor;
        }

        vSensorPositionFromFirstPattern = pSensors.getModule(to_string(vISensor)).getPosition();
        vSensorPositionFromSecondPattern = pSensors.getModule(to_string(vMinISensor - 1)).getPosition();

        gB(i_line - 1, 0) = (vSensorPositionFromFirstPattern.getCoordonateNumber(1) * vSensorPositionFromFirstPattern.getCoordonateNumber(1) +
                        vSensorPositionFromFirstPattern.getCoordonateNumber(2) * vSensorPositionFromFirstPattern.getCoordonateNumber(2) + 
                        vSensorPositionFromFirstPattern.getCoordonateNumber(3) * vSensorPositionFromFirstPattern.getCoordonateNumber(3)) - 
                        (vSensorPositionFromSecondPattern.getCoordonateNumber(1) * vSensorPositionFromSecondPattern.getCoordonateNumber(1) + 
                        vSensorPositionFromSecondPattern.getCoordonateNumber(2) * vSensorPositionFromSecondPattern.getCoordonateNumber(2) + 
                        vSensorPositionFromSecondPattern.getCoordonateNumber(3) * vSensorPositionFromSecondPattern.getCoordonateNumber(3)) - 
                        (pDistances[to_string(vISensor)] * pDistances[to_string(vISensor)] - 
                        pDistances[to_string(vMinISensor - 1)] * pDistances[to_string(vMinISensor - 1)]);

        vISensor++;
    }
    
}