#include "CapteurPression.hpp"

Adafruit_BMP5xx bmp; 
float offset_pression = 0.0;

void configurer_reg_BMP() {
  // Configuration des registres pour la précision maximale (128X)
  bmp.setTemperatureOversampling(BMP5XX_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP5XX_OVERSAMPLING_128X);
  bmp.setIIRFilterCoeff(BMP5XX_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP5XX_ODR_10_HZ); 
}

// ====================================================================
// VERSION 1 : Initialisation avec REMAPPAGE des broches
// ====================================================================
void initialiserBMP581(int pin_cs, SPIClass &spi, int pin_sck, int pin_miso, int pin_mosi) {
  
  // Configuration des broches physiques sur l'instance SPI passée en paramètre
  #if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED)
    spi.setSCK(pin_sck);
    spi.setMISO(pin_miso);
    spi.setMOSI(pin_mosi);
    spi.begin(); // Démarre le bus avec le remappage
  #elif defined(ARDUINO_ARCH_ESP32)
    spi.begin(pin_sck, pin_miso, pin_mosi, pin_cs);
  #else
    // Fallback si l'architecture ne supporte pas le remappage dynamique de cette façon
    spi.begin(); 
  #endif

  // Initialisation du shield Adafruit avec l'instance configurée
  if (!bmp.begin(pin_cs, &spi)) {
    Serial.println("Erreur: Impossible d'initialiser le BMP581 (Mode Remap)");
    return;
  }
  configurer_reg_BMP();
}

// ====================================================================
// VERSION 2 (OVERLOAD) : Initialisation avec broches par DÉFAUT
// ====================================================================
void initialiserBMP581(int pin_cs, SPIClass &spi) {
  
  // On appelle simplement spi.begin() sans toucher aux broches natives
  spi.begin();

  if (!bmp.begin(pin_cs, &spi)) {
    Serial.println("Erreur: Impossible d'initialiser le BMP581 (Mode Par Defaut)");
    return;
  }
  configurer_reg_BMP();
}

float getPressionBMP581_RAW() {

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

// Effectue la tare de la pression actuelle
void tarerBMP581() {
  float somme = 0.0;
  int mesures_valides = 0;
  
  // On fait la moyenne de 5 mesures pour lisser le bruit de départ
  for (int i = 0; i < 5; i++) {
    float p = getPressionBMP581_RAW();
    if (p > 0.0) {
      somme += p;
      mesures_valides++;
    }
    delay(100); // Puisque ODR = 10_HZ, une mesure est disponible toutes les 100ms
  }
  
  if (mesures_valides > 0) {
    offset_pression = somme / mesures_valides;
  } else {
    offset_pression = 101325.0; // Valeur par défaut standard (niveau de la mer) en cas de bug
  }
}

// Retourne la variation de pression en Pascal par rapport à la tare (valeur signée)
float getPressionBMP581() {
  float p_actuelle = getPressionBMP581_RAW();
  return p_actuelle - offset_pression;
}