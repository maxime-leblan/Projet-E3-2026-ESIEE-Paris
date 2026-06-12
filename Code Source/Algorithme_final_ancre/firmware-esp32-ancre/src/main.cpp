#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- CONFIGURATION MATÉRIELLE MAKERFABS ---
#define I2C_SDA 39
#define I2C_SCL 38
#define POWER_PIN 42 // anciennement 43
#define UWB_RX 18 
#define UWB_TX 17 