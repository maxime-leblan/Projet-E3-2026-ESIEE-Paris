#include "CapteurPression.h"

// 1. Constructeur : on enregistre la broche CS
CapteurPression::CapteurPression(int pin_cs) {
  cs_pin = pin_cs;
}

// 2. Fonction d'initialisation (remplace le code du setup)
bool CapteurPression::initialiser() {
  return bmp.begin(cs_pin, &SPI);
}

// 3. Ta fonction sur mesure
float CapteurPression::getPression() {
  // Première lecture "fantôme"
  bmp.performReading();
  
  // Pause nécessaire pour la mesure
  delay(50); 
  
  // Deuxième lecture : la vraie valeur
  if (bmp.performReading()) {
    return (float)(bmp.pressure);
  } else {
    return -1.0; 
  }
}