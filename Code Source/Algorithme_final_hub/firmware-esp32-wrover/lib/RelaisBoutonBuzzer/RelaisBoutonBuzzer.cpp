#include "RelaisBoutonBuzzer.hpp"

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
