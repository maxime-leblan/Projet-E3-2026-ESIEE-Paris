#ifndef CAPTEUR_PRESSION_H
#define CAPTEUR_PRESSION_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BMP5xx.h>

class CapteurPression {
  private:
    Adafruit_BMP5xx bmp; 
    int cs_pin;

  public:
    CapteurPression(int pin_cs);
    bool initialiser();
    float getPression();
};

#endif