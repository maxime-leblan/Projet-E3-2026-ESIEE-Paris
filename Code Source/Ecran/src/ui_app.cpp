#include "ui_app.h"
#include "config.h"
#include "hub_com.h"

static void btn_go_vehicules_cb(lv_event_t * e) { lv_scr_load(scr_vehicules); }
static void btn_go_radar_cb(lv_event_t * e) { lv_scr_load(scr_radar); }

// Dans src/ui_app.cpp

#include "hub_com.h" // <-- Crée le lien avec le module UART
#include <ArduinoJson.h>

static void btn_select_vehicule_cb(lv_event_t * e) {
    lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
    int idx = (int)(uintptr_t)lv_obj_get_user_data(btn);
    id_vehicule_actif = idx;

    // 1. MISE À JOUR DE L'AFFICHAGE LOCAL (Code existant inchangé)
    lv_line_set_points(polygone_exclusion, liste_vehicules[idx].zone_pixels, liste_vehicules[idx].nb_points);
    String chemin = "A:/" + liste_vehicules[idx].fichier_image;
    lv_img_set_src(pelleteuse, chemin.c_str());

    lv_label_set_text(label_titre_config, liste_vehicules[idx].nom.c_str());

    for(int i=0; i<MAX_CAPTEURS; i++) lv_obj_add_flag(visuel_capteurs[i], LV_OBJ_FLAG_HIDDEN);
    for(int i=0; i<liste_vehicules[idx].nb_capteurs; i++) {
        lv_obj_clear_flag(visuel_capteurs[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(visuel_capteurs[i], LV_ALIGN_CENTER,
                     liste_vehicules[idx].capteurs_pixels[i].x - CENTRE_X,
                     liste_vehicules[idx].capteurs_pixels[i].y - CENTRE_Y);
    }

    lv_obj_move_foreground(polygone_exclusion);
    for(int i=0; i<liste_vehicules[idx].nb_capteurs; i++) lv_obj_move_foreground(visuel_capteurs[i]);
    for(int i=0; i<MAX_TAGS; i++) lv_obj_move_foreground(tags_ui[i].point);


    // =======================================================
    // 2. ENVOI DE LA NOUVELLE CONFIGURATION AU HUB DE CALCULS
    // =======================================================
    JsonDocument doc;
    doc["cmd"] = "change_config";
    doc["nom"] = liste_vehicules[idx].nom;

    // On extrait et on envoie la zone d'exclusion (les 64 points en MÈTRES)
    JsonArray zoneArray = doc["zone"].to<JsonArray>();
    for (int i = 0; i < liste_vehicules[idx].nb_points; i++) {
        JsonObject pt = zoneArray.add<JsonObject>();
        pt["x"] = liste_vehicules[idx].zone_metres[i][0];
        pt["y"] = liste_vehicules[idx].zone_metres[i][1];
    }

    // On extrait et on envoie les positions des capteurs/ancres (en MÈTRES)
    JsonArray sensorsArray = doc["sensors"].to<JsonArray>();
    for (int i = 0; i < liste_vehicules[idx].nb_capteurs; i++) {
        JsonObject s = sensorsArray.add<JsonObject>();
        s["x"] = liste_vehicules[idx].capteurs_metres[i][0];
        s["y"] = liste_vehicules[idx].capteurs_metres[i][1];
    }

    // Envoi immédiat à travers l'UART au Hub WROVER
    envoyer_commande_hub(doc);


    // 3. CHARGEMENT DE L'ÉCRAN RADAR (Code existant inchangé)
    lv_scr_load(scr_radar);
}


static void initialiser_composant_tag(int index, lv_obj_t * parent) {
    tags_ui[index].point = lv_obj_create(parent);
    lv_obj_set_size(tags_ui[index].point, 20, 20);
    lv_obj_set_style_radius(tags_ui[index].point, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(tags_ui[index].point, LV_ALIGN_CENTER, 0, -500);
    lv_obj_add_flag(tags_ui[index].point, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(tags_ui[index].point, LV_OBJ_FLAG_SCROLLABLE);
    
    tags_ui[index].label_id = lv_label_create(tags_ui[index].point);
    lv_label_set_text(tags_ui[index].label_id, "");
    lv_obj_set_style_text_font(tags_ui[index].label_id, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(tags_ui[index].label_id, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(tags_ui[index].label_id);

    tags_ui[index].label_z = lv_label_create(parent);
    lv_obj_set_style_text_color(tags_ui[index].label_z, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(tags_ui[index].label_z, LV_FONT_DEFAULT, 0);
    lv_obj_add_flag(tags_ui[index].label_z, LV_OBJ_FLAG_HIDDEN);
    
    tags_ui[index].utilise = false;
    tags_ui[index].en_alarme = false;
}

void lv_create_main_gui(void) {
    scr_radar = lv_obj_create(NULL);
    lv_obj_clear_flag(scr_radar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(scr_radar, lv_color_hex(0x000000), 0);

    scr_vehicules = lv_obj_create(NULL);
    lv_obj_clear_flag(scr_vehicules, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(scr_vehicules, lv_color_hex(0x111111), 0);

    cadre_alerte_global = lv_obj_create(lv_layer_top());
    lv_obj_set_size(cadre_alerte_global, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_opa(cadre_alerte_global, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(cadre_alerte_global, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_border_width(cadre_alerte_global, 10, 0);
    lv_obj_add_flag(cadre_alerte_global, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(cadre_alerte_global, LV_OBJ_FLAG_CLICKABLE);
    
    lv_obj_t * btn_veh = lv_btn_create(scr_radar);
    lv_obj_set_size(btn_veh, 100, 60);
    lv_obj_align(btn_veh, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_event_cb(btn_veh, btn_go_vehicules_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_btn = lv_label_create(btn_veh);
    lv_label_set_text(lbl_btn, "MENU");
    lv_obj_center(lbl_btn);

    pelleteuse = lv_img_create(scr_radar);
    lv_obj_align(pelleteuse, LV_ALIGN_CENTER, 0, 0);

    polygone_exclusion = lv_line_create(scr_radar);
    lv_obj_set_style_line_width(polygone_exclusion, 5, 0);
    lv_obj_set_style_line_color(polygone_exclusion, lv_color_hex(0xFF3333), 0);

    for(int i=0; i<MAX_CAPTEURS; i++) {
        visuel_capteurs[i] = lv_obj_create(scr_radar);
        lv_obj_set_size(visuel_capteurs[i], 12, 12);
        lv_obj_set_style_bg_color(visuel_capteurs[i], lv_color_hex(0xFBC02D), 0);
        lv_obj_set_style_radius(visuel_capteurs[i], 2, 0);
        lv_obj_add_flag(visuel_capteurs[i], LV_OBJ_FLAG_HIDDEN);
    }

    label_vide = lv_label_create(scr_radar);
    lv_label_set_text(label_vide, "En attente de configuration.\nConnectez-vous au Wi-Fi.");
    lv_obj_set_style_text_color(label_vide, lv_color_hex(0x888888), 0);
    lv_obj_align(label_vide, LV_ALIGN_CENTER, 0, 0);

    label_titre_config = lv_label_create(scr_radar);
    lv_obj_align(label_titre_config, LV_ALIGN_TOP_LEFT, 15, 15);
    lv_obj_set_style_text_color(label_titre_config, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(label_titre_config, "Aucune configuration");

    for(int i=0; i<MAX_TAGS; i++) {
        initialiser_composant_tag(i, scr_radar);
    }

    lv_obj_t * btn_rad = lv_btn_create(scr_vehicules);
    lv_obj_set_size(btn_rad, 100, 60);
    lv_obj_align(btn_rad, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_event_cb(btn_rad, btn_go_radar_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_rad = lv_label_create(btn_rad);
    lv_label_set_text(lbl_rad, "RETOUR");
    lv_obj_center(lbl_rad);

    mosaique = lv_obj_create(scr_vehicules);
    lv_obj_set_size(mosaique, 700, 350);
    lv_obj_align(mosaique, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_flex_flow(mosaique, LV_FLEX_FLOW_ROW_WRAP);

    lv_scr_load(scr_radar);
}

void construire_menu_vehicules() {
    lv_obj_clean(mosaique);
    
    if(total_vehicules == 0) {
        lv_obj_clear_flag(label_vide, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(polygone_exclusion, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(pelleteuse, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(label_titre_config, LV_OBJ_FLAG_HIDDEN);
        for(int i=0; i<MAX_CAPTEURS; i++) lv_obj_add_flag(visuel_capteurs[i], LV_OBJ_FLAG_HIDDEN);
        for(int i=0; i<MAX_TAGS; i++) {
            lv_obj_add_flag(tags_ui[i].point, LV_OBJ_FLAG_HIDDEN);
            if(tags_ui[i].label_z) lv_obj_add_flag(tags_ui[i].label_z, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }
    
    lv_obj_add_flag(label_vide, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(polygone_exclusion, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(pelleteuse, LV_OBJ_FLAG_HIDDEN);

    for(int i = 0; i < total_vehicules; i++) {
        lv_obj_t * btn = lv_btn_create(mosaique);
        lv_obj_set_size(btn, 180, 150);
        lv_obj_set_user_data(btn, (void *)(uintptr_t)i);
        lv_obj_add_event_cb(btn, btn_select_vehicule_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_t * lbl = lv_label_create(btn);
        lv_label_set_text(lbl, liste_vehicules[i].nom.c_str());
        lv_obj_center(lbl);
    }

    if (id_vehicule_actif < 0 || id_vehicule_actif >= total_vehicules) id_vehicule_actif = 0;
    lv_line_set_points(polygone_exclusion, liste_vehicules[id_vehicule_actif].zone_pixels, liste_vehicules[id_vehicule_actif].nb_points);
    String chemin = "A:/" + liste_vehicules[id_vehicule_actif].fichier_image;
    lv_img_set_src(pelleteuse, chemin.c_str());

    lv_label_set_text(label_titre_config, liste_vehicules[id_vehicule_actif].nom.c_str());

    for(int i=0; i<MAX_CAPTEURS; i++) lv_obj_add_flag(visuel_capteurs[i], LV_OBJ_FLAG_HIDDEN);
    for(int i=0; i<liste_vehicules[id_vehicule_actif].nb_capteurs; i++) {
        lv_obj_clear_flag(visuel_capteurs[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(visuel_capteurs[i], LV_ALIGN_CENTER,
                     liste_vehicules[id_vehicule_actif].capteurs_pixels[i].x - CENTRE_X,
                     liste_vehicules[id_vehicule_actif].capteurs_pixels[i].y - CENTRE_Y);
    }

    lv_obj_move_foreground(polygone_exclusion);
    for(int i=0; i<liste_vehicules[id_vehicule_actif].nb_capteurs; i++) lv_obj_move_foreground(visuel_capteurs[i]);
}

