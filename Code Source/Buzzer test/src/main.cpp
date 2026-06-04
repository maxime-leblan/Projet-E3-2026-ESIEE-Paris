#include <Arduino.h>

const int pinBuzzer = 9; // Broche D9 (gérée par le Timer 1)
int dutyCyclePourcent = 50; // Duty cycle initial à 50%
int frequenceHz = 440;      // Fréquence initiale (Note LA - 440Hz)
bool systemeActif = true;   // État du buzzer (ON/OFF)

// Fonction pour configurer le Timer 1 de l'ATmega328P pour une fréquence et un duty cycle précis
// Inspiré des guides d'application d'Atmel sur le PWM "Phase and Frequency Correct"
void configurerPWM(unsigned int frequence, int dutyPourcent) {
  if (frequence == 0 || !systemeActif || dutyPourcent == 0) {
    TCCR1A = 0; // Déconnecte le timer, sortie à 0
    digitalWrite(pinBuzzer, LOW);
    return;
  }

  // Configuration du Timer 1 : Mode 8 (Phase and Frequency Correct PWM avec ICR1 comme TOP)
  pinMode(pinBuzzer, OUTPUT);
  
  // Calcul de la valeur TOP pour atteindre la fréquence voulue avec un prescaler de 8
  // Formule : TOP = F_CPU / (2 * Prescaler * F_cible)
  long topValeur = 16000000L / (2 * 8 * frequence);
  
  // Sécurité pour les limites du Timer 1 (16 bits = 65535 max)
  if (topValeur > 65535) topValeur = 65535;

  // Calcul de la valeur de comparaison pour le Duty Cycle (OCR1A)
  long ocrValeur = (topValeur * dutyPourcent) / 100;

  // Application aux registres du processeur
  ICR1 = topValeur;
  OCR1A = ocrValeur;

  // TCCR1A : Toggle OC1A au comparateur, Mode PWM Phase/Freq Correct
  TCCR1A = _BV(COM1A1);
  // TCCR1B : Mode PWM Phase/Freq Correct (ICR1), Prescaler à 8
  TCCR1B = _BV(WGM13) | _BV(CS11); 
}

void afficherStatut() {
  Serial.println(F("--- STATUT ACTUEL ---"));
  Serial.print(F("État : "));
  Serial.println(systemeActif ? F("ALLUMÉ (ON)") : F("ÉTEINT (OFF)"));
  Serial.print(F("Fréquence : "));
  Serial.print(frequenceHz);
  Serial.println(F(" Hz"));
  Serial.print(F("Duty Cycle : "));
  Serial.print(dutyCyclePourcent);
  Serial.println(F(" %"));
  Serial.println(F("Commandes : '+' (ON), '-' (OFF), 'f440' (freq), 'd50' (duty)"));
  Serial.println(F("---------------------"));
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Temps de stabilisation du moniteur série
  
  configurerPWM(frequenceHz, dutyCyclePourcent);
  afficherStatut();
}

void loop() {
  if (Serial.available() > 0) {
    String commande = Serial.readStringUntil('\n');
    commande.trim(); // Nettoie les espaces et retours à la ligne

    if (commande.length() > 0) {
      char type = commande.charAt(0);

      // Commande d'allumage / extinction
      if (type == '+') {
        systemeActif = true;
        Serial.println(F("[Action] Buzzer activé."));
      } 
      else if (type == '-') {
        systemeActif = false;
        Serial.println(F("[Action] Buzzer désactivé."));
      } 
      // Commande de fréquence (ex: f1000)
      else if (type == 'f' || type == 'F') {
        int nouvelleFreq = commande.substring(1).toInt();
        if (nouvelleFreq >= 31 && nouvelleFreq <= 20000) { // Limites physiques du montage
          frequenceHz = nouvelleFreq;
          Serial.print(F("[Action] Fréquence modifiée à : "));
          Serial.println(frequenceHz);
        } else {
          Serial.println(F("[Erreur] Fréquence hors limites (31Hz - 20kHz)."));
        }
      } 
      // Commande de Duty Cycle (ex: d30)
      else if (type == 'd' || type == 'D') {
        int nouveauDuty = commande.substring(1).toInt();
        if (nouveauDuty >= 0 && nouveauDuty <= 100) {
          dutyCyclePourcent = nouveauDuty;
          Serial.print(F("[Action] Duty Cycle modifié à : "));
          Serial.println(dutyCyclePourcent);
        } else {
          Serial.println(F("[Erreur] Le duty cycle doit être entre 0 et 100%."));
        }
      }

      // Appliquer les changements et afficher le récapitulatif
      configurerPWM(frequenceHz, dutyCyclePourcent);
      afficherStatut();
    }
  }
}