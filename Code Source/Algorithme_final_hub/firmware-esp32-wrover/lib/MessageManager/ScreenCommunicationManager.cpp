#include "ScreenCommunicationManager.hpp"
#include "UartMessageManager.hpp"

#define HUB_UART_NUM 1
#define HUB_TX_PIN 33
#define HUB_RX_PIN 32
#define HUB_BAUDRATE 115200

HardwareSerial ScreenSerial(HUB_UART_NUM);

// Instanciation des variables partagées avec le main.cpp
volatile HubState etatActuelHub = HUB_STATE_IDLE;
volatile int idTagSelectionne = -1;

void setupScreenCommunication() {
    initUARTReceiver(HUB_UART_NUM, HUB_RX_PIN, HUB_TX_PIN, HUB_BAUDRATE, ScreenSerial);
}

void envoyerListeTagsDecouverts(int ids[], float distances[], int count) {
    JsonDocument doc;
    doc["type"] = "discovered_tags";
    JsonArray data = doc["data"].to<JsonArray>();

    for (int i = 0; i < count; i++) {
        JsonObject t = data.add<JsonObject>();
        t["id"] = ids[i];
        t["dist"] = distances[i];
    }

    serializeJson(doc, ScreenSerial);
    ScreenSerial.println();
}

void envoyerGeometrieCalibration(float ancres[4][2], float points[64][2]) {
    JsonDocument doc;
    doc["type"] = "calib_geometry";

    JsonArray sensors = doc["sensors"].to<JsonArray>();
    for (int i = 0; i < 4; i++) {
        JsonObject s = sensors.add<JsonObject>();
        s["x"] = ancres[i][0];
        s["y"] = ancres[i][1];
    }

    JsonArray pts = doc["points"].to<JsonArray>();
    for (int i = 0; i < 64; i++) {
        JsonObject p = pts.add<JsonObject>();
        p["x"] = points[i][0];
        p["y"] = points[i][1];
    }

    serializeJson(doc, ScreenSerial);
    ScreenSerial.println();
}

void envoyerDonneesTagRuntime(int id, float x, float y, float distance, bool alarme) {
    JsonDocument doc;
    doc["type"] = "tags";
    JsonArray data = doc["data"].to<JsonArray>();

    JsonObject t = data.add<JsonObject>();
    t["id"] = id;
    t["x"] = x;
    t["y"] = y;
    t["distance"] = distance;
    t["alarme"] = alarme;

    serializeJson(doc, ScreenSerial);
    ScreenSerial.println();
}

void loopScreenCommunication() {
    String receivedMessage = "";
    receiveDataUART(ScreenSerial, receivedMessage);

    if (receivedMessage.length() > 0) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, receivedMessage);
        if (!err) {
            String cmd = doc["cmd"].as<String>();

            // Étape : L'utilisateur demande un scan (index.html clique sur "Suivant")
            if (cmd == "start_calib") {
                etatActuelHub = HUB_STATE_DETECTING_TAGS_FOR_INIT;
            } 
            // Étape : L'utilisateur sélectionne son tag cible
            else if (cmd == "select_tag") {
                idTagSelectionne = doc["tag_id"].as<int>();
                etatActuelHub = HUB_STATE_GENERATING_GEOMETRY;
            } 
            // Étape : Sauvegarde ou changement de configuration (Sort de IDLE ou de la Calib)
            else if (cmd == "save_config" || cmd == "change_config") {
                // LOGIQUE MÉTIER HUB : Tu pourras parser doc["zone"] et doc["sensors"] ici si besoin
                etatActuelHub = HUB_STATE_RUNNING;
            }
        }
    }
}