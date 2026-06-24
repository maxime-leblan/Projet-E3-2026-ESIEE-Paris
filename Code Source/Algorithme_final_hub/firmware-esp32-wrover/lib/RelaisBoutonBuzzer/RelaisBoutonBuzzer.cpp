#include "RelaisBoutonBuzzer.hpp"
#include <Arduino.h>

static int _pinBuzzer = 18;
static int _dutyCyclePourcent = 50;
static int _frequenceHz = 440;
static bool _systemeActif = true;

const int canalPWM = 0; 
const int resolutionBits = 10;
const int maxDutyValeur = (1 << resolutionBits) - 1; // 1023

void initRelaisBoutonBuzzer() {
    pinMode(RELAIS_GPIO, OUTPUT);
    pinMode(BUZZER_GPIO, OUTPUT);
    pinMode(LED_BOUTON_GPIO, OUTPUT);
    pinMode(BOUTON_READ_GPIO, INPUT); // PULLdown déjà externe
    // ACTIVATION RELAIS VITAL
    digitalWrite(RELAIS_GPIO, HIGH);
    digitalWrite(LED_BOUTON_GPIO, HIGH);
    // Dès que le BOUTON_READ_GPIO passe à un niveau HAUT (RISING), on appelle couperRelaisUrgence
    attachInterrupt(digitalPinToInterrupt(BOUTON_READ_GPIO), couperRelais, RISING);
}

// interruption bouton pour couper le relais
void IRAM_ATTR couperRelais() {
    // Action immédiate : on éteint le GPIO
    digitalWrite(RELAIS_GPIO, LOW); 
}

void faireSonnerBuzzer() {
    tone(BUZZER_GPIO, BUZZER_FREQUENCY);
}

void eteindreBuzzer() {
    noTone(BUZZER_GPIO);
}

// Initialisation de la broche
void initialiserBuzzer(int pin) {
  _pinBuzzer = pin;
  // Associe la broche physique au canal PWM sélectionné
  ledcAttachPin(_pinBuzzer, canalPWM);
  configurerPWM(_frequenceHz, _dutyCyclePourcent);
}

// Configuration du PWM (Spécifique ESP32 version < 3.0)
void configurerPWM(unsigned int frequence, int dutyPourcent) {
  _frequenceHz = frequence;
  _dutyCyclePourcent = dutyPourcent;

  if (_frequenceHz == 0 || !_systemeActif || _dutyCyclePourcent == 0) {
    ledcWrite(canalPWM, 0); // Met le devoir à 0%
    return;
  }

  // Configuration de la fréquence et de la résolution sur le canal
  ledcSetup(canalPWM, _frequenceHz, resolutionBits);
  
  // Calcul de la valeur finale (0 à 1023)
  int ocrValeur = (maxDutyValeur * _dutyCyclePourcent) / 100;
  
  // Application au canal
  ledcWrite(canalPWM, ocrValeur);
}

// Affichage du statut dans le Moniteur Série
void afficherStatut() {
  Serial.println(F("--- STATUT ACTUEL (LIBRAIRIE ESP32 v2) ---"));
  Serial.print(F("État : "));
  Serial.println(_systemeActif ? F("ALLUMÉ (ON)") : F("ÉTEINT (OFF)"));
  Serial.print(F("Fréquence : "));
  Serial.print(_frequenceHz);
  Serial.println(F(" Hz"));
  Serial.print(F("Duty Cycle : "));
  Serial.print(_dutyCyclePourcent);
  Serial.println(F(" %"));
  Serial.println(F("Commandes : '+' (ON), '-' (OFF), 'f440' (freq), 'd50' (duty)"));
  Serial.println(F("------------------------------------------"));
}

// --- GETTERS ET SETTERS ---
void setSystemeActif(bool actif) { _systemeActif = actif; }
bool getSystemeActif() { return _systemeActif; }

void setFrequenceHz(int freq) { _frequenceHz = freq; }
int getFrequenceHz() { return _frequenceHz; }

void setDutyCyclePourcent(int duty) { _dutyCyclePourcent = duty; }
int getDutyCyclePourcent() { return _dutyCyclePourcent; }