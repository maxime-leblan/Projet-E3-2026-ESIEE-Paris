#pragma once

#include <Arduino.h>
#include "Messages.hpp"
#include "UWBDataManager.hpp"
#include "UWBMessageManager.hpp"
#include "OLEDManager.hpp"

#define POWER_PIN 42
#define UWB_RX 18
#define UWB_TX 17
#define RESET_WAKEUP_PIN 16
#define NETWORK_ID 1111

extern HardwareSerial UWBSerial;

float readDistanceFromUWB(int pModuleId, Stream & pUWBSerial);
void initUWBModule(int pAnchorId);

// NOUVELLES FONCTIONS DE FORÇAGE D'ÉTAT
void setUWBModeTag(int pAnchorId);
void setUWBModeAnchor(int pAnchorId);

