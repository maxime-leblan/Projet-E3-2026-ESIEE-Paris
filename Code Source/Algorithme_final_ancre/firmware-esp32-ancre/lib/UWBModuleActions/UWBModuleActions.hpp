#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "UWBDataManager.hpp"
#include "UWBMessageManager.hpp"

// --- CONFIGURATION MATÉRIELLE MAKERFABS ---
#define I2C_SDA 39
#define I2C_SCL 38
#define POWER_PIN 42 // anciennement 43
#define UWB_RX 18 
#define UWB_TX 17 

TwoWire I2C_OLED = TwoWire(1);
Adafruit_SSD1306 display(128, 64, &I2C_OLED, -1);
HardwareSerial UWBSerial(1);

/**
 * Récupère la chaine des distances du module UWB de l'ancre et renvoie sa distance par rapport à l'ancre dont l'id est passé en paramètre
 * @param pModuleId Identifiant de l'ancre par rapport à laquelle on veut récupérer la distance
 * @return La distance entre l'ancre du module UWB et l'ancre dont l'id est passé en paramètre
 */
float readDistanceFromUWB(int pModuleId, Stream & pUWBSerial);

void initUWBModule(int pAnchorId);

void toggleUWBMode(int pAnchorId);

void updateScreen(String role, String value);