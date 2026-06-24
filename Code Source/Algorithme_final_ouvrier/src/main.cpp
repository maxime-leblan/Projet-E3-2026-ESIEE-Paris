#include <Adafruit_TinyUSB.h>
#include "TagActions.hpp"
#include "CapteurPression.hpp"

void setup()
{
  // --- SÉQUENCE DE DÉMARRAGE ---
  Serial.begin(115200);
  // Petite pause optionnelle pour laisser le temps d'ouvrir le moniteur série
  // while(!Serial) delay(10); 

  Serial.println("\n==================================================");
  Serial.println("[SETUP] Démarrage du Tag Ouvrier VigiZone...");
  Serial.println("==================================================");

  digitalWrite(LED_RED, LOW);
  
  Serial.println("[SETUP] Étape 1/3 : Initialisation de la carte XIAO...");
  initialiserXiao();
  
  Serial.println("[SETUP] Étape 2/3 : Initialisation du module UWB...");
  initialiserUWB();
  
  Serial.println("[SETUP] Étape 3/3 : Initialisation du capteur BMP581 (SPI)...");
  initialiserBMP581(D3, SPI); 
  
  Serial.println("[SETUP] Configuration des broches d'alimentation et LEDs...");
  pinMode(LED_RED_CARTE, OUTPUT);
  pinMode(LED_BLUE, HIGH); 
  pinMode(PIN_VBAT_ENABLE, OUTPUT);
  digitalWrite(PIN_VBAT_ENABLE, HIGH); 
  analogReadResolution(12);  
  digitalWrite(LED_RED, HIGH);           
  
  Serial.println("[SETUP] Terminé. Entrée dans la boucle principale.");
  Serial.println("==================================================\n");
}

void loop()
{
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);

  bool vIsDistanceReceived = false;
  float vTagDistanceFromSafeZone = 0.0f; // Initialisé à 0

  String vMessageReceived;
  UWBMessage vMessageReceivedData;

  // ====================================================================
  // 1. ÉCOUTE ET RÉCEPTION RADIO
  // ====================================================================
  if (receiveUWBMessage(Serial1, vMessageReceived, Serial))
  {
    //Serial.printf("[%lu] [RX] Trame captée : %s\n", millis(), vMessageReceived.c_str());
    
    if (decodeUWBMessage(vMessageReceived, vMessageReceivedData, Serial))
    {
      Serial.printf("[%lu] [PARSER] Décodage réussi. Analyse du type d'ordre...\n", millis());

      // --- TRI : LE TAG IGNORE SES PROPRES CALCULS ---
      if (vMessageReceivedData.aIsStandardDistanceMessage) {
         //Serial.printf("[%lu] [TRI] Trame AT+RANGE locale ignorée.\n", millis());
      }
      else if (vMessageReceivedData.orderType == HUB_ORDER_START_TAG_CALIBRATION)
      {
        Serial.printf("[%lu] [ACTION] Ordre reçu : DÉMARRER CALIBRATION.\n", millis());
        noTone(XIAO_TO_BIPPER_PIN);
        safeZoneCalibration();
        Serial.printf("[%lu] [ACTION] Calibration terminée, retour à l'écoute.\n", millis());
      }
      else if (vMessageReceivedData.orderType == HUB_ORDER_TAG_DISTANCE_FROM_SF || vMessageReceivedData.orderType == 3)
      {
        vIsDistanceReceived = true;
        vTagDistanceFromSafeZone = vMessageReceivedData.dataValue;
        Serial.printf("[%lu] [ACTION] Ordre reçu : MISE À JOUR DISTANCE = %.2f mètres.\n", millis(), vTagDistanceFromSafeZone);
      }
      else 
      {
         Serial.printf("[%lu] [ACTION] Ordre inconnu ignoré (Type : %d).\n", millis(), vMessageReceivedData.orderType);
      }
    }
    else 
    {
      Serial.printf("[%lu] [PARSER ERROR] Impossible de décoder la trame.\n", millis());
    }
  }

  // ====================================================================
  // 2. GESTION DES ALARMES ET MISE EN VEILLE
  // ====================================================================
  if (vIsDistanceReceived)
  {
    Serial.printf("[%lu] [LOGIQUE] Évaluation de la sécurité pour D = %.2f m...\n", millis(), vTagDistanceFromSafeZone);

    if (vTagDistanceFromSafeZone > 0.0f)
    {
      Serial.printf("[%lu] [LOGIQUE] Résultat : HORS ZONE DE DANGER. Coupure du bipper.\n", millis());
      noTone(XIAO_TO_BIPPER_PIN);
      
      // On calcule le temps de veille en ms
      int vSleepTime = (vTagDistanceFromSafeZone / AVG_RUNNING_SPEED) * 1000;
      Serial.printf("[%lu] [ENERGIE] Lancement de la séquence de VEILLE pour %d ms.\n", millis(), vSleepTime);

      // Désactivation matérielle
      Serial.printf("[%lu] -> Extinction UWB...\n", millis());
      veilleUWB();
      
      Serial.printf("[%lu] -> Sommeil XIAO...\n", millis());
      // --- ATTENTION : Le port série USB risque d'être coupé pendant veilleXiao ---
      Serial.flush(); // Force l'envoi des logs avant de dormir
      
      veilleXiao(vSleepTime);

      // Réactivation matérielle
      // Note: Le premier log au réveil peut être manqué si l'USB met du temps à se reconnecter
      reveilXiao();
      Serial.printf("[%lu] -> Réveil XIAO OK. Relance UWB...\n", millis());
      
      reveilUWB();
      Serial.printf("[%lu] -> Réveil UWB OK. Fin du cycle de veille.\n", millis());
    }
    else
    {
      Serial.printf("[%lu] [LOGIQUE] Résultat : DANGER ! Pénétration dans la zone (%.2f m <= 0.0m).\n", millis(), vTagDistanceFromSafeZone);
      Serial.printf("[%lu] [ALARME] DÉCLENCHEMENT DU BIPPER à %d Hz.\n", millis(), BIPPER_FREQUENCY);
      tone(XIAO_TO_BIPPER_PIN, BIPPER_FREQUENCY);
    }
  }
 
  // ====================================================================
  // 3. GESTION DE L'ÉNERGIE ET DES VOYANTS (Background)
  // ====================================================================
  static uint32_t lastBatCheck = 0;
  static bool isBatteryLow = false;

  // On vérifie la batterie toutes les 5 secondes
  if (millis() - lastBatCheck > 5000) {
    lastBatCheck = millis();
    isBatteryLow = calculBatteryLow();
    
    if (isBatteryLow) {
        Serial.printf("[%lu] [DIAGNOSTIC] Alerte : Niveau de batterie FAIBLE !\n", millis());
    } else {
        // Optionnel : Décommenter pour avoir un heartbeat régulier confirmant que la boucle tourne
        // Serial.printf("[%lu] [DIAGNOSTIC] Batterie OK. Système nominal.\n", millis());
    }
  }
  
  bool isChargerPlugged = (NRF_POWER->USBREGSTATUS & 0x01);
  bool isCharging = (digitalRead(PIN_CHARGE_STATUS) == LOW);

  static uint32_t lastBlinkTime = 0;
  static bool ledState = false;

  if (isChargerPlugged && isCharging) {
    if (millis() - lastBlinkTime > 500) { 
      lastBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(LED_RED_CARTE, ledState ? HIGH : LOW); 
      // Éviter de logger les clignotements pour ne pas polluer la console
    }
  } 
  else if (isBatteryLow) {
    if (millis() - lastBlinkTime > 200) {
      lastBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(LED_RED_CARTE, ledState ? HIGH : LOW);
    }
  } 
  else {
    digitalWrite(LED_RED_CARTE, LOW);
  }
}