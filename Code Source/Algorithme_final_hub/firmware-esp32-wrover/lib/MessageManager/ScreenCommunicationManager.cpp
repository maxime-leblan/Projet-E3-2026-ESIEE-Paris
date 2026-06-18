#include "ScreenCommunicationManager.hpp"
#include "UartMessageManager.hpp"

// Configuration matérielle issue de vos commentaires pour le Hub
#define HUB_UART_NUM 1
#define HUB_TX_PIN 33
#define HUB_RX_PIN 32
#define HUB_BAUDRATE 115200

HardwareSerial ScreenSerial(HUB_UART_NUM);

// Variables d'état pour la gestion de la calibration et des timers
bool isCalibrating = false;
unsigned long calibrationTimer = 0;
int targetTagId = -1;
unsigned long lastTagUpdateTransmission = 0;

void setupScreenCommunication() {
    // Utilisation de votre bibliothèque existante pour configurer l'UART
    initUARTReceiver(HUB_UART_NUM, HUB_RX_PIN, HUB_TX_PIN, HUB_BAUDRATE, ScreenSerial);
    Serial.println("Module ScreenCommunicationManager active.");
}

void processIncomingCommand(JsonDocument& doc) {
    String cmd = doc["cmd"].as<String>();

    // 1. Commande reçue lorsque le téléphone lance un nouveau calibrage
    if (cmd == "start_calib") {
        targetTagId = doc["tag_id"].as<int>();
        isCalibrating = true;
        calibrationTimer = millis();
        Serial.printf("[Hub Hub] Demarrage calibration cible sur le Tag ID: %d\n", targetTagId);
    } 
    // 2. Commande reçue lors d'une sauvegarde finale (Tel) ou d'un changement de machine (Ecran Matouch)
    else if (cmd == "save_config" || cmd == "change_config") {
        String machineName = doc["nom"].as<String>();
        Serial.printf("\n=============================================");
        Serial.printf("[Hub Application] Synchronisation de la machine : %s\n", machineName.c_str());

        // Extraction et chargement de la zone de sécurité (64 points 2D)
        JsonArray zone = doc["zone"].as<JsonArray>();
        int pointsCount = 0;
        for (JsonVariant v : zone) {
            float x = v["x"].as<float>();
            float y = v["y"].as<float>();
            // LOGIQUE MÉTIER HUB : Stockez ici x et y dans vos structures de calculs de distance
            pointsCount++;
        }
        Serial.printf("[Hub Logic] %d points de périmètre enregistrés.\n", pointsCount);

        // Extraction et chargement des positions des capteurs/ancres
        JsonArray sensors = doc["sensors"].as<JsonArray>();
        int sensorsCount = 0;
        for (JsonVariant s : sensors) {
            float sx = s["x"].as<float>();
            float sy = s["y"].as<float>();
            // LOGIQUE MÉTIER HUB : Enregistrez la position réelle de vos modules de détection
            sensorsCount++;
        }
        Serial.printf("[Hub Logic] %d ancres de capteurs configurées.\n", sensorsCount);
        Serial.println("=============================================\n");
    }
}

void sendTagsToScreen() {
    JsonDocument doc;
    doc["type"] = "tags";
    JsonArray data = doc["data"].to<JsonArray>();

    // EXEMPLE DE SIMULATION DE COORDONNÉES DES TAGS EN MÈTRES :
    // Remplacez ces lignes par vos vraies variables issues de l'algorithme du Hub
    JsonObject tag1 = data.add<JsonObject>();
    tag1["id"] = 101; tag1["x"] = 4.2; tag1["y"] = 1.5; tag1["alarme"] = false;

    JsonObject tag2 = data.add<JsonObject>();
    tag2["id"] = 105; tag2["x"] = -1.8; tag2["y"] = 3.2; tag2["alarme"] = true; // Déclenche le cadre rouge sur l'écran

    String output;
    serializeJson(doc, output);
    sendDataUART(ScreenSerial, output); // Utilisation de votre fonction de transmission
}

void sendCalibrationDataToScreen() {
    JsonDocument doc;
    doc["type"] = "calib_data";
    
    // Génération automatique des positions des 4 ancres/capteurs (mètres)
    JsonArray sensors = doc["sensors"].to<JsonArray>();
    float staticPositions[4][2] = {{-1.2, 2.5}, {1.2, 2.5}, {-1.2, -2.5}, {1.2, -2.5}};
    for (int i = 0; i < 4; i++) {
        JsonObject s = sensors.add<JsonObject>();
        s["x"] = staticPositions[i][0]; s["y"] = staticPositions[i][1];
    }

    // Génération automatique d'un périmètre d'exclusion initial de 64 points (mètres)
    JsonArray points = doc["points"].to<JsonArray>();
    for (int i = 0; i < 64; i++) {
        float angle = (i * 2 * PI) / 64.0;
        JsonObject p = points.add<JsonObject>();
        p["x"] = cos(angle) * 6.5; // Rayon de protection simulé de 6.5 mètres
        p["y"] = sin(angle) * 6.5;
    }

    String output;
    serializeJson(doc, output);
    sendDataUART(ScreenSerial, output);
}

void loopScreenCommunication() {
    // 1. ÉCOUTE ET LECTURE DE L'UART EN PROVENANCE DE L'ÉCRAN MATOUCH
    String receivedMessage = "";
    receiveDataUART(ScreenSerial, receivedMessage); // Utilisation de votre fonction non bloquante

    if (receivedMessage.length() > 0) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, receivedMessage);
        if (!err) {
            processIncomingCommand(doc);
        }
    }

    // 2. AUTOMATE DE CALIBRATION (Simulation du traitement matériel de calcul des coordonnées)
    if (isCalibrating) {
        // Une fois les mesures matérielles du Hub finalisées (simulé ici à 3 secondes)
        if (millis() - calibrationTimer > 3000) {
            sendCalibrationDataToScreen();
            isCalibrating = false; // Fin du processus de traitement de calibration
        }
    }

    // 3. ENVOI RÉGULIER DES TAGS EN COURS DE FONCTIONNEMENT (Fréquence : 500ms)
    if (!isCalibrating && (millis() - lastTagUpdateTransmission > 500)) {
        sendTagsToScreen();
        lastTagUpdateTransmission = millis();
    }
}

