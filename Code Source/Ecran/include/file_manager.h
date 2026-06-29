#pragma once
#include <Arduino.h>

String obtenirHeure();
void logWarningSD(int tag_id, float x, float y);
void charger_vehicules_sd();
void lv_port_fs_init();
