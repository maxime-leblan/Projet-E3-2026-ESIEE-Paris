#include "UWBModuleActions.hpp"

// Les objets I2C_OLED et display ont été supprimés d'ici !
HardwareSerial UWBSerial(1);

float readDistanceFromUWB(int pModuleId, Stream & pUWBSerial)
{
    pUWBSerial.println("AT+RANGE");

    String response = "";
    
    unsigned long startTime = millis();
    while (millis() - startTime < 150) { 
        if (pUWBSerial.available()) {
            response = pUWBSerial.readStringUntil('\n');
            response.trim();
            
            if (response.startsWith("AT+RANGE")) {
                break; 
            }
        }
        delay(1);
    }

    MessageAncreHub parsedMessage;
    
    if (parseUWBMessage(response, parsedMessage)) {
        if (pModuleId >= 0 && pModuleId < 4) {
            float distanceCm = parsedMessage.distances[pModuleId] * 100.0f;
            if (distanceCm > 0.0f) {
                return distanceCm;
            }
        }
    }

    Serial.printf("[UWB] Erreur : Aucune distance valide obtenue pour l'ancre %d (Trame: %s)\n", pModuleId, response.c_str());
    return -1.0f;
}

void initUWBModule(int pAnchorId)
{
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime < 4000)) {
        delay(10);
    } 
    
    Serial.println("\n=================================");
    Serial.println("ESP32-S3 : Demarrage du programme...");
    Serial.println("=================================");

    Serial.println("Etape 1 : Activation de la puissance (POWER_PIN)...");
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);

    // Etape 2 (Initialisation OLED) a été supprimée, car le main.cpp s'en charge.
    
    updateCANAction("INITIALISATION", "Config UWB...");

    Serial.println("Etape 3 : Initialisation UWB...");

    pinMode(RESET_WAKEUP_PIN, OUTPUT);
    digitalWrite(RESET_WAKEUP_PIN, LOW);
    delay(3100);
    pinMode(RESET_WAKEUP_PIN, INPUT);
    delay(1500);
    
    UWBSerial.setRxBufferSize(2048);
    UWBSerial.begin(115200, SERIAL_8N1, UWB_RX, UWB_TX);
    while(UWBSerial.available()) { UWBSerial.read(); }

    sendATCommand("AT?", Serial, UWBSerial);

    sendATCommand("AT+SETCFG=" + String(pAnchorId) + ",1,1,0", Serial, UWBSerial); 
    sendATCommand("AT+SETCAP=6,10,1", Serial, UWBSerial); 
    sendATCommand("AT+SETPAN=" + String(NETWORK_ID), Serial, UWBSerial); 
    sendATCommand("AT+SETRPT=1", Serial, UWBSerial); // REMIS A 1 !
    sendATCommand("AT+SAVE", Serial, UWBSerial);
    sendATCommand("AT+RESTART", Serial, UWBSerial);

    Serial.println("Initialisation du module UWB terminee !");

    updateCANAction("ANCRE " + String(pAnchorId), "Attente Tag...");
}

void setUWBModeTag(int pAnchorId)
{
    Serial.printf("\n[UWB] ---> Demande de passage materiel en TAG pour l'Ancre %d\n", pAnchorId);
    
    // 1. L'ORDRE DE SILENCE : On stoppe le spam UART de la puce UWB
    UWBSerial.println("AT+SETRPT=0");
    delay(150); // On laisse le temps à la puce de se taire
    while(UWBSerial.available()) { UWBSerial.read(); } // On vide tous les résidus du buffer
    
    // 2. CONFIGURATION SEREINE
    sendATCommand("AT+SETCFG=" + String(pAnchorId) + ",0,1,0", Serial, UWBSerial);
    sendATCommand("AT+SETPAN=" + String(NETWORK_ID), Serial, UWBSerial);
    sendATCommand("AT+SETRPT=1", Serial, UWBSerial); // On réactive le rapport automatique
    sendATCommand("AT+SAVE", Serial, UWBSerial);
    
    // 3. REDÉMARRAGE SIMPLE (Un seul suffit car la ligne est dégagée)
    Serial.println("Envoi -> AT+RESTART");
    UWBSerial.println("AT+RESTART");
    sendATCommand("AT+RESTART", Serial, UWBSerial);
    
    // 4. SCANNER DE BOOT
    Serial.println("[BOOT UWB] --- Ecoute du redemarrage matériel ---");
    unsigned long bootStart = millis();
    while (millis() - bootStart < 4000) {
        if (UWBSerial.available()) {
            String line = UWBSerial.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) {
                Serial.print("[BOOT UWB] ");
                Serial.println(line);
            }
        }
    }
    Serial.println("[BOOT UWB] --- Fin de l'ecoute ---");
    Serial.println("[UWB] Tag pret et reveille.");
}

void setUWBModeAnchor(int pAnchorId)
{
    Serial.printf("\n[UWB] ---> Demande de passage materiel en ANCRE pour l'Ancre %d\n", pAnchorId);
    
    // 1. L'ORDRE DE SILENCE
    UWBSerial.println("AT+SETRPT=0");
    delay(150);
    while(UWBSerial.available()) { UWBSerial.read(); }
    
    // 2. CONFIGURATION SEREINE
    sendATCommand("AT+SETCFG=" + String(pAnchorId) + ",1,1,0", Serial, UWBSerial);
    sendATCommand("AT+SETPAN=" + String(NETWORK_ID), Serial, UWBSerial);
    sendATCommand("AT+SETRPT=0", Serial, UWBSerial); // Une ancre DOIT rester muette !
    sendATCommand("AT+SAVE", Serial, UWBSerial);
    
    // 3. REDÉMARRAGE SIMPLE
    Serial.println("Envoi -> AT+RESTART");
    UWBSerial.println("AT+RESTART");
    sendATCommand("AT+RESTART", Serial, UWBSerial);
    
    // 4. SCANNER DE BOOT
    Serial.println("[BOOT UWB] --- Ecoute du redemarrage matériel ---");
    unsigned long bootStart = millis();
    while (millis() - bootStart < 4000) {
        if (UWBSerial.available()) {
            String line = UWBSerial.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) {
                Serial.print("[BOOT UWB] ");
                Serial.println(line);
            }
        }
    }
    Serial.println("[BOOT UWB] --- Fin de l'ecoute ---");
    Serial.println("[UWB] Ancre prete et reveillee.");
}