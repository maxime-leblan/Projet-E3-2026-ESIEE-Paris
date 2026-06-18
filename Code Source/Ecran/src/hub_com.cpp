#include "hub_com.h"
#include "config.h"
#include "file_manager.h" // Pour logWarningSD
#include <Arduino.h>
#include <lvgl.h>

#define HUB_RX_PIN 19
#define HUB_TX_PIN 20

String uart_buffer = "";
// On pré-réserve de la mémoire pour éviter que le buffer crash sur les longs messages
const size_t MAX_BUFFER_SIZE = 4096;

void setup_hub_com() {
    uart_buffer.reserve(MAX_BUFFER_SIZE);
    Serial2.setRxBufferSize(MAX_BUFFER_SIZE);
    Serial2.begin(115200, SERIAL_8N1, HUB_RX_PIN, HUB_TX_PIN);
}

void envoyer_commande_hub(JsonDocument& doc) {
    serializeJson(doc, Serial2);
    Serial2.println();
}

void loop_hub_com() {
    while (Serial2.available()) {
        char c = Serial2.read();
        if (c == '\n') {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, uart_buffer);
            if (!err) {
                String type = doc["type"].as<String>();
               
                // ==========================================
                // 1. AFFICHAGE DES TAGS EN TEMPS RÉEL
                // ==========================================
                if (type == "tags") {
                    JsonArray arr = doc["data"].as<JsonArray>();
                   
                    // On cache tout d'abord
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
                        
                        // --- 1. LECTURE DE LA DISTANCE DEPUIS LE JSON ---
                        float distance = v["distance"] | 0.0f;
                        tags_ui[idx].distance_actuelle = distance;
                       
                        tags_ui[idx].utilise = true;
                        tags_ui[idx].id_actuel = v["id"].as<int>();
                        tags_ui[idx].en_alarme = v["alarme"].as<bool>();
                       
                        // Calcul mathématique de position en pixels
                        int px_x = CENTRE_X + (int)(x * PIXELS_PER_METER);
                        int px_y = CENTRE_Y - (int)(y * PIXELS_PER_METER);

                        // Mise à jour du point graphique principal (Badge ID)
                        lv_label_set_text_fmt(tags_ui[idx].label_id, "%d", tags_ui[idx].id_actuel);
                        lv_obj_align(tags_ui[idx].point, LV_ALIGN_CENTER, px_x - CENTRE_X, px_y - CENTRE_Y);
                        lv_obj_clear_flag(tags_ui[idx].point, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_move_foreground(tags_ui[idx].point);

                        // --- 2. FORMATAGE ET AFFICHAGE DE LA DISTANCE (Écrase le mot "Text") ---
                        char buffer_distance[16];
                        snprintf(buffer_distance, sizeof(buffer_distance), "%.1fm", distance); // Formatage ex: "3.2m"
                        lv_label_set_text(tags_ui[idx].label_z, buffer_distance); // On remplace le texte !
                        
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
                    calib_state = 2;
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
}