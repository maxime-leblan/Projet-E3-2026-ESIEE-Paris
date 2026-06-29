#pragma once

#include "UWBMessageManager.hpp"
#include "LowConsumptionMode.hpp"

// Id du tag de l'ouvrier
#define MY_TAG_ID 4

// Mode (Tag=0, Ancre=1):"x", Débit (6.8M=1, 860K=0):"y", Filtre (ON=1, OFF=0):"z" 
#define TAG_MODE_CONFIG "0,1,0"

// Identifiant du réseau UWB
#define TAG_NETWORK_ID 1111

// Nb tags max, Minimum fenetre calcul, Etendre le paquet ou non
#define TAG_SETCAP_CONFIG "6,10,1"
 
/*
Si vaut 1 : Permet au tag de calculer aut
*/
#define TAG_AUTO_REPORT 1

#define HUB_ORDER_START_TAG_CALIBRATION 2
#define HUB_ORDER_END_TAG_CALIBRATION 3
#define HUB_ORDER_TAG_DISTANCE_FROM_SF 4

// Vitesse de course d'endurance moyenne d'un sportif (en m/s)
#define AVG_RUNNING_SPEED (4.16)

// Pin de la Xiao connecté au bipper
#define XIAO_TO_BIPPER_PIN D1

// Fréquence du bipper (en Hz)
#define BIPPER_FREQUENCY 200



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