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

  
  String vMessage1;
  readDistancesInTagSerial(Serial1, Serial, vMessage1);
  //Serial.println("Ça marche dans la loop");
  

  // Mesure de la pression actuelle
  float vCurrentPressure = lirePressionBMP581();
  if (vCurrentPressure != -1.0) {
    //Serial.print("Pression : ");
    //Serial.print(vCurrentPressure, 2);
    //Serial.println(" hPa");
  }

  // FIN TESTS

  String vMessageReceived;
  UWBMessage vMessageReceivedData;

  // On vérifie si le hub veut que l'on passe en mode calibration de la zone de sécurité
  if (receiveUWBMessage(Serial1, vMessageReceived))
  {
    if (decodeUWBMessage(vMessageReceived, vMessageReceivedData))
    {
      if (vMessageReceivedData.orderType == HUB_ORDER_START_TAG_CALIBRATION)
      {
        // On stoppe le bipper s'il sonne toujours
        noTone(XIAO_TO_BIPPER_PIN);

        safeZoneCalibration();
      }
    }
    else
    {
      Serial.println("[PRINCIPAL] Impossible de décoder le message reçu");
    }
  }

  // On envoie aux ancres nos distances par rapport à elles
  sendDistancesToAnchor(Serial1, Serial);

  // On attend de recevoir par transmission UWB d’une ancre relayant un message du hub qui contient sa distance à la zone de sécurité
  bool vIsDistanceReceived = false;
  float vTagDistanceFromSafeZone = -1.0f;

  // On définit une sécurité de 800ms pour éviter le blocage infini
  uint32_t vStartTime = millis();
  const uint32_t vTimeoutMs = 800;

  while (!vIsDistanceReceived && (millis() - vStartTime < vTimeoutMs))
  {
    if (receiveUWBMessage(Serial1, vMessageReceived))
    {
      if (decodeUWBMessage(vMessageReceived, vMessageReceivedData))
      {
        if (vMessageReceivedData.orderType == HUB_ORDER_TAG_DISTANCE_FROM_SF)
        {
          vIsDistanceReceived = true;
          vTagDistanceFromSafeZone = vMessageReceivedData.dataValue;
        }
      }
      else
      {
        Serial.println("[PRINCIPAL] Impossible de décoder le message reçu");
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
 vStartTime = millis();
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
    /*
    else
    {
      int batteryLevel = calculBattery();
      Serial.print("[XIAO] Batterie OK : ");
      Serial.print(batteryLevel);
      Serial.println(" V");
      break;
    }
    */
    
  }

}