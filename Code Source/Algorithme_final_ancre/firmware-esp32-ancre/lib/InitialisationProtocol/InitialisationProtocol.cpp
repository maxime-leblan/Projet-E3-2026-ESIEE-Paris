#include "InitialisationProtocol.hpp"
#include "OLEDManager.hpp" // INDISPENSABLE : Pour piloter la zone haute de l'écran

void runInitializationPhase(uint8_t pMyAnchorId)
{
    bool messageReceived = false;
    twai_message_t vMessage;
    DecodedData vData;
    uint8_t vStaticAnchorId = 0; 

    Serial.printf("[INIT] Ancre %d en attente du signal pour passer en Tag...\n", pMyAnchorId);
    updateCANAction("CALIBRATION", "Attente ordre Hub...");

    // PHASE 1 : A reception du signal, on passe en mode TAG sauf si on est l'ancre statique
    while (!messageReceived) {
        if (receiveCanMessage(vMessage)) {
            if (decodeCanMessage(vMessage, vData)) {
                if (vData.type == MESSAGE_HUB_ORDER && 
                    vData.id_ancre == pMyAnchorId && 
                    vData.aOrderType == HUB_ORDER_TOGGLE_MODULE_MODE) 
                {
                    vStaticAnchorId = vData.aStaticAnchorIdDuringToggle; 
                    Serial.println("[INIT] Ordre reçu ! Passage en mode TAG pour toutes les ancres sauf " + String(vStaticAnchorId) + ".");
                    updateCANAction("CALIBRATION", "Passage en TAG...");
                    if (vStaticAnchorId != pMyAnchorId) {
                        toggleUWBMode(pMyAnchorId);
                        Serial.println("[INIT] Mode TAG activé. En attente du Hub aura récupéré les distances depuis l'ancre statique...");
                        updateCANAction("CALIBRATION", "Mode TAG Actif");
                    } else {
                        Serial.println("[INIT] Je suis l'ancre statique. Je reste en mode ANCRE.");
                        updateCANAction("CALIBRATION", "Mode ANCRE Actif");
                    }
                    messageReceived = true;
                }
            }
        }
    }

    // SI l'ancre est devenue un TAG pour l'initialisation
    while (gCurrentUWBMode == TAG_MODE) {
        if (receiveCanMessage(vMessage)) {
            if (decodeCanMessage(vMessage, vData)) {
                if (vData.type == MESSAGE_HUB_ORDER && 
                    vData.id_ancre == pMyAnchorId && 
                    vData.aOrderType == HUB_ORDER_TOGGLE_MODULE_MODE) 
                {
                    Serial.println("[INIT] Ordre de toggle reçu du HUB ! Retour en mode ANCRE...");
                    toggleUWBMode(pMyAnchorId);
                    Serial.println("[INIT] Séquence terminée pour cette ancre. Retour à la normale.");
                    updateCANAction("ANCRE " + String(pMyAnchorId), "Prete (Surveillance)");
                    return; // On sort de la fonction car l'ancre est redevenue une ancre normale
                }
            }
        }
    }

    // Si l'ancre est l'ancre statique, elle reste en mode ANCRE 
    // CORRECTION : On dimensionne à 4 (pour les IDs 0, 1, 2, 3), chaque case contenant un vecteur de distances
    std::vector<std::vector<uint16_t>> allTagDistances(4);

    // Vecteur final contenant les 4 distances moyennes à envoyer au Hub (Indexé par l'ID des Tags)
    // On l'initialise avec des 0 par défaut
    std::vector<uint16_t> tagDistances(4, 0);

    bool distances_sent = false;
    while (!distances_sent)
    {
        if (receiveCanMessage(vMessage)) {
            if (decodeCanMessage(vMessage, vData)) {
                if (vData.type == MESSAGE_HUB_ORDER && 
                    vData.aStaticAnchorIdDuringToggle == pMyAnchorId && 
                    vData.aOrderType == HUB_ORDER_REQUEST_ANCHOR_DISTANCES_DURING_CALIB)
                {
                    Serial.println("[INIT] Ordre de demande de distances reçu du HUB ! Envoi des distances depuis l'ancre statique...");
                    Serial.println("[INIT] Calcul de la moyennes des distances pour chaque tag...");
                    
                    for (int tid = 0; tid < 4; tid++) {
                        // Si le tag est l'ancre elle-même, la distance vaut contractuellement 0
                        if (tid == pMyAnchorId) {
                            tagDistances[tid] = 0;
                            continue;
                        }

                        size_t nbMesures = allTagDistances[tid].size();
                        if (nbMesures > 0) {
                            Serial.printf("[INIT] Calcul de la moyenne pour le Tag %d avec %d mesures valides.\n", tid, nbMesures);
                            uint32_t somme = 0; // Utilisation d'un 32 bits pour éviter l'overflow lors de la somme
                            for (size_t i = 0; i < nbMesures; i++) {
                                somme += allTagDistances[tid][i];
                            }
                            tagDistances[tid] = (uint16_t)(somme / nbMesures);
                            Serial.printf("[INIT] Moyenne finale calculée pour Tag %d : %d cm (%d mesures)\n", tid, tagDistances[tid], nbMesures);
                        } else {
                            // Sécurité si aucune trame radio n'a été captée pour ce module précis
                            tagDistances[tid] = 0; 
                            Serial.printf("[INIT] Avertissement : Aucune mesure captée pour le Tag %d !\n", tid);
                        }
                    }

                    // affichage des distances finales pour vérification
                    Serial.println("[INIT] Distances finales moyennées pour chaque Tag :");
                    for (int tid = 0; tid < 4; tid++) {
                        Serial.printf("[INIT] Tag %d : Distance moyenne = %d cm\n", tid, tagDistances[tid]);
                    }

                    // Envoi du vecteur final nettoyé et moyenné
                    sendCanDistanceFromStaticAnchorToHub(pMyAnchorId, tagDistances);
                    Serial.println("[INIT] Distances envoyées au HUB. En attente du prochain ordre...");
                    distances_sent = true;
                    break;
                }
            }
        } else {
            Serial.println("Mesure distance..");
            // Lecture temps réel des trames AT+RANGE sur l'UART
            if (UWBSerial.available()) {
                String ligne = UWBSerial.readStringUntil('\n');
                ligne.trim();

                if (ligne.startsWith("AT+RANGE")) {
                    Serial.printf("[INIT] Trame AT+RANGE reçue : %s\n", ligne.c_str());
                    int tid;
                    int r[8], a[8]; 
                    
                    int matched = sscanf(ligne.c_str(), "AT+RANGE=tid:%d,mask:%*x,seq:%*d,range:(%d,%d,%d,%d,%d,%d,%d,%d),ancid:(%d,%d,%d,%d,%d,%d,%d,%d)", 
                                         &tid, 
                                         &r[0], &r[1], &r[2], &r[3], &r[4], &r[5], &r[6], &r[7],
                                         &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7]);
                    
                    if (matched == 17) {
                        if (tid < allTagDistances.size() && tid != pMyAnchorId) { 
                            int maDistance = -1;
                            
                            for (int i = 0; i < 8; i++) {
                                if (a[i] == pMyAnchorId) {
                                    maDistance = r[i]; 
                                    break;
                                }
                            }
                            
                            if (maDistance > 0) {
                                Serial.printf("[INIT] Distance reçue depuis le Tag %d : %d cm\n", tid, maDistance);
                                allTagDistances[tid].push_back((uint16_t)maDistance);
                            }
                        }
                    }
                }
            }
        }
    }
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