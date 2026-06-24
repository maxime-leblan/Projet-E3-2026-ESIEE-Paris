#include <Adafruit_TinyUSB.h>
#include "TagActions.hpp"
#include "CapteurPression.hpp"

void setup()
{
  digitalWrite(LED_RED, LOW);
  initialiserXiao();
  initialiserUWB();
  initialiserBMP581(D3, SPI); // CS = D3, sur le BUS SPI par défaut (et pas SPI1)
  
  pinMode(LED_RED_CARTE, OUTPUT);
  pinMode(LED_BLUE, HIGH); // On éteint la LED bleue par défaut
  pinMode(PIN_VBAT_ENABLE, OUTPUT);
  digitalWrite(PIN_VBAT_ENABLE, HIGH); // On garde éteint par défaut
  analogReadResolution(12);  
  digitalWrite(LED_RED, HIGH);           // Configuration de la résolution 12 bits
  
}

void loop()
{
  //Serial.println("-------- START LOOP --------");

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);

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
      if (vMessageReceivedData.aIsStandardDistanceMessage){
        float vCurrentPression = getPressionBMP581();
        
        Serial.printf("[TAG TX DIAG] Lecture BMP581 = %.2f hPa\n", vCurrentPression);
        if (vCurrentPression <= 0.0f) {
          Serial.println("[TAG TX ERROR] Alerte : Valeur de pression invalide (Erreur capteur ou SPI) !");
        }
        
        sendDistancesWithPressionToAnchor(Serial1, Serial, vMessageReceived, vCurrentPression);
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
 
  //  MISE À JOUR DE L'ÉTAT DE LA BATTERIE (Toutes les 5s) ---
  static uint32_t lastBatCheck = 0;
  static bool isBatteryLow = false;

  if (millis() - lastBatCheck > 5000) {
    lastBatCheck = millis();
    isBatteryLow = calculBatteryLow();
  }
  bool isChargerPlugged = (NRF_POWER->USBREGSTATUS & 0x01);
  bool isCharging = (digitalRead(PIN_CHARGE_STATUS) == LOW);

  // GESTION CENTRALISÉE DE LA LED PAR PRIORITÉ ---
  static uint32_t lastBlinkTime = 0;
  static bool ledState = false;

  if (isChargerPlugged && isCharging) {
    // PRIORITÉ 1 : En charge -> Clignotement lent )
    if (millis() - lastBlinkTime > 500) { 
      lastBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(LED_RED_CARTE, ledState ? HIGH : LOW); 
    }
  } 
  else if (isBatteryLow) {
    // PRIORITÉ 2 : Batterie faible (et pas en charge) -> Clignotement rapide
    if (millis() - lastBlinkTime > 200) {
      lastBlinkTime = millis();
      ledState = !ledState;
      Serial.println("[XIAO] Batterie faible !");
      digitalWrite(LED_RED_CARTE, ledState ? HIGH : LOW);
    }
  } 
  else {
    // PRIORITÉ 3 : Tout est normal -> On éteint la LED
    digitalWrite(LED_RED_CARTE, LOW);
  }

  // (Sommeil/Réveil selon la charge) ---
   // veille_UWB_chargebattery();

  //Serial.println("-------- END LOOP --------\n");
}