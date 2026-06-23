#include "UWBModuleActions.hpp"

// Les objets I2C_OLED et display ont été supprimés d'ici !
HardwareSerial UWBSerial(1);

int gCurrentUWBMode = ANCHOR_DEFAULT_MODE;


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

void toggleUWBMode(int pAnchorId)
{
    String ATCommand;

    Serial.println("\n[UWB] Tentative d'inversion du mode (Tag <-> Ancre)...");
    
    if (gCurrentUWBMode == ANCHOR_MODE) {
        Serial.printf("[UWB] Passage de Ancre vers mode TAG (ID: %d)\n", pAnchorId);
        gCurrentUWBMode = TAG_MODE;
        updateCANAction("MODE UWB", "Passage en TAG");

        ATCommand = "AT+SETRPT=" + String(ACTIVATE_AT_RANGE);
        sendATCommand(ATCommand, Serial, UWBSerial);
        ATCommand = "AT+SAVE";
        sendATCommand(ATCommand, Serial, UWBSerial);
    } else {
        Serial.printf("[UWB] Passage de Tag vers mode ANCRE (ID: %d)\n", pAnchorId);
        gCurrentUWBMode = ANCHOR_MODE;
        updateCANAction("MODE UWB", "Passage en ANCRE");

        ATCommand = "AT+SETRPT=" + String(ACTIVATE_AT_RANGE);
        sendATCommand(ATCommand, Serial, UWBSerial);
    }

    ATCommand = "AT+SETCFG=" + String(pAnchorId) + "," + String(gCurrentUWBMode) + "," + String(ANCHOR_RATE) + "," + String(ANCHOR_FILTER_STATUS);
    sendATCommand(ATCommand, Serial, UWBSerial);
    ATCommand = "AT+SAVE";
    sendATCommand(ATCommand, Serial, UWBSerial);
    ATCommand = "AT+RESTART";
    sendATCommand(ATCommand, Serial, UWBSerial);
    Serial.println("[UWB] Commande AT+RESTART envoyee. Attente du redemarrage...");
    updateCANAction("MODE UWB", "Redemarrage UWB...");
    
    delay(1000); 

    // Purge totale des messages de boot (élimine le spam)
    while (UWBSerial.available()) { UWBSerial.read(); }
    
    Serial.println("[UWB] Changement de mode effectif, buffer purge.\n");
    updateCANAction("MODE UWB", "Mode change effectif");
    
}