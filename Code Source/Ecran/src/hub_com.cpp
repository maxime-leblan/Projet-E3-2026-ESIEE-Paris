#include "hub_com.h"
#include "config.h"
#include <Arduino.h>

#define HUB_RX_PIN 19
#define HUB_TX_PIN 20 

String uart_buffer = "";

void setup_hub_com() {
    // Initialisation du Serial2 pour communiquer avec le Hub
    Serial2.begin(115200, SERIAL_8N1, HUB_RX_PIN, HUB_TX_PIN);
}

void envoyer_commande_hub(JsonDocument& doc) {
    serializeJson(doc, Serial2);
    Serial2.println(); // Le saut de ligne indique la fin du message
}

void loop_hub_com() {
    // Lecture non-bloquante de l'UART
    while (Serial2.available()) {
        char c = Serial2.read();
        if (c == '\n') {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, uart_buffer);
            if (!err) {
                String type = doc["type"].as<String>();
                
                // 1. Réception des Tags en temps réel
                if (type == "tags") {
                    JsonArray arr = doc["data"].as<JsonArray>();
                    // Désactiver tous les tags
                    for(int i=0; i<MAX_TAGS; i++) tags_ui[i].utilise = false;
                    
                    int idx = 0;
                    for (JsonVariant v : arr) {
                        if (idx >= MAX_TAGS) break;
                        tags_ui[idx].utilise = true;
                        tags_ui[idx].id_actuel = v["id"].as<int>();
                        tags_ui[idx].en_alarme = v["alarme"].as<bool>();
                        
                        // Enregistrement des positions pour LVGL (dans le main.cpp ou ui.cpp)
                        // Note : on stocke ça dans de nouvelles variables globales si besoin
                        idx++;
                    }
                } 
                // 2. Réception des données de Calibration terminées (4 ancres + 64 pts)
                else if (type == "calib_data") {
                    JsonArray pts = doc["points"].as<JsonArray>();
                    int p_idx = 0;
                    for (JsonVariant p : pts) {
                        if(p_idx >= 64) break;
                        sim_calib_points[p_idx][0] = p["x"].as<float>();
                        sim_calib_points[p_idx][1] = p["y"].as<float>();
                        p_idx++;
                    }
                    
                    JsonArray sensors = doc["sensors"].as<JsonArray>();
                    int s_idx = 0;
                    for (JsonVariant s : sensors) {
                        if(s_idx >= MAX_CAPTEURS) break;
                        sim_calib_capteurs[s_idx][0] = s["x"].as<float>();
                        sim_calib_capteurs[s_idx][1] = s["y"].as<float>();
                        s_idx++;
                    }
                    sim_calib_nb_capteurs = s_idx;
                    
                    // Déclenche la mise à jour de la page web (polling)
                    calib_state = 2; 
                }
            }
            uart_buffer = ""; // Reset du buffer
        } else {
            uart_buffer += c;
        }
    }
}
