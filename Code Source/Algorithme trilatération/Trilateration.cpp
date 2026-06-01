#include "Trilateration.h"

Matrix<float, 6, 3> gA;
Matrix<float, 6, 1> gB;

V3 trilateration3D(UWBModuleList pSensors, unordered_map<int, float> pDistances)
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

        for (int i_line = 0; i_line < vNbRowInCramerSystem; i_line++)
        {
            if (vISensor > vNbSensors)
            {
                vMinISensor++;
                vISensor = vMinISensor;
            }
            
            gA(i_line, i_coord - 1) = 2 * (pSensors.getModule(vISensor).getPosition().getCoordonateNumber(i_coord) - 
                                        pSensors.getModule(vMinISensor - 1).getPosition().getCoordonateNumber(i_coord));

            vISensor++;
        }
    }
}

void initMatrixB(UWBModuleList pSensors, unordered_map<int, float> pDistances)
{
    int vNbSensors = pSensors.size();
    int vNbRowInCramerSystem = (vNbSensors * (vNbSensors - 1))/2;
    int vISensor;
    int vMinISensor;
    V3 vSensorPositionFromFirstPattern;
    V3 vSensorPositionFromSecondPattern;

    
    vMinISensor = 2;
    vISensor = vMinISensor;

    for (int i_line = 0; i_line < vNbRowInCramerSystem; i_line++)
    {
        if (vISensor > vNbSensors)
        {
            vMinISensor++;
            vISensor = vMinISensor;
        }

        vSensorPositionFromFirstPattern = pSensors.getModule(vISensor).getPosition();
        vSensorPositionFromSecondPattern = pSensors.getModule(vMinISensor - 1).getPosition();

        gB(i_line, 0) = (vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_X) * vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_X) +
                        vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_Y) * vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_Y) + 
                        vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_Z) * vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_Z)) - 
                        (vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_X) * vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_X) + 
                        vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_Y) * vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_Y) + 
                        vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_Z) * vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_Z)) - 
                        (pDistances[vISensor] * pDistances[vISensor] - 
                        pDistances[(vMinISensor - 1)] * pDistances[(vMinISensor - 1)]);

        vISensor++;
    }
    
}