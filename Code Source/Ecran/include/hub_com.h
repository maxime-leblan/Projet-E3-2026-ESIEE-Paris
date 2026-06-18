#pragma once
#include <ArduinoJson.h>

void setup_hub_com();
void loop_hub_com();
void envoyer_commande_hub(JsonDocument& doc);