#ifndef CAPTEUR_PRESSION_H
#define CAPTEUR_PRESSION_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BMP5xx.h>

// Version 1 : Avec remappage complet des broches du bus SPI
void initialiserBMP581(int pin_cs, SPIClass &spi, int pin_sck, int pin_miso, int pin_mosi);
// Version 2 (Overload) : Utilise les broches par défaut du bus SPI fourni
void initialiserBMP581(int pin_cs, SPIClass &spi);

float getPressionBMP581_RAW();
void infosBMP();
void tarerBMP581();
float getPressionBMP581();

#endif