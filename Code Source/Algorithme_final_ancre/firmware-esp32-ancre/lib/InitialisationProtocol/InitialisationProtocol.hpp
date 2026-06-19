#pragma once

#include "CANMessageManager.hpp"
#include "UWBModuleActions.hpp"

/**
 * Fonction qui ordonne à l'ancre dont l'id est passé en paramètre d'attendre un message du Hub pour se transformer en tag, puis d'envoyer en boucle ses distances par rapport à l'ancre statique (la seule qui reste une ancre sur les 4)
 */
void runInitializationPhase(uint8_t pMyAnchorId);