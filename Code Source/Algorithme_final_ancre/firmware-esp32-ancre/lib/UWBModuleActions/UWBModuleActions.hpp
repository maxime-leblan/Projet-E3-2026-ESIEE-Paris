#pragma once

#include <Arduino.h>

#include "Messages.hpp"
#include "UWBDataManager.hpp"
#include "UWBMessageManager.hpp"
#include "OLEDManager.hpp" // NOUVEAU : On inclut uniquement ton manager d'écran

#define POWER_PIN 42
#define UWB_RX 18 
#define UWB_TX 17 
#define RESET_WAKEUP_PIN 16

#define NETWORK_ID 1111
#define ANCHOR_RATE 1
#define ANCHOR_FILTER_STATUS 0
#define ANCHOR_DEFAULT_MODE 1
#define ACTIVATE_AT_RANGE 1
#define ACTIVATE_AT_RDATA 1
#define NUMBER_OF_ANCHORS 4
#define MAX_SUPPORTED_TAGS 6
#define TAG_MODE 0
#define ANCHOR_MODE 1

extern HardwareSerial UWBSerial;
// stocke le mode UWB actuel de l'ancre
extern int gCurrentUWBMode;

/**
 * Récupère la chaine des distances du module UWB de l'ancre et renvoie sa distance par rapport à l'ancre dont l'id est passé en paramètre (utilisé uniquement pendant la phase d'initialisation des positions des ancres)
 * @param pModuleId Identifiant de l'ancre par rapport à laquelle on veut récupérer la distance
 * @return La distance entre l'ancre du module UWB et l'ancre dont l'id est passé en paramètre
 */
float readDistanceFromUWB(int pModuleId, Stream & pUWBSerial);
void initUWBModule(int pAnchorId);
void toggleUWBMode(int pAnchorId);
