#pragma once

#include <Arduino.h>

/**
 * Fonction bloquante exécutant le protocole complet d'initialisation des positions.
 * Maintient l'ancre dans un état d'écoute hybride (CAN/UWB) jusqu'à la libération par le Hub.
 */
void runCompleteInitialisationPhase(uint8_t pMyAnchorId);

