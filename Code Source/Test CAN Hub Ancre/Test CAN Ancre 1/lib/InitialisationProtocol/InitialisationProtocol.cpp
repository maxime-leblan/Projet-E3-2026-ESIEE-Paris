#include "InitialisationProtocol.hpp"

void runInitializationPhase(uint8_t pMyAnchorId)
{
    bool isTagMode = false;
    twai_message_t vMessage;
    DecodedData vData;

    Serial.printf("[INIT] Ancre %d en attente du signal pour passer en Tag...\n", pMyAnchorId);

    // =========================================================
    // PHASE 1 : Attente du signal du Hub pour devenir un Tag
    // =========================================================
    while (!isTagMode) 
    {
        if (receiveCanMessage(vMessage)) 
        {
            if (decodeCanMessage(vMessage, vData)) 
            {
                // Vérification : Ordre du Hub + Destiné à Moi + Ordre de Toggle
                if (vData.type == MESSAGE_HUB_ORDER && 
                    vData.id_ancre == pMyAnchorId && 
                    vData.aOrderType == HUB_ORDER_TOGGLE_MODULE_MODE) 
                {
                    Serial.println("[INIT] Ordre reçu ! Passage en mode TAG.");
                    isTagMode = true;
                }
            }
        }
        delay(5); // Petite pause pour ne pas surcharger le processeur (Anti-Watchdog)
    }

    // On exécute ta fonction pour changer le rôle du module UWB via AT Commands
    toggleUWBMode(pMyAnchorId);

    // =========================================================
    // PHASE 2 : Envoi des distances en boucle jusqu'au stop
    // =========================================================
    Serial.println("[INIT] Mode TAG actif. Envoi des distances en continu...");
    
    while (isTagMode) 
    {
        // 1. On écoute d'abord le bus CAN pour voir si le Hub nous dit d'arrêter
        if (receiveCanMessage(vMessage)) 
        {
            if (decodeCanMessage(vMessage, vData)) 
            {
                if (vData.type == MESSAGE_HUB_ORDER && 
                    vData.id_ancre == pMyAnchorId && 
                    vData.aOrderType == HUB_ORDER_TOGGLE_MODULE_MODE) 
                {
                    Serial.println("[INIT] Ordre d'arrêt reçu ! Retour en mode ANCRE.");
                    isTagMode = false;
                    break; // On casse la boucle pour passer à l'étape finale
                }
            }
        }

        // 2. On tente de lire une distance depuis le module UWB
        uint8_t vStaticAnchorId = vData.aStaticAnchorIdDuringToggle; 
        float vDistance = readDistanceFromUWB(vStaticAnchorId, UWBSerial);

        // 3. Si on a capté une vraie distance, on l'envoie au Hub
        if (vDistance > 0.0f) 
        {
            /*
             * /!\ POINT TRÈS IMPORTANT CONCERNANT TON CODE HUB /!\
             * Dans ton fichier InitAnchorPosition.cpp du Hub, la ligne :
             * if (vData.type == MESSAGE_TAG_ID_AND_DISTANCE && vData.id_ancre == vCurrentId)
             * vérifie que l'ID CAN de la trame reçue correspond à vCurrentId (qui est l'ID du Tag dans cette boucle).
             * Par conséquent, on passe pMyAnchorId (mon propre ID) dans le premier paramètre
             * pour que le Hub l'accepte.
             */
            sendCanDistance(pMyAnchorId, vStaticAnchorId, vDistance);
            
            Serial.printf("[INIT] Distance envoyée : %.2fm vers l'Ancre %d\n", vDistance, vStaticAnchorId);
        }

        // On temporise l'envoi pour ne pas flooder le bus CAN 
        // (Le Hub a un timeout de 500ms, envoyer toutes les 20-50ms est idéal)
        delay(20); 
    }

    // =========================================================
    // PHASE 3 : Restauration de l'état
    // =========================================================
    toggleUWBMode(pMyAnchorId);
    Serial.println("[INIT] Séquence terminée pour cette ancre. Retour à la normale.");
}

void runCompleteInitialisationPhase(uint8_t pMyAnchorId)
{
    bool vHasReceivedEndMessage = false;
    twai_message_t vMessage;
    DecodedData vMessageData;

    Serial.println("[COMPLETE INIT] Démarrage de l'initialisation de la position de l'ancre (et des 3 autres)");

    while (!vHasReceivedEndMessage)
    {
        // On vérifie que le Hub ne nous a pas envoyé un message de fin de phase d'initialisation de la position des ancres
        if (receiveCanMessage(vMessage))
        {
            if (decodeCanMessage(vMessage, vMessageData))
            {
                if (vMessageData.type == MESSAGE_HUB_ORDER && 
                    vMessageData.id_ancre == pMyAnchorId && 
                    vMessageData.aOrderType == HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL) 
                {
                    Serial.println("[COMPLETE INIT] Ordre de fin d'initialisation des ancres reçu !");
                    vHasReceivedEndMessage = true;
                    break; // On casse la boucle pour passer à l'étape finale
                }
            }
        }

        // Sinon cela veut dire que l'on doit continuer l'initialisation
        Serial.println("[COMPLETE INIT] On lance une phase d'initialisation (toggleAnchorMode)...");
        runInitializationPhase(pMyAnchorId);
    }
}