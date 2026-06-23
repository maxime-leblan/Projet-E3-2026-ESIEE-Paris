#pragma once

#include <Arduino.h>

// Définissez le pin de la XIAO relié physiquement à la broche "UART2 RX" de l'UWB
#define UWB_WAKEUP_PIN 0  // Broche D0 de la XIAO ici
#define PIN_VBAT_ENABLE 14             // Broche de contrôle du MOSFET pour la lecture de la batterie (P0.14)
#define PIN_CHARGE_STATUS 23

#define LED_RED_CARTE D4

/**
 * Met le module UWB en veille prolongée (Consommation minimale)
 */
void veilleUWB();

/**
 * Réveille le module UWB via une impulsion matérielle sur son UART2 RX
 */
void reveilUWB();

// --- FONCTIONS POUR LA XIAO BLE ---

/**
 * Met la XIAO BLE en sommeil basse consommation (System ON) pendant une durée précise
 * @param duree_ms Durée de la veille en millisecondes
 */
void veilleXiao(uint32_t duree_ms);

/**
 * Relance les périphériques de la XIAO après son réveil automatique
 */
void reveilXiao();

/**
 * Calcule si la batterie est faible ou non
 * @return true si la batterie est faible, false sinon
 */
bool calculBatteryLow();

/**
 * Regarde si la carte est en charge et contrôle l'état de la LED bleue
 */
void chargebattery();


