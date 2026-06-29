#include "GridLibrary.hpp"

V3 giveEpicenterPosition(UWBModuleList pAnchors)
{
    std::map<int, V3> vModulePositionList = pAnchors.giveModulePositionList();

    V3 vEpicenterPosition = V3(0.0f, 0.0f, 0.0f);

    // on calcule l'épicentre des ancres (correspond à la moyenne des coordonnées des ancres)
    for (auto it = vModulePositionList.begin(); it != vModulePositionList.end(); it++)
    {
        vEpicenterPosition += it->second;
    }

    vEpicenterPosition = vEpicenterPosition / (vModulePositionList.size());

    return vEpicenterPosition;
}

vector<V3> applyRotationOnPoints(vector<V3> pPoints, Eigen::Matrix<float, 3, 3> pRotationalMatrix)
{
    vector<V3> vNewPoints;

    for (int i = 0; i < pPoints.size(); i++)
    {
        vNewPoints.push_back(pPoints[i] * pRotationalMatrix);
    }

    return vNewPoints;
}

vector<V3> changeCoordinateSystem(vector<V3> pPoints, V3 pNewBasisVector)
{
    // on récupère le premier vecteur unitaire de la nouvelle base qui sera perpendiculaire au plan (X, Y) de notre repère
    V3 vFirstAxisVector = pNewBasisVector.getNormalized();

    // on choisit un vecteur arbitraire t en fonction de la valeur de l'abscisse du vecteur du premier axe
    V3 vTempVector;
    if (abs(vFirstAxisVector.getX()) < 0.9)
    {
        vTempVector = V3(1, 0, 0);
    }
    else
    {
        vTempVector = V3(0, 1, 0);
    }

    // on calcule les coordonnées du vecteur du 2e axe en faisant le produit vectoriel des 2 vecteurs précédents
    V3 vSecondAxisVector = prodVect(vFirstAxisVector, vTempVector);
    vSecondAxisVector.normalize();

    // on en déduit le 3e axe en calculant le produit vectoriel des 2 premiers axes
    V3 vThirdAxisVector = prodVect(vFirstAxisVector, vSecondAxisVector);
    vThirdAxisVector.normalize();

    // on construit la matrice de passage pour passer tous les points dans la nouvelle base
    Eigen::Matrix<float, 3, 3> vTransitionMatrix;
    vTransitionMatrix << vSecondAxisVector.getX(), vSecondAxisVector.getY(), vSecondAxisVector.getZ(),
                        vThirdAxisVector.getX(), vThirdAxisVector.getY(), vThirdAxisVector.getZ(),
                        vFirstAxisVector.getX(), vFirstAxisVector.getY(), vFirstAxisVector.getZ();

    // on calcule les nouvelles coordonnées de tous les points
    vector<V3> vNewPoints;
    V3 vNewCurrentPoint;

    for (int i = 0; i < pPoints.size(); i++)
    {
        vNewCurrentPoint = vTransitionMatrix * pPoints[i];
        vNewPoints.push_back(vNewCurrentPoint);
    }

    return vNewPoints;
}

void alignAnchorsCoordinatesWithGridOrigin(UWBModuleList & pAnchors)
{
    vector<int> vAnchorsIdList = pAnchors.giveModuleIdList();

    V3 vEpicenterPosition = giveEpicenterPosition(pAnchors);

    // on en déduit le vecteur translation
    V3 vTranslationVector = -vEpicenterPosition;
    V3 vNewAnchorPosition;

    // on parcourt pAnchors pour mettre à jour les positions de chaque ancre après la translation
    for (int i = 0; i < vAnchorsIdList.size(); i++)
    {
        vNewAnchorPosition = pAnchors.getModule(vAnchorsIdList[i]).getPosition() + vTranslationVector;
        pAnchors.setModulePosition(vAnchorsIdList[i], vNewAnchorPosition);
    }
}

void initAnchorsCoordinates(UWBModuleList & pAnchors, unordered_map<string, float> pDistances)
{
    // on récupère les véritables identifiants des ancres (0 à 3)
    vector<int> vAnchorIdList = pAnchors.giveModuleIdList();
    sort(vAnchorIdList.begin(), vAnchorIdList.end());

    // Ancre 1 (0,0,0)
    pAnchors.setModulePosition(vAnchorIdList[0], V3(0, 0, 0));

    // Ancre 2
    string vDistanceBtwA1andA2Id = to_string(vAnchorIdList[0] + 1) + to_string(vAnchorIdList[1] + 1);
    float vD1to2 = pDistances[vDistanceBtwA1andA2Id];
    pAnchors.setModulePosition(vAnchorIdList[1], V3(vD1to2, 0, 0));

    // Ancre 3
    string vDistanceBtwA1andA3Id = to_string(vAnchorIdList[0] + 1) + to_string(vAnchorIdList[2] + 1);
    float vD1to3 = pDistances[vDistanceBtwA1andA3Id];
    string vDistanceBtwA2andA3Id = to_string(vAnchorIdList[1] + 1) + to_string(vAnchorIdList[2] + 1);
    float vD2to3 = pDistances[vDistanceBtwA2andA3Id];

    float vX3 = (vD1to2 * vD1to2 + vD1to3 * vD1to3 - vD2to3 * vD2to3) / (2 * vD1to2);
    float vY3 = sqrt(abs(vD1to3 * vD1to3 - vX3 * vX3)); // Sécurité abs() ajoutée
    pAnchors.setModulePosition(vAnchorIdList[2], V3(vX3, vY3, 0));

    // Ancre 4
    string vDistanceBtwA1andA4Id = to_string(vAnchorIdList[0] + 1) + to_string(vAnchorIdList[3] + 1);
    float vD1to4 = pDistances[vDistanceBtwA1andA4Id];
    string vDistanceBtwA2andA4Id = to_string(vAnchorIdList[1] + 1) + to_string(vAnchorIdList[3] + 1); 
    float vD2to4 = pDistances[vDistanceBtwA2andA4Id];
    string vDistanceBtwA3andA4Id = to_string(vAnchorIdList[2] + 1) + to_string(vAnchorIdList[3] + 1); 
    float vD3to4 = pDistances[vDistanceBtwA3andA4Id];

    float vX4 = (vD1to2 * vD1to2 + vD1to4 * vD1to4 - vD2to4 * vD2to4) / (2 * vD1to2);
    float vY4 = (vD1to4 * vD1to4 - vD3to4 * vD3to4 + vX3 * vX3 + vY3 * vY3 - 2 * vX3 * vX4) / (2 * vY3);
    float vZ4 = sqrt(abs(vD1to4 * vD1to4 - vX4 * vX4 - vY4 * vY4)); // Sécurité abs() ajoutée

    pAnchors.setModulePosition(vAnchorIdList[3], V3(vX4, vY4, vZ4));
}

void initAnchorsCoordinatesWithGD(UWBModuleList & pAnchors, unordered_map<string, float> pDistances, int pIter, float pAlpha)
{
    vector<int> vAnchorIdList = pAnchors.giveModuleIdList();
    sort(vAnchorIdList.begin(), vAnchorIdList.end());

    int vAnchorAmount = vAnchorIdList.size();
    string vKey;

    for (int iter = 0; iter < pIter; iter++)
    {
        vector<V3> vGradients(vAnchorAmount, V3(0, 0, 0));

        // 1. Calcul des forces (gradients) pour chaque paire d'ancres
        for (int i = 0; i < vAnchorAmount; i++)
        {
            for (int j = 0; j < vAnchorAmount; j++)
            {
                if (i == j) continue;

                // Identifiants internes compris entre 0 et 3
                int minId = min(vAnchorIdList[i], vAnchorIdList[j]);
                int maxId = max(vAnchorIdList[i], vAnchorIdList[j]);
                
                // Transformation magique : id de 0 à 3 convertis en clés textuelles de 1 à 4 ("12", "13", etc.)
                vKey = to_string(minId + 1) + to_string(maxId + 1);

                // Si la clé n'est pas dans le dictionnaire, on passe au suivant
                if (pDistances.find(vKey) == pDistances.end()) continue;

                float vGoalDistance = pDistances[vKey];
                V3 posI = pAnchors.giveModulePositionList()[vAnchorIdList[i]];
                V3 posJ = pAnchors.giveModulePositionList()[vAnchorIdList[j]];

                V3 diff = posI - posJ;
                float vCurrentDistance = diff.norm();

                // Sécurité anti-division par zéro
                if (vCurrentDistance < 0.001f) {
                    diff = V3((float)rand()/RAND_MAX, (float)rand()/RAND_MAX, (float)rand()/RAND_MAX);
                    vCurrentDistance = diff.norm();
                }

                // Formule de gradient optimisée et stable
                float vError = (vCurrentDistance - vGoalDistance) / vCurrentDistance;
                vGradients[i] += diff * vError;
            }
        }

        // 2. Application du déplacement avec respect des contraintes géométriques du repère virtuel
        for (int i = 0; i < vAnchorAmount; i++)
        {
            V3 vCurrentPos = pAnchors.giveModulePositionList()[vAnchorIdList[i]];
            V3 vNewPos = vCurrentPos - (vGradients[i] * (pAlpha / vAnchorAmount));

            if (i == 0) // Ancre 0 : Bloquée à l'origine (0,0,0)
            {
                vNewPos = V3(0.0f, 0.0f, 0.0f);
            }
            else if (i == 1) // Ancre 1 : Alignée strictement sur l'axe X (y=0, z=0)
            {
                vNewPos = V3(vNewPos.x, 0.0f, 0.0f);
            }
            else if (i == 2) // Ancre 2 : Contrainte sur le plan XY (z=0)
            {
                vNewPos = V3(vNewPos.x, vNewPos.y, 0.0f);
            }
            // Ancre 3 : Libre de se déplacer en 3D (x,y,z)

            pAnchors.setModulePosition(vAnchorIdList[i], vNewPos);
        }
    }
}

Eigen::Matrix<float, 3, 3> giveRotationalMatrix(V3 pStartVector, V3 pResultVector)
{
    V3 vUnitStartVector = pStartVector.getNormalized();
    V3 vUnitResultVector = pResultVector.getNormalized();
    Eigen::Matrix<float, 3, 3> vR;

    // on calcule la valeur du vecteur qui modélise l'axe de rotation auquel le vecteur pStartVector devra tourner pour donner le pResultVector
    V3 vRotationalAxis = prodVect(vUnitStartVector, vUnitResultVector);

    // puis on calcule le cosinus de l'angle entre pStartVector et pResultVector
    float vAngleCosinus = prodScal(vUnitStartVector, vUnitResultVector);

    Eigen::Matrix<float, 3, 3> vAntiSymmetric;
    vAntiSymmetric << 0.0f, -vRotationalAxis.getZ(), vRotationalAxis.getY(),
                    vRotationalAxis.getZ(), 0.0f, -vRotationalAxis.getX(),
                    -vRotationalAxis.getY(), vRotationalAxis.getX(), 0.0f;
    
    Eigen::Matrix<float, 3, 3> vIdentity = Eigen::Matrix<float, 3, 3>::Identity();

    vR = vIdentity + vAntiSymmetric + (1/(1 + vAngleCosinus)) * (vAntiSymmetric * vAntiSymmetric);
    
    return vR;

}