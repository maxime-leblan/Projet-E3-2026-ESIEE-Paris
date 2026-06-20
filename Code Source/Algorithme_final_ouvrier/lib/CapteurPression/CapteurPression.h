#ifndef CAPTEUR_PRESSION_H
#define CAPTEUR_PRESSION_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BMP5xx.h>

#define BMP_CS D3

void initialiserBMP581();
float lirePressionBMP581();
void infosBMP();

#endif