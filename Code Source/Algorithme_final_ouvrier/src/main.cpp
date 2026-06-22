#include <Adafruit_TinyUSB.h>
#include "TagActions.hpp"
#include "CapteurPression.h"

void setup()
{
  initialiserXiao();
  initialiserUWB();
  initialiserBMP581();

  digitalWrite(LED_RED, LOW);


  pinMode(PIN_VBAT_ENABLE, OUTPUT);
  digitalWrite(PIN_VBAT_ENABLE, HIGH); // On garde éteint par défaut
  analogReadResolution(12);             // Configuration de la résolution 12 bits
}

void loop()
{
    // On écoute le port Série matériel lié au module UWB
    if (Serial1.available()) 
    {
        // On lit la ligne entière UNE SEULE FOIS
        String ligne = Serial1.readStringUntil('\n');
        ligne.trim(); // On nettoie les sauts de ligne invisibles
        
        // On affiche TOUT ce que le module UWB recrache, sans exception
        Serial.println("[UWB RAW] : " + ligne);
    }
}