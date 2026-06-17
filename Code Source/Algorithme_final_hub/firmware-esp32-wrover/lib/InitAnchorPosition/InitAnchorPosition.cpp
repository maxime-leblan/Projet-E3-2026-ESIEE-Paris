#include "InitAnchorPosition.hpp"

std::unordered_map<string, float> getAnchorDistances(int pAnchorId, UWBModuleList pAnchors)
{
    std::unordered_map<string, float> vDistances;
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();
    
    // 1. Filtrage propre
    vAnchorsId.erase(std::remove(vAnchorsId.begin(), vAnchorsId.end(), pAnchorId), vAnchorsId.end());

    toggleAnchorsMode(vAnchorsId, pAnchorId);

    for (int vCurrentId : vAnchorsId)
    {
        unsigned long startTime = millis();
        bool received = false;
        
        // Timeout de 500ms par ancre pour éviter le blocage total
        while (millis() - startTime < 500) 
        {
            twai_message_t vMessage;
            if (receiveCanMessage(vMessage)) 
            {
                DecodedData vData;
                if (decodeCanMessage(vMessage, vData)) 
                {
                    // Validation stricte : est-ce le bon type et la bonne ancre ?
                    if (vData.type == MESSAGE_TAG_ID_AND_DISTANCE && vData.id_ancre == vCurrentId)
                    {
                        // on enregistre la distance uniquement si elle n'a pas déjà été calculé dans l'autre sens auparavant
                        // 
                        if (pAnchorId < vData.id_ancre)
                        {
                            string vKey = to_string(pAnchorId + 1) + to_string(vData.id_ancre + 1);
                            vDistances[vKey] = vData.distance;
                        }
                        received = true;
                        break; // Sort du while
                    }
                }
            }
        }
        if (!received) Serial.printf("Timeout : Ancre %d n'a pas répondu.\n", vCurrentId);
    }

    toggleAnchorsMode(vAnchorsId, pAnchorId);
    return vDistances;
}

void initAnchorsPosition(UWBModuleList & pAnchors)
{
    std::unordered_map<string, float> vAnchorDistances;
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();

    for (int vCurrentId : vAnchorsId)
    {
        std::unordered_map<string, float> vCurrentAnchorDistances = getAnchorDistances(vCurrentId, pAnchors);

        // on ajoute toutes les distances qui ont pu être récupérées dans le dictionnaire final
        for (auto it = vCurrentAnchorDistances.begin(); it != vCurrentAnchorDistances.end(); it++)
        {
            vAnchorDistances[it->first] = it->second;
        }
    }

    // on calcule et on attribue des coordonnées de base aux Ancres du véhicule
    initAnchorsCoordinatesWithGD(pAnchors, vAnchorDistances, ITERATIONS, LEARNING_RATE);
}

void toggleAnchorsMode(std::vector<int> pAnchorsId, uint8_t pStaticAnchorId)
{
    for (int i = 0; i < pAnchorsId.size(); i++)
    {
        MsgToggleHubOrder vMessage;
        vMessage.staticAnchorId = pStaticAnchorId;
        sendCanOrderFromHubTo(pAnchorsId[i], HUB_ORDER_TOGGLE_MODULE_MODE, vMessage);
    }
}