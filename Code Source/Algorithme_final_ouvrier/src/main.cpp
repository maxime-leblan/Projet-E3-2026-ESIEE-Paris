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
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);
  // TESTS

  
  //String vMessage1;
  //readDistancesInTagSerial(Serial1, Serial, vMessage1);
  //Serial.println("Ça marche dans la loop");
  

  // Mesure de la pression actuelle
  float vCurrentPressure = lirePressionBMP581();
  if (vCurrentPressure != -1.0) {
    //Serial.print("Pression : ");
    //Serial.print(vCurrentPressure, 2);
    //Serial.println(" hPa");
  }

  bool vIsDistanceReceived = false;
  float vTagDistanceFromSafeZone = -1.0f;

  String vMessageReceived;
  UWBMessage vMessageReceivedData;

  // On vérifie si le module UWB du tag a reçu un message quelconque (AT+RANGE ou bien ordre du Hub)
  if (receiveUWBMessage(Serial1, vMessageReceived, Serial))
  {
    if (decodeUWBMessage(vMessageReceived, vMessageReceivedData, Serial))
    {
      // Si c'est un AT+RANGE, on l'envoie aux ancres
      if (vMessageReceivedData.aIsStandardDistanceMessage)
      {
        sendDistancesToAnchor(Serial1, Serial, vMessageReceived);
      }
      // Si c'est un ordre de calibration du tag, on la démarre
      else if (vMessageReceivedData.orderType == HUB_ORDER_START_TAG_CALIBRATION)
      {
        // On stoppe le bipper s'il sonne toujours
        noTone(XIAO_TO_BIPPER_PIN);

        safeZoneCalibration();
      }
      // Si c'est la distance du tag par rapport au véhicule, on récupère la distance
      else if (vMessageReceivedData.orderType == HUB_ORDER_TAG_DISTANCE_FROM_SF)
      {
        vIsDistanceReceived = true;
        vTagDistanceFromSafeZone = vMessageReceivedData.dataValue;
      }
    }
  }

  // On vérifie d'abords que l'on a bien reçu une distance
  if (vIsDistanceReceived)
  {
    // Si la distance reçue est > 0, on calcule le temps de mise en veille du tag puis on le met en veille
    if (vTagDistanceFromSafeZone > 0)
    {
      // On stoppe le bipper s'il sonne toujours
          noTone(XIAO_TO_BIPPER_PIN);

      // On calcule le temps de veille en ms
      int vSleepTime = (vTagDistanceFromSafeZone / AVG_RUNNING_SPEED) * 1000;

      // On met tout le matériel en veille
      veilleUWB();
      veilleXiao(vSleepTime);

      // Puis on se réveille
      reveilXiao();
      reveilUWB();
    }
    // Sinon on fait bipper le tag
    else
    {
      // On fait sonner le bipper (rajouter )
      tone(XIAO_TO_BIPPER_PIN, BIPPER_FREQUENCY);
    }
  }
 // On vérifie si la batterie est faible
 uint32_t vStartTime = millis();
 const uint32_t TimeoutBatMS = 1000; // 

  if (millis() - vStartTime > TimeoutBatMS)
  {
    if(calculBatteryLow())
    {
      Serial.println("[XIAO] Batterie faible !");
      tone(XIAO_TO_BIPPER_PIN, BIPPER_FREQUENCY);
      digitalWrite(LED_RED, HIGH);
      delay(2000);
      noTone(XIAO_TO_BIPPER_PIN);
      digitalWrite(LED_RED, LOW);
    }
  vStartTime = millis();
  }
}