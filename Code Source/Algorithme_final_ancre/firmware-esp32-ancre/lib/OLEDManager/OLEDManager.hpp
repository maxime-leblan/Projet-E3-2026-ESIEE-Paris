#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define OLED_SDA_PIN 39
#define OLED_SCL_PIN 38

// Structure pour mémoriser l'état actuel de l'écran
struct ScreenState {
    int anchorId; // <-- Ajout de l'ID de la carte
    String canActionTitle;
    String canActionDetails;
    int currentTagId;
    uint16_t distances[4];
};

void initOLED(int pAnchorId);
void updateCANAction(String title, String details);
void updateUWBData(int tagId, uint16_t d0, uint16_t d1, uint16_t d2, uint16_t d3);
void refreshScreen();