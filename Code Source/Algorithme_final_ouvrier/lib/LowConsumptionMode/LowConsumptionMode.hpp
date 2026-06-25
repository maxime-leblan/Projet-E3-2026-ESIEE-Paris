#pragma once

#include <Arduino.h>

// --- BROCHES MATÉRIELLES (ARDUINO) ---
// Pin de la XIAO relié physiquement à la broche "UART2 RX" de l'UWB
#define UWB_WAKEUP_PIN 0  

// Broche de lecture de l'état de charge (P0.17 mapped sur 23)
#define PIN_CHARGE_STATUS 23

// Broche de la LED rouge utilisateur de la carte
#define LED_RED_CARTE D2 

// --- BROCHES PHYSIQUES BARE-METAL (Registres NRF_P0) ---
// Numéros des pins physiques de la puce Nordic pour le contrôle d'énergie
#define PHYSICAL_PIN_VBAT_ENABLE 14  // Broche P0.14 (Contrôle du pont diviseur)
#define PHYSICAL_PIN_CHARGE_SPEED 13 // Broche P0.13 (Contrôle du BQ25101)

// Broche analogique pour la lecture de tension
#define ANALOG_VBAT_READ_PIN 32

// --- FONCTIONS UWB ---

/**
 * Met le module UWB en veille prolongée (Consommation minimale)
 */
void veilleUWB();

/**
 * Réveille le module UWB via une impulsion matérielle sur son UART2 RX
 */
void reveilUWB();

// --- FONCTIONS XIAO BLE ---

/**
 * Met la XIAO BLE en sommeil basse consommation (System ON) pendant une durée précise
 * @param duree_ms Durée de la veille en millisecondes
 */
void veilleXiao(uint32_t duree_ms);

/**
 * Relance les périphériques de la XIAO après son réveil automatique
 */
void reveilXiao();

// --- FONCTIONS D'ALIMENTATION ET BATTERIE ---

/**
 * Initialise la résolution de l'ADC et les broches matérielles pour lire la batterie
 */
void initialiserLectureBatterie();

/**
 * Force la puce de charge interne (BQ25101) à charger à 100mA (Charge Rapide)
 */
void activerChargeRapide100mA();

/**
 * Laisse la puce de charge interne (BQ25101) charger à 50mA (Charge Normale par défaut)
 */
void activerChargeNormale50mA();

/**
 * Calcule si la batterie est faible (lecture analogique via pont diviseur)
 * @return true si la batterie est sous le seuil critique, false sinon
 */
bool calculBatteryLow();