#include "CapteurPression.h"

Adafruit_BMP5xx bmp; 

void initialiserBMP581() {
  
  bmp.begin(BMP_CS, &SPI);

  // Réglages optionnels capteur BMP581 pour plus de precision (cf librairie Adafruit_BMP5xx)
  bmp.setTemperatureOversampling(BMP5XX_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP5XX_OVERSAMPLING_128X);
  bmp.setIIRFilterCoeff(BMP5XX_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP5XX_ODR_10_HZ); 
}

float getPressionBMP581() {

  if (bmp.performReading()) {
    return (float)(bmp.pressure);
  }

  return -1.0; 
}

void infosBMP() {
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