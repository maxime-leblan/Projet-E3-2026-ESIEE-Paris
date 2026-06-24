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
    std::vector<int> aIds = pSensors.giveModuleIdList();
    std::sort(aIds.begin(), aIds.end());
    int vNbSensors = aIds.size();

    for (int i_coord = 1; i_coord <= NUMBER_OF_COORDONATES; i_coord++)
    {
        int i_line = 0;
        for (int i = 0; i < vNbSensors - 1; i++) 
        {
            for (int j = i + 1; j < vNbSensors; j++) 
            {
                gA(i_line, i_coord - 1) = 2 * (pSensors.getModule(aIds[j]).getPosition().getCoordonateNumber(i_coord) - 
                                               pSensors.getModule(aIds[i]).getPosition().getCoordonateNumber(i_coord));
                i_line++;
            }
        }
    }
}

void initMatrixA2D(UWBModuleList pSensors)
{
    std::vector<int> aIds = pSensors.giveModuleIdList();
    std::sort(aIds.begin(), aIds.end());
    int vNbSensors = aIds.size();

    for (int i_coord = 1; i_coord <= NUMBER_OF_COORDONATES - 1; i_coord++)
    {
        int i_line = 0;
        for (int i = 0; i < vNbSensors - 1; i++) 
        {
            for (int j = i + 1; j < vNbSensors; j++) 
            {
                gA2D(i_line, i_coord - 1) = 2 * (pSensors.getModule(aIds[j]).getPosition().getCoordonateNumber(i_coord) - 
                                                 pSensors.getModule(aIds[i]).getPosition().getCoordonateNumber(i_coord));
                i_line++;
            }
        }
    }
}

void initMatrixB(UWBModuleList pSensors, unordered_map<int, float> pDistances)
{
    std::vector<int> aIds = pSensors.giveModuleIdList();
    std::sort(aIds.begin(), aIds.end());
    int vNbSensors = aIds.size();
    
    int i_line = 0;
    for (int i = 0; i < vNbSensors - 1; i++) 
    {
        for (int j = i + 1; j < vNbSensors; j++) 
        {
            V3 posJ = pSensors.getModule(aIds[j]).getPosition();
            V3 posI = pSensors.getModule(aIds[i]).getPosition();
            
            gB(i_line, 0) = (posJ.getCoordonateNumber(AXIS_X)*posJ.getCoordonateNumber(AXIS_X) + 
                             posJ.getCoordonateNumber(AXIS_Y)*posJ.getCoordonateNumber(AXIS_Y) + 
                             posJ.getCoordonateNumber(AXIS_Z)*posJ.getCoordonateNumber(AXIS_Z)) - 
                            (posI.getCoordonateNumber(AXIS_X)*posI.getCoordonateNumber(AXIS_X) + 
                             posI.getCoordonateNumber(AXIS_Y)*posI.getCoordonateNumber(AXIS_Y) + 
                             posI.getCoordonateNumber(AXIS_Z)*posI.getCoordonateNumber(AXIS_Z)) - 
                            (pDistances[aIds[j]] * pDistances[aIds[j]] - 
                             pDistances[aIds[i]] * pDistances[aIds[i]]);
            i_line++;
        }
    }
}

void initMatrixB2D(UWBModuleList pSensors, unordered_map<int, float> pDistances, float pAltitude)
{
    std::vector<int> aIds = pSensors.giveModuleIdList();
    std::sort(aIds.begin(), aIds.end());
    int vNbSensors = aIds.size();
    
    int i_line = 0;
    for (int i = 0; i < vNbSensors - 1; i++) 
    {
        for (int j = i + 1; j < vNbSensors; j++) 
        {
            V3 posJ = pSensors.getModule(aIds[j]).getPosition();
            V3 posI = pSensors.getModule(aIds[i]).getPosition();
            
            float vSqRadJ = pDistances[aIds[j]] * pDistances[aIds[j]] -
                            (pAltitude - posJ.getCoordonateNumber(AXIS_Z)) * (pAltitude - posJ.getCoordonateNumber(AXIS_Z));
            float vSqRadI = pDistances[aIds[i]] * pDistances[aIds[i]] -
                            (pAltitude - posI.getCoordonateNumber(AXIS_Z)) * (pAltitude - posI.getCoordonateNumber(AXIS_Z));
            
            gB2D(i_line, 0) = (posJ.getCoordonateNumber(AXIS_X)*posJ.getCoordonateNumber(AXIS_X) + 
                               posJ.getCoordonateNumber(AXIS_Y)*posJ.getCoordonateNumber(AXIS_Y)) - 
                              (posI.getCoordonateNumber(AXIS_X)*posI.getCoordonateNumber(AXIS_X) + 
                               posI.getCoordonateNumber(AXIS_Y)*posI.getCoordonateNumber(AXIS_Y)) - 
                              (vSqRadJ - vSqRadI);
            i_line++;
        }
    }
}