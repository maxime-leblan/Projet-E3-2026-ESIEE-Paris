#include "web_server_api.h"
#include "config.h"
#include "file_manager.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SD_MMC.h>
#include <ArduinoJson.h>
#include "hub_com.h"

AsyncWebServer server(80);

extern void clear_radar_display();
extern String json_tags_decouverts;

void setup_web_server() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if (SD_MMC.exists("/index.html")) request->send(SD_MMC, "/index.html", "text/html");
        else request->send(200, "text/plain", "Fichier index.html introuvable !");
    });

    server.on("/api/time", HTTP_POST, [](AsyncWebServerRequest *request) {
        if(request->hasParam("ts", true)){
            long ts = request->getParam("ts", true)->value().toInt();
            struct timeval tv; tv.tv_sec = ts; tv.tv_usec = 0;
            settimeofday(&tv, NULL);
            setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
            tzset();
        }
        request->send(200, "text/plain", "Heure synchronisée");
    });

    server.on("/api/warnings", HTTP_GET, [](AsyncWebServerRequest *request){
        if (SD_MMC.exists("/warnings.csv")) request->send(SD_MMC, "/warnings.csv", "text/csv");
        else request->send(200, "text/csv", "Date,TagID,X,Y\n");
    });
    
    server.on("/api/calibrations", HTTP_GET, [](AsyncWebServerRequest *request){
        if (SD_MMC.exists("/calibrations.json")) request->send(SD_MMC, "/calibrations.json", "application/json");
        else request->send(200, "application/json", "[]");
    });

    server.on("/api/calibrations", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"ok\"}");
}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    static File file;
    if (!index) file = SD_MMC.open("/calibrations.json", "w");
    if (file) file.write(data, len);
    
    if (index + len == total) {
        if (file) file.close();
        flag_recharger_ui = true; // Demande à l'écran de rafraîchir son menu

        // ENVOI DE LA NOUVELLE CONFIGURATION AU HUB (WROVER)
        // On réouvre le fichier qu'on vient d'écrire pour extraire la config
        File readFile = SD_MMC.open("/calibrations.json", "r");
        if (readFile) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, readFile);
            readFile.close();
            
            if (!error && doc.is<JsonArray>()) {
                JsonArray array = doc.as<JsonArray>();
                if (array.size() > 0) {
                    // Le téléphone envoie tout le tableau, la nouvelle config est donc la dernière
                    JsonObject derniereConfig = array[array.size() - 1].as<JsonObject>();
                    
                    // On prépare le message JSON pour le Hub
                    JsonDocument hub_doc;
                    hub_doc["cmd"] = "save_config";
                    hub_doc["nom"] = derniereConfig["nom"];
                    hub_doc["zone"] = derniereConfig["zone"];       // Les 64 points
                    hub_doc["sensors"] = derniereConfig["sensors"]; // Les 4 ancres
                    
                    // Envoi physique immédiat via UART (Serial2) au Hub WROVER
                    envoyer_commande_hub(hub_doc);
                }
            }
        }
    }
});

    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Upload OK");
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        static File imgFile;
        if (!index) imgFile = SD_MMC.open("/" + filename, "w");
        if (imgFile) imgFile.write(data, len);
        if (final && imgFile) imgFile.close();
    });

    server.on("/api/hub/tags", HTTP_GET, [](AsyncWebServerRequest *request){
        clear_radar_display();

        // 1. On réveille le Hub et on lui dit de scanner
        JsonDocument order;
        order["cmd"] = "start_calib"; 
        envoyer_commande_hub(order);
        
        calib_state = 0;
        json_tags_decouverts = "[]";

        // 2. On attend que le Hub réponde avec sa liste (Max 3 secondes)
        unsigned long start_wait = millis();
        while (calib_state == 0 && (millis() - start_wait < 3000)) {
            loop_hub_com(); // VITAL : Permet d'écouter la réponse de l'UART pendant l'attente
            delay(10);
        }

        // 3. On renvoie la liste trouvée au téléphone
        request->send(200, "application/json", json_tags_decouverts);
    });

    server.on("/api/hub/start", HTTP_POST, [](AsyncWebServerRequest *request){
        int tag_id = 0;

        if (request->hasParam("tag_id", true)){ // param dans le body ?
            tag_id = request->getParam("tag_id", true)->value().toInt();
        } else if (request->hasParam("tag_id")) { // Sécurité si ca trouve pas dans le body alors dans l'URL
            tag_id = request->getParam("tag_id")->value().toInt(); 
        }
        
        // On envoie l'ID choisi au Hub pour qu'il génère la forme de la machine (64 points)
        JsonDocument doc;
        doc["cmd"] = "select_tag"; // <-- CHANGEMENT ICI : Ce n'est plus start_calib
        doc["tag_id"] = tag_id;
        envoyer_commande_hub(doc);

        calib_state = 1; // On dit à l'écran d'attendre la géométrie
        request->send(200, "text/plain", "OK");
    });

    server.on("/api/hub/cancel", HTTP_POST, [](AsyncWebServerRequest *request){
        clear_radar_display();

        JsonDocument doc;
        doc["cmd"] = "cancel_calib";
        // Si id_vehicule_actif >= 0, cela signifie qu'un véhicule tournait avant qu'on lance la calibration
        doc["resume_running"] = (id_vehicule_actif >= 0); 
        envoyer_commande_hub(doc);

        calib_state = 0; 
        request->send(200, "text/plain", "Annulé");
    });


    server.on("/api/config_log", HTTP_GET, [](AsyncWebServerRequest *request){
        if (SD_MMC.exists("/config_log.csv")) request->send(SD_MMC, "/config_log.csv", "text/csv");
        else request->send(200, "text/csv", "Date,Action,Machine,Operateur\n");
    });

    server.on("/api/config_action", HTTP_POST, [](AsyncWebServerRequest *request){
        String act = request->hasParam("action", true) ? request->getParam("action", true)->value() : "Action";
        String nom = request->hasParam("nom", true) ? request->getParam("nom", true)->value() : "Inconnu";
        String op = request->hasParam("op", true) ? request->getParam("op", true)->value() : "Inconnu";
        
        File f = SD_MMC.open("/config_log.csv", FILE_APPEND);
        if (f) {
            if (f.size() == 0) f.println("Date,Action,Machine,Operateur");
            f.printf("%s,%s,%s,%s\n", obtenirHeure().c_str(), act.c_str(), nom.c_str(), op.c_str());
            f.close();
        }

        if(act == "Suppression" && request->hasParam("image", true)){
            String img = request->getParam("image", true)->value();
            if(img.length() > 0 && img != "default.bin") {
                String path = "/" + img;
                if(SD_MMC.exists(path)) SD_MMC.remove(path);
            }
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/api/hub/status", HTTP_GET, [](AsyncWebServerRequest *request){
        if (calib_state == 0) { request->send(200, "application/json", "{\"status\":\"idle\"}"); }
        else if (calib_state == 1) { request->send(200, "application/json", "{\"status\":\"wait\"}"); }
        else if (calib_state == 2) {
            JsonDocument doc;
            doc["status"] = "done";
            
            JsonArray pts = doc["points"].to<JsonArray>();
            for(int i=0; i<64; i++){
                JsonObject p = pts.add<JsonObject>();
                p["x"] = sim_calib_points[i][0];
                p["y"] = sim_calib_points[i][1];
            }

            JsonArray caps = doc["sensors"].to<JsonArray>();
            for(int i=0; i<sim_calib_nb_capteurs; i++){
                JsonObject c = caps.add<JsonObject>();
                c["x"] = sim_calib_capteurs[i][0];
                c["y"] = sim_calib_capteurs[i][1];
            }

            String res;
            serializeJson(doc, res);
            request->send(200, "application/json", res);
            calib_state = 0;
        }
    });

    server.begin();
}