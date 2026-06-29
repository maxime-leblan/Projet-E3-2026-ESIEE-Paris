#include "CapteurPression.h"

// 1. Constructeur : on enregistre la broche CS
CapteurPression::CapteurPression(int pin_cs) {
  cs_pin = pin_cs;
}

// 2. Fonction d'initialisation
bool CapteurPression::initialiser() {
  
  if (!bmp.begin(cs_pin, &SPI)) {
    return false;
  }

  // --- CONFIGURATION EXTRÊME PRÉCISION (ALTIMÈTRE) ---
  
  // Température : Oversampling 8X est un bon compromis
  bmp.setTemperatureOversampling(BMP5XX_OVERSAMPLING_8X);
  
  // Pression : Oversampling 128X (Maximum absolu pour écraser le bruit et capter le cm)
  bmp.setPressureOversampling(BMP5XX_OVERSAMPLING_128X); 
  
  // Filtre IIR pour lisser les courants d'air
  // On utilise un coefficient de 3 pour stabiliser la mesure sans trop de latence
  bmp.setIIRFilterCoeff(BMP5XX_IIR_FILTER_COEFF_3);
  
  // Vitesse de sortie des données à 10 Hz
  bmp.setOutputDataRate(BMP5XX_ODR_10_HZ); 

  return true;
}

float CapteurPression::getPression() {

  if (bmp.performReading()) {
    return (float)(bmp.pressure);
  }
  
  return -1.0; 
}

void displayBMPInfo(Adafruit_BMP5xx &bmp) {
  Serial.println("Informations sur le capteur BMP581 :");
  Serial.print("Température : ");
  Serial.print(bmp.readTemperature());
  Serial.println(" °C");

  Serial.print("Pression : ");
  Serial.print(bmp.readPressure());
  Serial.println(" hPa");

  Serial.print("Altitude : ");
  Serial.print(bmp.readAltitude());
  Serial.println(" m");
}