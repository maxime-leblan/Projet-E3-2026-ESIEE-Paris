#include "hub_com.h"
#include "config.h"
#include "file_manager.h" // Pour logWarningSD
#include <Arduino.h>
#include <lvgl.h>

#define ECRAN_RX_PIN 19
#define ECRAN_TX_PIN 20

String uart_buffer = "";
// On pré-réserve de la mémoire pour éviter que le buffer crash sur les longs messages
const size_t MAX_BUFFER_SIZE = 4096;

extern void clear_radar_display();

unsigned long last_hub_msg_time = 0;

void setup_hub_com() {
    uart_buffer.reserve(MAX_BUFFER_SIZE);
    Serial2.setRxBufferSize(MAX_BUFFER_SIZE);
    Serial2.begin(115200, SERIAL_8N1, ECRAN_RX_PIN, ECRAN_TX_PIN);
}

void envoyer_commande_hub(JsonDocument& doc) {
    serializeJson(doc, Serial2);
    Serial2.println();
}

void loop_hub_com() {
    // 1. LECTURE DE L'UART EN PROVENANCE DU HUB
    while (Serial2.available()) {
        char c = Serial2.read();
        if (c == '\n') {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, uart_buffer);
            if (!err) {
                // FIX TIMEOUT : On valide la présence du Hub uniquement si le JSON est valide
                last_hub_msg_time = millis(); 

                String type = doc["type"].as<String>();

                if (type == "discovered_tags") {
                    JsonArray arr = doc["data"].as<JsonArray>();
                    String tmp;
                    serializeJson(arr, tmp);

                    json_tags_decouverts = tmp;

                    calib_state = 1;
                }
               
                // ==========================================
                // 1. AFFICHAGE DES TAGS EN TEMPS RÉEL
                // ==========================================
                else if (type == "tags") {
                    JsonArray arr = doc["data"].as<JsonArray>();
                   
                    // Masquage temporaire avant redessinage des nouveaux points reçus
                    for(int i=0; i<MAX_TAGS; i++) {
                        tags_ui[i].utilise = false;
                        lv_obj_add_flag(tags_ui[i].point, LV_OBJ_FLAG_HIDDEN);
                        if (tags_ui[i].label_z) lv_obj_add_flag(tags_ui[i].label_z, LV_OBJ_FLAG_HIDDEN);
                    }
                    alarme_danger = false;
                   
                    int idx = 0;
                    for (JsonVariant v : arr) {
                        if (idx >= MAX_TAGS) break;
                       
                        float x = v["x"].as<float>();
                        float y = v["y"].as<float>();
                        float distance = v["distance"] | 0.0f;
                       
                        tags_ui[idx].utilise = true;
                        tags_ui[idx].id_actuel = v["id"].as<int>();
                        tags_ui[idx].en_alarme = v["alarme"].as<bool>();
                       
                        int px_x = CENTRE_X + (int)(x * PIXELS_PER_METER);
                        int px_y = CENTRE_Y - (int)(y * PIXELS_PER_METER);

                        lv_label_set_text_fmt(tags_ui[idx].label_id, "%d", tags_ui[idx].id_actuel);
                        lv_obj_align(tags_ui[idx].point, LV_ALIGN_CENTER, px_x - CENTRE_X, px_y - CENTRE_Y);
                        lv_obj_clear_flag(tags_ui[idx].point, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_move_foreground(tags_ui[idx].point);

                        // --- FIX AFFICHAGE DISTANCE SÉCURISÉ ---
                        String texte_distance = String(distance, 1) + "m";
                        lv_label_set_text(tags_ui[idx].label_z, texte_distance.c_str());
                       
                        lv_obj_align_to(tags_ui[idx].label_z, tags_ui[idx].point, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
                        lv_obj_clear_flag(tags_ui[idx].label_z, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_move_foreground(tags_ui[idx].label_z);

                        if (tags_ui[idx].en_alarme) {
                            alarme_danger = true;
                            logWarningSD(tags_ui[idx].id_actuel, x, y);
                        }
                        idx++;
                    }
                }
                // ==========================================
                // 2. RÉCEPTION DE LA ZONE DE CALIBRATION
                // ==========================================
                else if (type == "calib_geometry") { // <-- FIX : Remplacement de "calib_data" par "calib_geometry"
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
                    calib_state = 2; // Débloque l'application sur le téléphone !
                }
            }
            uart_buffer = ""; // Vider le buffer
        } else {
            // Anti-débordement manuel de sécurité
            if(uart_buffer.length() < MAX_BUFFER_SIZE - 10) {
                uart_buffer += c;
            } else {
                uart_buffer = ""; // Si ça déborde, on jette la trame corrompue
            }
        }
    }

    // --- FIX CRUCIAL TIMEOUT (SORTI DU WHILE SEIRAL AVAILABLE) ---
    // Ce bloc s'exécute à chaque tick de la loop, même s'il n'y a aucun fil branché.
    if (last_hub_msg_time > 0 && (millis() - last_hub_msg_time > 5000)) {
        Serial.println("[IHM Ecran] Perte de connexion HUB (>5s). Nettoyage de l'écran");
        clear_radar_display();
        last_hub_msg_time = 0; // Bloque les déclenchements en boucle
    }
}

// --- FONCTION DE NETTOYAGE ---
void clear_radar_display() {
    // 1. On cache et désactive absolument tous les tags à l'écran
    for(int i = 0; i < MAX_TAGS; i++) {
        tags_ui[i].utilise = false;
        tags_ui[i].en_alarme = false;
        if (tags_ui[i].point) lv_obj_add_flag(tags_ui[i].point, LV_OBJ_FLAG_HIDDEN);
        if (tags_ui[i].label_z) lv_obj_add_flag(tags_ui[i].label_z, LV_OBJ_FLAG_HIDDEN);
    }
    // 2. On éteint l'état d'alarme global
    alarme_danger = false;
    // 3. On masque physiquement le grand cadre rouge clignotant
    if (cadre_alerte_global) {
        lv_obj_add_flag(cadre_alerte_global, LV_OBJ_FLAG_HIDDEN);
    }
    Serial.println("[IHM Écran] Écran Radar entièrement nettoyé (Clear). Alertes réinitialisées.");
}