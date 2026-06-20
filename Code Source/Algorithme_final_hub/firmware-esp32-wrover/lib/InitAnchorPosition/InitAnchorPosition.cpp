#include "InitAnchorPosition.hpp"
#include "Config.hpp"
#include "WifiMessageManager.hpp"
#include "RecuperationDonneesAncres.hpp"

// Déclaré dans le main.cpp pour lire les données accumulées
extern RecuperationDonneesAncres recupDonnees;

std::unordered_map<string, float> getAnchorDistances(int pAnchorId, UWBModuleList pAnchors)
{
    std::unordered_map<string, float> vDistances;
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();
   
    // 1. Filtrage propre
    vAnchorsId.erase(std::remove(vAnchorsId.begin(), vAnchorsId.end(), pAnchorId), vAnchorsId.end());

    // Le paramètre "true" indique au Wi-Fi de devenir un Tag
    toggleAnchorsMode(vAnchorsId, pAnchorId, true);

    if (MODE_ACTUEL == MODE_WIFI) 
    {
        unsigned long startTime = millis();
        
        // Timeout de 500ms global pour le WiFi pour laisser les tags remonter l'info
        while (millis() - startTime < 500) {
            delay(10); 
        }

        std::vector<DistanceMoyennes> tagsLisses;
        if (recupDonnees.getDonneesLissees(tagsLisses)) {
            for (int vCurrentId : vAnchorsId) {
                for (const auto& tag : tagsLisses) {
                    if (tag.tag_id == vCurrentId) {
                        if (pAnchorId < vCurrentId) {
                            string vKey = to_string(pAnchorId + 1) + to_string(vCurrentId + 1);
                            vDistances[vKey] = tag.distances[pAnchorId];
                        }
                    }
                }
            }
        }
    }
    else 
    {
        // ==========================================================
        // CODE CAN ORIGINAL (Ne pas modifier)
        // ==========================================================
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
    }

    // Le paramètre "false" indique au Wi-Fi de redevenir une Ancre
    toggleAnchorsMode(vAnchorsId, pAnchorId, false);
    return vDistances;
}

void initAnchorsPosition(UWBModuleList & pAnchors)
{
    // On envoie d'abords un message à toutes les ancres pour leur indiquer que la phase d'initialisation débute
    sendToAnchorsInitialisationPhaseSignal(pAnchors, HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL);

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

    // On envoie un message à toutes les ancres pour leur indiquer que la phase d'initialisation se termine
    sendToAnchorsInitialisationPhaseSignal(pAnchors, HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL);

    // on calcule et on attribue des coordonnées de base aux Ancres du véhicule
    initAnchorsCoordinatesWithGD(pAnchors, vAnchorDistances, ITERATIONS, LEARNING_RATE);
}

void sendToAnchorsInitialisationPhaseSignal(UWBModuleList & pAnchors, int pSignalType)
{
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();

    for (uint8_t vCurrentId : vAnchorsId)
    {
        // on envoie à chaque Ancre un message contenant pSignalType pour leur indiquer si on débute ou termine la phase d'initialisation
        MsgAnchorCalibHubOrder vMessage = {vCurrentId};
        sendCanOrderFromHubTo(vCurrentId, pSignalType, vMessage);
    }
}

void toggleAnchorsMode(std::vector<int> pAnchorsId, uint8_t pStaticAnchorId, bool becomeTags)
{
    for (int i = 0; i < pAnchorsId.size(); i++)
    {
        if (MODE_ACTUEL == MODE_WIFI) 
        {
            // Commande Wi-Fi : 1 = Deviens un Tag, 0 = Deviens une Ancre
            uint8_t command = becomeTags ? 1 : 0;
            envoyerOrdreChangementRole(pAnchorsId[i], command);
        } 
        else 
        {
            // Commande CAN d'origine
            MsgToggleHubOrder vMessage;
            vMessage.staticAnchorId = pStaticAnchorId;
            sendCanOrderFromHubTo(pAnchorsId[i], HUB_ORDER_TOGGLE_MODULE_MODE, vMessage);
        }
    }
}

