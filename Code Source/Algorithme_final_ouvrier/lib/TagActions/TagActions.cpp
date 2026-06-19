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
    
    // 1. Initialisation de la communication Série (TX/RX) avec l'UWB
    Serial1.begin(115200);
    
    // 2. Configuration de la broche physique d'interruption (UART2 RX de l'UWB)
    // On la configure en SORTIE et on la met à l'état HAUT (HIGH) immédiatement.
    // Le réveil se déclenche sur un état BAS (LOW), donc la laisser à HAUT évite un réveil accidentel.
    pinMode(UWB_WAKEUP_PIN, OUTPUT);
    digitalWrite(UWB_WAKEUP_PIN, HIGH);
    
    // 3. Optionnel : On peut envoyer une commande de configuration initiale si nécessaire
    // ex: configureUWBForMessaging(Serial1);
    
    Serial.println("[UWB] Broches de contrôle et liaison Série configurées.");
}

