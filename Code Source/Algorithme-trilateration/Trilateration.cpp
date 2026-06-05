#include "Trilateration.h"

Matrix<float, 6, 3> gA;
Matrix<float, 6, 1> gB;
Matrix<float, 3, 2> gA2D;
Matrix<float, 3, 1> gB2D;

float giveAltitude(float pAtmospheriqPression)
{
    return ((288.15)/(0.0065 * pow(1013.25, (1/5.255)))) * (pow(1013.25, (1/5.255)) - pow(pAtmospheriqPression, (1/5.255)));
}

V3 trilateration3D(UWBModuleList pSensors, unordered_map<int, float> pDistances)
{
    initMatrixB(pSensors, pDistances);
    Matrix<float, 3, 1> vX = ((gA.transpose().eval() * gA).inverse().eval()) * (gA.transpose().eval() * gB);
    return V3(vX(0, 0), vX(1, 0), vX(2, 0));
}

V3 trilateration3D(UWBModuleList pSensors, unordered_map<int, float> pDistances, float pAltitude)
{
    initMatrixB2D(pSensors, pDistances, pAltitude);
    Matrix<float, 2, 1> vX = ((gA2D.transpose().eval() * gA2D).inverse().eval()) * (gA2D.transpose().eval() * gB2D);
    return V3(vX(0, 0), vX(1, 0), pAltitude);
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

void initMatrixA2D(UWBModuleList pSensors)
{
    int vNbSensors = pSensors.size();
    int vNbRowInCramerSystem = (vNbSensors * (vNbSensors - 1))/2;
    int vISensor;
    int vMinISensor;

    for (int i_coord = 1; i_coord <= NUMBER_OF_COORDONATES - 1; i_coord++)
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
            
            gA2D(i_line, i_coord - 1) = 2 * (pSensors.getModule(vISensor).getPosition().getCoordonateNumber(i_coord) - 
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

void initMatrixB2D(UWBModuleList pSensors, unordered_map<int, float> pDistances, float pAltitude)
{
    int vNbSensors = pSensors.size();
    int vNbRowInCramerSystem = (vNbSensors * (vNbSensors - 1))/2;
    int vISensor;
    int vMinISensor;
    V3 vSensorPositionFromFirstPattern;
    V3 vSensorPositionFromSecondPattern;

    float vSquareRadiusFromFirstPattern;
    float vSquareRadiusFromSecondPattern;

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

        vSquareRadiusFromFirstPattern = pDistances[vISensor] * pDistances[vISensor] -
                                (pAltitude - vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_Z)) * (pAltitude - vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_Z));
        
        vSquareRadiusFromSecondPattern = pDistances[(vMinISensor - 1)] * pDistances[(vMinISensor - 1)] -
                                (pAltitude - vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_Z)) * (pAltitude - vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_Z));

        gB2D(i_line, 0) = (vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_X) * vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_X) +
                        vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_Y) * vSensorPositionFromFirstPattern.getCoordonateNumber(AXIS_Y)) - 
                        (vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_X) * vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_X) + 
                        vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_Y) * vSensorPositionFromSecondPattern.getCoordonateNumber(AXIS_Y)) - 
                        (vSquareRadiusFromFirstPattern - vSquareRadiusFromSecondPattern);

        vISensor++;
    }
}