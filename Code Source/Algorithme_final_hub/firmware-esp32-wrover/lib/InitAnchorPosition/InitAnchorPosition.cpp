#include "InitAnchorPosition.hpp"
#include "Config.hpp"
#include "RecuperationDonneesAncres.hpp"
#include <Arduino.h>
#include <algorithm> // Pour std::remove

// Déclarations externes nécessaires pour faire le lien avec le main.cpp
extern RecuperationDonneesAncres recupDonnees;
extern void ecouterReseauFilaire();

std::unordered_map<std::string, float> getAnchorDistances(int pAnchorId, UWBModuleList pAnchors)
{
    Serial.println("[INIT ANCRES] On va calculer les distances relatives à l'ancre d'id " + String(pAnchorId));
    std::unordered_map<std::string, float> vDistances;
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();
   
    // Filtrage strict : on isole l'id de l'ancre qui effectue la mesure
    vAnchorsId.erase(std::remove(vAnchorsId.begin(), vAnchorsId.end(), pAnchorId), vAnchorsId.end());

    // Basculement de rôle : Les autres modules deviennent temporairement des Tags
    toggleAnchorsMode(vAnchorsId, pAnchorId);

    unsigned long startTime = millis();
    
    // Timeout de 5000ms pour garantir plusieurs cycles de polling
    while (millis() - startTime < 5000) 
    {
        // 1. Polling : L'ancre statique demande les distances pour les modules devenus Tags
        for (int vCurrentTagId : vAnchorsId) 
        {
            sendCanRequestDistances(pAnchorId, vCurrentTagId);
            delay(10); // Délai de traitement du bus matériel
        }

        // 2. Réception : Vidage du buffer TWAI/CAN vers l'accumulateur 'recupDonnees'
        ecouterReseauFilaire();

        // 3. Extraction ciblée : On interroge les données lissées uniquement pour les tags ciblés
        for (int vCurrentTagId : vAnchorsId) 
        {
            DistanceMoyennes tagMoyenne;
            
            // Si des données lissées sont disponibles pour ce module précis
            if (recupDonnees.getDonneesLisseesPourTag(vCurrentTagId, tagMoyenne)) 
            {
                // Règle d'unicité : on ne stocke que si l'ID demandeur est inférieur à l'ID cible (évite les doublons 1-2 et 2-1)
                if (pAnchorId < vCurrentTagId) 
                {
                    if (tagMoyenne.distances[pAnchorId] > 0.0f) 
                    {
                        std::string vKey = std::to_string(pAnchorId + 1) + std::to_string(vCurrentTagId + 1);
                        vDistances[vKey] = tagMoyenne.distances[pAnchorId];
                    }
                }
            }
        }
        delay(20);
    }

    // Rétablissement : on renvoie le même ordre pour que les modules redeviennent des Ancres
    toggleAnchorsMode(vAnchorsId, pAnchorId);
    
    Serial.println("[INIT ANCRES] Fin du calcul des distances relatives à l'ancre d'id " + String(pAnchorId));
    return vDistances;
}

void initAnchorsPosition(UWBModuleList & pAnchors)
{
    Serial.println("[INIT ANCRES] ---- DEMARRAGE INIT ANCRES PROTOCOLE ----");
    sendToAnchorsInitialisationPhaseSignal(pAnchors, HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL);

    std::unordered_map<std::string, float> vAnchorDistances;
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();

    for (int vCurrentId : vAnchorsId)
    {
        std::unordered_map<std::string, float> vCurrentAnchorDistances = getAnchorDistances(vCurrentId, pAnchors);

        for (auto it = vCurrentAnchorDistances.begin(); it != vCurrentAnchorDistances.end(); ++it)
        {
            vAnchorDistances[it->first] = it->second;
        }
    }

    sendToAnchorsInitialisationPhaseSignal(pAnchors, HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL);

    Serial.println("[INIT ANCRES] ---- REALIGNEMENT COORD ANCRES ----");
    // Calcul mathématique des coordonnées à partir de la matrice des distances
    initAnchorsCoordinatesWithGD(pAnchors, vAnchorDistances, ITERATIONS, LEARNING_RATE);
    Serial.println("[INIT ANCRES] ---- FIN INIT ANCRES PROTOCOLE ----");
}

void sendToAnchorsInitialisationPhaseSignal(UWBModuleList & pAnchors, int pSignalType)
{
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();

    Serial.println("[INIT ANCRES] On envoie à toutes les ancres le signal d'init : " + String(pSignalType));

    for (uint8_t vCurrentId : vAnchorsId)
    {
        MsgAnchorCalibHubOrder vMessage = {vCurrentId};
        sendCanOrderFromHubTo(vCurrentId, pSignalType, vMessage);
    }
}

void toggleAnchorsMode(std::vector<int> pAnchorsId, uint8_t pStaticAnchorId)
{
    Serial.println("[INIT ANCRES] On ordonne aux ancres de changer de mode (ID Ancre statique : " + String(pStaticAnchorId) + ")");
    for (size_t i = 0; i < pAnchorsId.size(); i++)
    {
        MsgToggleHubOrder vMessage;
        vMessage.staticAnchorId = pStaticAnchorId;
        sendCanOrderFromHubTo(pAnchorsId[i], HUB_ORDER_TOGGLE_MODULE_MODE, vMessage);
    }
}

