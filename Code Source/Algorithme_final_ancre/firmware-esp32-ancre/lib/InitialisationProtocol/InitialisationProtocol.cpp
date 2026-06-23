#include "InitialisationProtocol.hpp"
#include "OLEDManager.hpp" // INDISPENSABLE : Pour piloter la zone haute de l'écran

void runInitializationPhase(uint8_t pMyAnchorId)
{
    bool isTagMode = false;
    twai_message_t vMessage;
    DecodedData vData;
    uint8_t vStaticAnchorId = 0; // Mémorisation de l'ancre fixe cible

    Serial.printf("[INIT] Ancre %d en attente du signal pour passer en Tag...\n", pMyAnchorId);
    updateCANAction("CALIBRATION", "Attente ordre Hub...");

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
                    updateCANAction("CALIBRATION", "Passage en TAG...");
                    
                    // On extrait et on verrouille immédiatement l'ID de l'ancre fixe
                    vStaticAnchorId = vData.aStaticAnchorIdDuringToggle; 
                    isTagMode = true;
                }
            }
        }
        delay(5); // Petite pause pour ne pas surcharger le processeur (Anti-Watchdog)
    }

    // On exécute la fonction pour changer le rôle du module UWB via AT Commands
    toggleUWBMode(pMyAnchorId);

    // =========================================================
    // PHASE 2 : Envoi des distances en boucle jusqu'au stop
    // =========================================================
    Serial.println("[INIT] Mode TAG actif. Envoi des distances en continu...");
    updateCANAction("CALIBRATION", "Mesure vers A" + String(vStaticAnchorId));
    
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
                    updateCANAction("CALIBRATION", "Passage en ANCRE...");
                    isTagMode = false;
                    break; // On casse la boucle pour passer à l'étape finale
                }
            }
        }

        // 2. On tente de lire une distance depuis le module UWB vers l'ancre fixe verrouillée
        float vDistance = readDistanceFromUWB(vStaticAnchorId, UWBSerial);

        // 3. Si on a capté une vraie distance, on l'envoie au Hub
        if (vDistance > 0.0f) 
        {
            sendCanDistance(pMyAnchorId, vStaticAnchorId, vDistance);
            Serial.printf("[INIT] Distance envoyée : %.2fm vers l'Ancre %d\n", vDistance, vStaticAnchorId);
            
            // Rafraîchissement de l'OLED en direct avec la mesure physique
            updateCANAction("MESURE EN COURS", "A" + String(vStaticAnchorId) + " : " + String((int)vDistance) + " cm");
        }

        // On temporise l'envoi pour ne pas flooder le bus CAN 
        delay(20); 
    }

    // =========================================================
    // PHASE 3 : Restauration de l'état
    // =========================================================
    toggleUWBMode(pMyAnchorId);
    Serial.println("[INIT] Séquence terminée pour cette ancre. Retour à la normale.");
    updateCANAction("ANCRE " + String(pMyAnchorId), "Prete (Surveillance)");
}

void runCompleteInitialisationPhase(uint8_t pMyAnchorId)
{
    bool vHasReceivedEndMessage = false;
    twai_message_t vMessage;
    DecodedData vMessageData;

    Serial.println("[COMPLETE INIT] Démarrage de l'initialisation de la position de l'ancre");
    updateCANAction("INIT POSITION", "Debut de phase");

    while (!vHasReceivedEndMessage)
    {
        // On vérifie que le Hub ne nous a pas envoyé un message de fin de phase d'initialisation
        if (receiveCanMessage(vMessage))
        {
            if (decodeCanMessage(vMessage, vMessageData))
            {
                if (vMessageData.type == MESSAGE_HUB_ORDER && 
                    vMessageData.id_ancre == pMyAnchorId && 
                    vMessageData.aOrderType == HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL) 
                {
                    Serial.println("[COMPLETE INIT] Ordre de fin d'initialisation des ancres reçu !");
                    updateCANAction("INIT POSITION", "Phase terminee OK");
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