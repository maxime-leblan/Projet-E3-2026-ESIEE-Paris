#include "InitAnchorPosition.hpp"
#include "Config.hpp"
#include "RecuperationDonneesAncres.hpp"
#include <Arduino.h>
#include <algorithm>

extern RecuperationDonneesAncres recupDonnees;
extern void ecouterReseauFilaire();

void setAnchorRole(uint8_t pAnchorId, uint8_t pRoleOrder)
{
    MsgAnchorCalibHubOrder vMessage = {pAnchorId};
    sendCanOrderFromHubTo(pAnchorId, pRoleOrder, vMessage);
    Serial.printf("[HUB CALIB] Ordre de role (Ordre: %d) envoye a l'ancre %d\n", pRoleOrder, pAnchorId);
}

void sendToAnchorsInitialisationPhaseSignal(UWBModuleList & pAnchors, uint8_t pSignalType)
{
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();
    for (uint8_t vCurrentId : vAnchorsId)
    {
        MsgAnchorCalibHubOrder vMessage = {vCurrentId};
        sendCanOrderFromHubTo(vCurrentId, pSignalType, vMessage);
    }
}

void initAnchorsPosition(UWBModuleList & pAnchors)
{
    Serial.println("\n[HUB CALIB] === DEBUT DU PROTOCOLE DE CALIBRATION SPATIALE ===");
    
    // 1. Mise en mode écoute de toutes les ancres
    sendToAnchorsInitialisationPhaseSignal(pAnchors, HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL);
    delay(500);

    std::unordered_map<std::string, float> vAnchorDistances;
    std::vector<int> vAnchorsId = pAnchors.giveModuleIdList();

    // 2. Rotation : Chaque ancre devient le Tag cible à tour de rôle
    for (int idCibleTag : vAnchorsId)
    {
        Serial.printf("\n[HUB CALIB] --- Phase: L'Ancre %d devient le TAG temporaire ---\n", idCibleTag);

        // Assignation explicite des rôles matériels
        for (int idCurrent : vAnchorsId)
        {
            if (idCurrent == idCibleTag) {
                setAnchorRole(idCurrent, HUB_ORDER_SET_AS_TAG);
            } else {
                setAnchorRole(idCurrent, HUB_ORDER_SET_AS_ANCHOR);
            }
        }

        delay(1000); // Stabilisation de la topologie radio UWB
        recupDonnees.effacerDonneesTag(idCibleTag); // Nettoyage de l'accumulateur

        unsigned long startTime = millis();
        
        // 3. Polling et Collecte (3 secondes par configuration)
        while (millis() - startTime < 3000)
        {
            // Le Hub interroge les modules restés "Ancres" sur la distance les séparant du nouveau "Tag"
            for (int idAncre : vAnchorsId)
            {
                if (idAncre != idCibleTag) {
                    sendCanRequestDistances(idAncre, idCibleTag);
                    delay(10);
                }
            }

            ecouterReseauFilaire();

            // Extraction des données lissées pour le module agissant en tant que Tag
            DistanceMoyennes tagMoyenne;
            if (recupDonnees.getDonneesLisseesPourTag(idCibleTag, tagMoyenne))
            {
                for (int idAncre : vAnchorsId)
                {
                    if (idAncre != idCibleTag)
                    {
                        float dist = tagMoyenne.distances[idAncre];
                        if (dist > 0.0f)
                        {
                            // Création d'une clé unique standardisée (ex: "12", "23") requise par GridLibrary
                            int minId = std::min(idAncre, idCibleTag);
                            int maxId = std::max(idAncre, idCibleTag);
                            std::string vKey = std::to_string(minId + 1) + std::to_string(maxId + 1);

                            // Ajout si la distance n'est pas encore enregistrée
                            if (vAnchorDistances.find(vKey) == vAnchorDistances.end() || vAnchorDistances[vKey] == 0) {
                                vAnchorDistances[vKey] = dist;
                                Serial.printf("[HUB CALIB] Succes : Distance validee %s -> %.2f cm\n", vKey.c_str(), dist);
                            }
                        }
                    }
                }
            }
            delay(20);
        }
    }

    Serial.println("\n[HUB CALIB] Fin de la collecte des distances inter-ancres.");
    
    // 4. Restauration de sécurité : Tout le monde redevient Ancre
    for (int idCurrent : vAnchorsId)
    {
        setAnchorRole(idCurrent, HUB_ORDER_SET_AS_ANCHOR);
    }
    delay(500);

    // 5. Clôture de la séquence
    sendToAnchorsInitialisationPhaseSignal(pAnchors, HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL);

    // 6. Résolution géométrique
    Serial.println("[HUB CALIB] Traitement mathématique (Descente de Gradient) en cours...");
    initAnchorsCoordinatesWithGD(pAnchors, vAnchorDistances, ITERATIONS, LEARNING_RATE);
    
    Serial.println("[HUB CALIB] === FIN DU PROTOCOLE DE CALIBRATION ===");
}

