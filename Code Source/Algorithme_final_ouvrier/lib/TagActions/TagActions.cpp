#include "TagActions.hpp"

/**
 * Initialise les périphériques propres à la XIAO (USB, Capteurs...)
 */
void initialiserXiao() {
    Serial.begin(115200);
    
    // Les cartes XIAO nRF52840 démarrent extrêmement vite.
    // On attend un court instant que le port USB du PC soit prêt (pour le débug)
    /*
    uint32_t startTime = millis();
    while (!Serial && (millis() - startTime < 2000)) {
        delay(10); 
    }
    */

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    
    Serial.println("\n=================================");
    Serial.println("[XIAO] Initialisation matérielle...");
    
    // C'est ici que vous initialiserez votre capteur de pression SPI plus tard
    // ex: monCapteur.initialiser();
    
    Serial.println("[XIAO] Prête et configurée.");
}

/**
 * Initialise la liaison et les broches de contrôle du module UWB
 */
void initialiserUWB() {
    Serial.println("[UWB] Initialisation du module...");

    // 2. Configuration de la broche physique d'interruption (UART2 RX de l'UWB)
    // On la configure en SORTIE et on la met à l'état HAUT (HIGH) immédiatement.
    // Le réveil se déclenche sur un état BAS (LOW), donc la laisser à HAUT évite un réveil accidentel.
    //REDÉMARRAGE MATÉRIEL PROPRE (Hard Reset)
    // On force la STM32 à s'éteindre puis à se rallumer
    pinMode(UWB_WAKEUP_PIN, OUTPUT);
    digitalWrite(UWB_WAKEUP_PIN, LOW);
    delay(3100);
    digitalWrite(UWB_WAKEUP_PIN, HIGH);
    delay(1500); // On attend 1.5s que le module UWB boot complètement

    // 1. Initialisation de la communication Série (TX/RX) avec l'UWB
    Serial1.begin(115200);

    sendATCommand("AT+RESTORE", Serial, Serial1);
    delay(500); // L'effacement de la mémoire prend du temps !

    // ID:1, Role:Tag(0), Rate:6.8M(1), Filter:OFF(0)
    sendATCommand("AT+SETCFG=" + String(MY_TAG_ID) + "," + TAG_MODE_CONFIG, Serial, Serial1);
    sendATCommand("AT+SETRPT=" + String(TAG_AUTO_REPORT), Serial, Serial1);
    sendATCommand("AT+SETCAP=" + String(TAG_SETCAP_CONFIG), Serial, Serial1);

    // Permet de définir une adresse réseau 
    sendATCommand("AT+SETPAN=" + String(TAG_NETWORK_ID), Serial, Serial1);

    sendATCommand("AT+SAVE", Serial, Serial1);
    delay(500); // L'écriture en mémoire flash prend du temps !
    
    sendATCommand("AT+RESTART", Serial, Serial1);
    delay(1500); // CRUCIAL : On attend que le module ait fini de redémarrer AVANT de rendre la main au main.cpp
    
    Serial.println("[UWB] Broches de contrôle et liaison Série configurées.");
}

void safeZoneCalibration()
{
    bool isCalibrationFinished = false;
    String vReceivedMessage;
    UWBMessage vReceivedMessageData;

    uint32_t lastRangeTime = 0;
    const uint32_t rangeInterval = 100; // On demande un Ranging toutes les 100 ms

    Serial.println("\n[CALIBRATION] Lancement de la calibration dynamique...");

    while (!isCalibrationFinished)
    {
        // 1. Émettre le AT+RANGE périodiquement
        if (millis() - lastRangeTime >= rangeInterval) 
        {
            lastRangeTime = millis();
            // Utilisation de la nouvelle fonction optimisée (sans timeout en paramètre)
            sendATCommand("AT+RANGE", Serial, Serial1);
        }

        // 2. Écouter le Hub (Prend 0 milliseconde si aucun message n'est dispo)
        if (receiveUWBMessage(Serial1, vReceivedMessage, Serial))
        {
            if (decodeUWBMessage(vReceivedMessage, vReceivedMessageData))
            {
                if (vReceivedMessageData.orderType == HUB_ORDER_END_TAG_CALIBRATION)
                {
                    isCalibrationFinished = true;
                    Serial.println("[CALIBRATION] Arrêt demandé par le Hub. Fin de la boucle.");
                }
            }
            else
            {
                Serial.println("[CALIBRATION] Impossible de décoder le message reçu");
            }
        }
        
        // Permet au CPU de la XIAO de basculer sur d'autres tâches en tâche de fond si nécessaire
        delay(1); 
    }
}