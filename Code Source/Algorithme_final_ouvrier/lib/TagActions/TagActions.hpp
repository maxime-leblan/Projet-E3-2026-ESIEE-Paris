#pragma once

#include "UWBMessageManager.hpp"
#include "LowConsumptionMode.hpp"

#define HUB_ORDER_START_TAG_CALIBRATION 2
#define HUB_ORDER_END_TAG_CALIBRATION 3
#define HUB_ORDER_TAG_DISTANCE_FROM_SF 4

// Vitesse de course d'endurance moyenne d'un sportif (en m/s)
#define AVG_RUNNING_SPEED (4.16)

// Pin de la Xiao connecté au bipper
#define XIAO_TO_BIPPER_PIN 1

// Fréquence du bipper (en Hz)
#define BIPPER_FREQUENCY 400

/**
 * Initialise les périphériques propres à la XIAO (USB, Capteurs...)
 */
void initialiserXiao();

/**
 * Initialise la liaison et les broches de contrôle du module UWB
 */
void initialiserUWB();

/**
 * Fonction executée pour envoyer en continue la distance du tag par rapport aux ancres pendant que le hub effectue l'enregistrement de la zone de sécurité
 */
void safeZoneCalibration();