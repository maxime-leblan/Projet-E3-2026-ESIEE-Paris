#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "hardware_setup.h"
#include "file_manager.h"
#include "ui_app.h"
#include "web_server_api.h"
#include "hub_com.h"

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

    //Setup de la communication.
    setup_hub_com();
}

void loop() {
    // loop communications avec hub :
    loop_hub_com();

    // Si une nouvelle configuration a été uploadée
    if (flag_recharger_ui) {
        charger_vehicules_sd();
        construire_menu_vehicules();
        flag_recharger_ui = false;
    }

    // ETAT GLOBAL DE CLIGNOTEMENT
    static bool etat_cligno_rouge = false;
    bool doit_clignoter_rouge = ((millis() / 250) % 2 == 0);

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