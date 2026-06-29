#include "config.h"

// Instanciation des variables globales
TagGraphique tags_ui[MAX_TAGS];
lv_obj_t * visuel_capteurs[MAX_CAPTEURS];
VehiculeConfig liste_vehicules[20];
int total_vehicules = 0;
int id_vehicule_actif = -1;

lv_obj_t * scr_radar;
lv_obj_t * scr_vehicules;
lv_obj_t * polygone_exclusion;
lv_obj_t * mosaique;
lv_obj_t * pelleteuse;
lv_obj_t * cadre_alerte_global;
lv_obj_t * label_vide;
lv_obj_t * label_titre_config;

bool alarme_danger = false;
bool flag_recharger_ui = false;

unsigned long dernier_temps = 0;
int etape_simulation = 0;
volatile int calib_state = 0;
unsigned long calib_timer = 0;

float sim_calib_points[64][2];
int sim_calib_nb_capteurs = 4;
float sim_calib_capteurs[MAX_CAPTEURS][2] = {
  {-1.0, 2.0}, {1.0, 2.0}, {-1.0, -2.0}, {1.0, -2.0}
};

String json_tags_decouverts = "[]";
