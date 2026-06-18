#pragma once

#include <Arduino.h>
#include <lvgl.h>

// =======================================================
// CONFIGURATION ÉCRAN & MÉTRIQUE
// =======================================================
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480
#define CENTRE_X (SCREEN_WIDTH / 2)
#define CENTRE_Y (SCREEN_HEIGHT / 2)
#define PIXELS_PER_METER 19.2f

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18
#define TOUCH_INT -1
#define TOUCH_RST 38
#define TFT_BL 10
#define PIN_SD_CMD 11
#define PIN_SD_CLK 12
#define PIN_SD_D0 13

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10)
#define MAX_TAGS 30
#define MAX_CAPTEURS 10

// =======================================================
// STRUCTURES DYNAMIQUES
// =======================================================
struct TagGraphique {
    lv_obj_t * point;
    lv_obj_t * label_id;
    lv_obj_t * label_z;
    int id_actuel;
    float distance_actuelle;
    bool en_alarme;
    bool utilise;
};

struct VehiculeConfig {
    String nom;
    String operateur;
    String date_creation;
    String fichier_image;
    float zone_metres[64][2];
    lv_point_t zone_pixels[64];
    int nb_points;
    float capteurs_metres[MAX_CAPTEURS][2];
    lv_point_t capteurs_pixels[MAX_CAPTEURS];
    int nb_capteurs;
};

// =======================================================
// VARIABLES GLOBALES (Partagées)
// =======================================================
extern TagGraphique tags_ui[MAX_TAGS];
extern lv_obj_t * visuel_capteurs[MAX_CAPTEURS];
extern VehiculeConfig liste_vehicules[20];
extern int total_vehicules;
extern int id_vehicule_actif;

extern lv_obj_t * scr_radar;
extern lv_obj_t * scr_vehicules;
extern lv_obj_t * polygone_exclusion;
extern lv_obj_t * mosaique;
extern lv_obj_t * pelleteuse;
extern lv_obj_t * cadre_alerte_global;
extern lv_obj_t * label_vide;
extern lv_obj_t * label_titre_config;

extern bool alarme_danger;
extern bool flag_recharger_ui;

extern unsigned long dernier_temps;
extern int etape_simulation;
extern volatile int calib_state;
extern unsigned long calib_timer;

extern float sim_calib_points[64][2];
extern int sim_calib_nb_capteurs;
extern float sim_calib_capteurs[MAX_CAPTEURS][2];