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

// On indique au compilateur que cette fonction existe dans le main.cpp
extern void appliquerNouvelleConfigurationMaterielle(JsonArray zoneJson, JsonArray sensorsJson);

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

void envoyerMiseAJourTagsRuntime(int ids[], float xs[], float ys[], float distances[], bool alarmes[], int count) {
    JsonDocument doc;
    doc["type"] = "tags";
    JsonArray data = doc["data"].to<JsonArray>();

    for (int i = 0; i < count; i++) {
        JsonObject t = data.add<JsonObject>();
        t["id"] = ids[i];
        t["x"] = xs[i];
        t["y"] = ys[i];
        t["distance"] = distances[i];
        t["alarme"] = alarmes[i];
    }

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

            // Étape : L'utilisateur demande un scan
            if (cmd == "start_calib") {
                etatActuelHub = HUB_STATE_DETECTING_TAGS_FOR_INIT;
            }
            // Étape : L'utilisateur sélectionne son tag cible
            else if (cmd == "select_tag") {
                idTagSelectionne = doc["tag_id"].as<int>();
                etatActuelHub = HUB_STATE_COLLECTING_POINTS;
            }
            // Etape : L'utilisateur demande la fin de la mesure.
            else if (cmd == "stop_measure") {
                etatActuelHub = HUB_STATE_GENERATING_GEOMETRY;
            }
            // Étape : Sauvegarde ou changement de configuration
            else if (cmd == "save_config" || cmd == "change_config") {
                
                // 1. On extrait les tableaux de coordonnées du JSON
                JsonArray zone = doc["zone"].as<JsonArray>();
                JsonArray sensors = doc["sensors"].as<JsonArray>();
                
                // 2. On transmet ces tableaux au main.cpp pour qu'il mette à jour la RAM
                appliquerNouvelleConfigurationMaterielle(zone, sensors);
                
                // 3. On bascule l'état pour lancer la boucle 33ms
                etatActuelHub = HUB_STATE_RUNNING;
            } 
            // Étape : Annulation depuis le téléphone
            else if (cmd == "cancel_calib") {
                bool doitReprendreSurveillance = doc["resume_running"] | false;
               
                if (doitReprendreSurveillance) {
                    etatActuelHub = HUB_STATE_RUNNING;
                    Serial.println("[Hub] Calibration annulée. Retour au fonctionnement normal (RUNNING).");
                } else {
                    etatActuelHub = HUB_STATE_IDLE;
                    Serial.println("[Hub] Calibration annulée. Aucune configuration active, retour en IDLE.");
                }
            } 
            // Étape : Suppression totale des configurations de la carte SD
            else if (cmd == "clear_config") {
                etatActuelHub = HUB_STATE_IDLE;

                // On appelle la fonction de nettoyage du main.cpp
                extern void reinitialiserObjetsMetierHub();
                reinitialiserObjetsMetierHub();

                Serial.println("[Hub UART] Ordre de suppression de la zone d'exclusion recu. Retour en IDLE.");
            }
        }
    }
}

