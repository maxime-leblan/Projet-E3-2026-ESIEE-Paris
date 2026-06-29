#ifndef CAPTEUR_PRESSION_H
#define CAPTEUR_PRESSION_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BMP5xx.h>

// Création de ta propre classe (ta bibliothèque)
class CapteurPression {
  private:
    Adafruit_BMP5xx bmp; // L'objet capteur d'Adafruit est caché à l'intérieur
    int cs_pin;

  public:
    // Le constructeur (pour indiquer la broche CS)
    CapteurPression(int pin_cs);
    
    // Les fonctions que tu pourras appeler dans ton main.cpp
    bool initialiser();
    float getPression();
};

#endif