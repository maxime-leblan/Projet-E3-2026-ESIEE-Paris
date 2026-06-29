#pragma once

#include <Arduino.h>

#define BUZZER_GPIO 4
#define RELAIS_GPIO 2  // GPIO2 != D2 (GPIO9)
#define BOUTON_READ_GPIO 35 // 2.9V si bouton appuyé, 0V sinon
#define LED_BOUTON_GPIO  12 

#define BUZZER_PWM_H


#define BUZZER_FREQUENCY 600

void initRelaisBoutonBuzzer();
void IRAM_ATTR couperRelais();
void faireSonnerBuzzer();
void eteindreBuzzer();


// Déclarations des fonctions accessibles depuis l'extérieur
void initialiserBuzzer(int pin);
void configurerPWM(unsigned int frequence, int dutyPourcent);
void afficherStatut();

// Getters et Setters pour interagir proprement avec les variables internes
void setSystemeActif(bool actif);
bool getSystemeActif();

void setFrequenceHz(int freq);
int getFrequenceHz();

void setDutyCyclePourcent(int duty);
int getDutyCyclePourcent();

