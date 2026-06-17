#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "hardware_setup.h"
#include "file_manager.h"
#include "ui_app.h"
#include "web_server_api.h"

void setup() {
    Serial.begin(115200);

    // 1. Matériel & SD
    setup_hardware_basic();
    charger_vehicules_sd();

    // 2. Serveur Web
    WiFi.softAP("MaTouch_Radar", "12345678");
    setup_web_server();

    // 3. LVGL & Graphiques
    lv_init();
    lv_port_fs_init();
    setup_hardware_lvgl();
    lv_create_main_gui();
    construire_menu_vehicules();
}

void loop() {
    // Si une nouvelle configuration a été uploadée
    if (flag_recharger_ui) {
        charger_vehicules_sd();
        construire_menu_vehicules();
        flag_recharger_ui = false;
    }

    // ETAT GLOBAL DE CLIGNOTEMENT
    static bool etat_cligno_rouge = false;
    bool doit_clignoter_rouge = ((millis() / 250) % 2 == 0);

    if (calib_state == 1 && (millis() - calib_timer > 3000)) {
        for(int i=0; i<64; i++){
            float angle = (i * 2 * PI) / 64.0;
            sim_calib_points[i][0] = cos(angle) * 8.0;
            sim_calib_points[i][1] = sin(angle) * 8.0;
        }
        sim_calib_points[63][0] = sim_calib_points[0][0];
        sim_calib_points[63][1] = sim_calib_points[0][1];
        calib_state = 2;
    }

    // SIMULATION HUB : GÉNÉRATION DES 3 TAGS
    if (millis() - dernier_temps > 2000 && id_vehicule_actif >= 0 && calib_state == 0) {
        alarme_danger = false;
        
        struct SimTag { int id; float x; float y; float z; bool alarme; };
        SimTag hub_tags[3];
        
        if (etape_simulation == 0) {
            hub_tags[0] = {101, 10.0, 5.0, 0, false};
            hub_tags[1] = {105, -5.0, 8.0, 0, false};
            hub_tags[2] = {110, 15.0, -10.0, 0, false};
        } else if (etape_simulation == 1) {
            hub_tags[0] = {101, 3.5, 2.0, 0, true};
            hub_tags[1] = {105, -2.0, 5.0, 5, false};
            hub_tags[2] = {110, 12.0, -8.0, 4, false};
        } else {
            hub_tags[0] = {101, -12.0, -8.0, -2, false};
            hub_tags[1] = {105, -8.0, -5.0, 6, false};
            hub_tags[2] = {110, 10.0, -10.0, -1.5, false};
        }

        for(int i=0; i<MAX_TAGS; i++) tags_ui[i].utilise = false;

        for(int i=0; i<3; i++) {
            int ui_idx = i;
            tags_ui[ui_idx].utilise = true;
            tags_ui[ui_idx].id_actuel = hub_tags[i].id;
            tags_ui[ui_idx].en_alarme = hub_tags[i].alarme;
            
            lv_label_set_text_fmt(tags_ui[ui_idx].label_id, "%d", hub_tags[i].id);
            int px_x = CENTRE_X + (int)(hub_tags[i].x * PIXELS_PER_METER);
            int px_y = CENTRE_Y - (int)(hub_tags[i].y * PIXELS_PER_METER);

            String texte_hauteur = "";
            if (hub_tags[i].z > 0.05) {
                texte_hauteur = LV_SYMBOL_UP + String(" ") + String(abs(hub_tags[i].z), 1) + "m";
            } else if (hub_tags[i].z < -0.05) {
                texte_hauteur = LV_SYMBOL_DOWN + String(" ") + String(abs(hub_tags[i].z), 1) + "m";
            }

            lv_obj_align(tags_ui[ui_idx].point, LV_ALIGN_CENTER, px_x - CENTRE_X, px_y - CENTRE_Y);
            lv_obj_clear_flag(tags_ui[ui_idx].point, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(tags_ui[ui_idx].point);

            lv_label_set_text(tags_ui[ui_idx].label_z, texte_hauteur.c_str());
            lv_obj_align_to(tags_ui[ui_idx].label_z, tags_ui[ui_idx].point, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
            lv_obj_clear_flag(tags_ui[ui_idx].label_z, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(tags_ui[ui_idx].label_z);

            if(hub_tags[i].alarme) {
                alarme_danger = true;
                logWarningSD(hub_tags[i].id, hub_tags[i].x, hub_tags[i].y);
            }
        }

        for(int i=0; i<MAX_TAGS; i++) {
            if(!tags_ui[i].utilise) {
                lv_obj_add_flag(tags_ui[i].point, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(tags_ui[i].label_z, LV_OBJ_FLAG_HIDDEN);
            }
        }
        
        etape_simulation++;
        if (etape_simulation > 2) etape_simulation = 0;
        dernier_temps = millis();
    }

    for(int i=0; i<MAX_TAGS; i++) {
        if(tags_ui[i].utilise) {
            if(tags_ui[i].en_alarme) {
                if(etat_cligno_rouge != doit_clignoter_rouge) {
                   lv_obj_set_style_bg_color(tags_ui[i].point, doit_clignoter_rouge ? lv_color_hex(0xFF0000) : lv_color_hex(0x000000), 0);
                }
            } else {
                 lv_obj_set_style_bg_color(tags_ui[i].point, lv_color_hex(0x00FF00), 0);
            }
        }
    }
    etat_cligno_rouge = doit_clignoter_rouge;

    if (alarme_danger) {
        if (doit_clignoter_rouge) lv_obj_clear_flag(cadre_alerte_global, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(cadre_alerte_global, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(cadre_alerte_global, LV_OBJ_FLAG_HIDDEN);
    }

    // Gestion du temps LVGL
    lv_tick_inc(5);
    lv_timer_handler();
    delay(5);
}