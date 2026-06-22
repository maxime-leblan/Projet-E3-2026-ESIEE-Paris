#include <Adafruit_TinyUSB.h>
#include "TagActions.hpp"
#include "CapteurPression.h"

void setup()
{
  Serial.println("\n[SETUP] === DEMARRAGE DU TAG ===");
  
  initialiserXiao();
  Serial.println("[SETUP] Materiel XIAO initialise.");
  
  initialiserUWB();
  Serial.println("[SETUP] Module UWB configure avec succes.");
  
  initialiserBMP581();
  Serial.println("[SETUP] Capteur de pression BMP581 pret.");

  digitalWrite(LED_RED, LOW);
  pinMode(PIN_VBAT_ENABLE, OUTPUT);
  digitalWrite(PIN_VBAT_ENABLE, HIGH); 
  analogReadResolution(12);             

  Serial.println("[SETUP] Fin de l'initialisation. Lancement de la boucle asynchrone...\n");
}

void loop()
{
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);

  // float vCurrentPressure = lirePressionBMP581(); // (Optionnel)

  // ====================================================================
  // 1. LECTURE UNIQUE ET CONTINUE DU PORT UWB
  // ====================================================================
  if (Serial1.available())
  {
      String ligne = Serial1.readStringUntil('\n');
      ligne.trim(); 

      // DÉBOGAGE BRUT : Décommente la ligne ci-dessous si tu veux voir le "bruit" radio complet
      // Serial.println("[RAW UWB] " + ligne);

      // -------------------------------------------------------------
      // CAS A : Télémétrie locale (Le Tag calcule ses distances)
      // -------------------------------------------------------------
      if (ligne.startsWith("AT+RANGE"))
      {
          int tid, r0, r1, r2, r3;
          int matched = sscanf(ligne.c_str(), "AT+RANGE=tid:%d, mask:%*x, seq: %*d, range: (%d,%d,%d,%d", 
                               &tid, &r0, &r1, &r2, &r3);

          if (matched == 5) 
          {
              if (tid == MY_TAG_ID) 
              {
                  // Log de succès : On a bien lu nos propres distances !
                  Serial.printf("[TELEM] Distances mesurees -> A0:%d, A1:%d, A2:%d, A3:%d\n", r0, r1, r2, r3);
                  
                  String payload = "tid:" + String(tid) + ",m:0,s:0,r:(" + 
                                   String(r0) + "," + String(r1) + "," + 
                                   String(r2) + "," + String(r3) + ")";
                  String atCommand = "AT+DATA=" + String(payload.length()) + "," + payload;
                  
                  Serial1.println(atCommand);
                  Serial.println("[TX RADIO] Broadcast expédié aux ancres : " + payload);
              } 
              else 
              {
                  // Ce log permet de voir si ton module écoute les slots d'autres tags par erreur
                  Serial.printf("[TELEM] Trame ignoree (Appartient au Tag %d)\n", tid);
              }
          } 
          else 
          {
              Serial.println("[ERREUR] Impossible de parser (sscanf a echoue) : " + ligne);
          }
      }
      // -------------------------------------------------------------
      // CAS B : Message distant reçu (Ordre du Hub)
      // -------------------------------------------------------------
      else if (ligne.startsWith("AT+RDATA"))
      {
          Serial.println("\n[RX RADIO] Trame de donnees recues : " + ligne);
          
          int sender, receiver, orderType;
          float distanceValue = 0.0f;
          
          int matched = sscanf(ligne.c_str(), "AT+RDATA=%*d,%d:%d:%d:%f", 
                               &sender, &receiver, &orderType, &distanceValue);

          if (matched >= 3) 
          {
              Serial.printf("[DECODAGE] Emetteur:%d | Cible:%d | Ordre:%d | Valeur:%.2f\n", 
                            sender, receiver, orderType, distanceValue);
                            
              // Vérification que le message nous est bien destiné
              if (receiver == MY_TAG_ID || receiver == 255) 
              {
                  if (orderType == HUB_ORDER_START_TAG_CALIBRATION)
                  {
                      Serial.println("[ACTION] Ordre de calibration ! Lancement de la boucle bloquee...");
                      noTone(XIAO_TO_BIPPER_PIN);
                      safeZoneCalibration();
                      Serial.println("[ACTION] Calibration terminee. Reprise normale.");
                  }
                  else if (orderType == HUB_ORDER_TAG_DISTANCE_FROM_SF)
                  {
                      Serial.printf("[ACTION] Alerte Hub ! Distance de securite = %.2fm\n", distanceValue);
                      
                      if (distanceValue > 0)
                      {
                          noTone(XIAO_TO_BIPPER_PIN);
                          int vSleepTime = (distanceValue / AVG_RUNNING_SPEED) * 1000;
                          Serial.printf("[VEILLE] Le travailleur est safe. Mise en sommeil pour %d ms\n", vSleepTime);
                          
                          veilleUWB();
                          veilleXiao(vSleepTime);
                          
                          reveilXiao();
                          reveilUWB();
                          Serial.println("[REVEIL] Systèmes relances.");
                      }
                      else
                      {
                          Serial.println("[DANGER] VIOLATION DE ZONE ! Activation du bipper.");
                          tone(XIAO_TO_BIPPER_PIN, BIPPER_FREQUENCY);
                      }
                  }
                  else 
                  {
                      Serial.printf("[ACTION] Ordre inconnu ignore (%d)\n", orderType);
                  }
              }
              else 
              {
                  Serial.printf("[DECODAGE] Message ignore (Il est destine au Tag %d)\n", receiver);
              }
          }
          else 
          {
              Serial.println("[ERREUR] Le parsing de l'ordre a echoue : " + ligne);
          }
      }
  }

  // ====================================================================
  // 2. GESTION DE LA BATTERIE
  // ====================================================================
  static uint32_t vLastBatCheckTime = millis(); 
  const uint32_t TimeoutBatMS = 5000; // Je l'ai passé à 5 secondes pour ne pas polluer tes logs

  if (millis() - vLastBatCheckTime > TimeoutBatMS)
  {
      vLastBatCheckTime = millis(); 

      if(calculBatteryLow())
      {
          Serial.println("\n[ALERTE] Niveau de batterie critique !");
          tone(XIAO_TO_BIPPER_PIN, BIPPER_FREQUENCY);
          digitalWrite(LED_RED, HIGH);
          delay(2000); 
          noTone(XIAO_TO_BIPPER_PIN);
          digitalWrite(LED_RED, LOW);
      }
  }
}