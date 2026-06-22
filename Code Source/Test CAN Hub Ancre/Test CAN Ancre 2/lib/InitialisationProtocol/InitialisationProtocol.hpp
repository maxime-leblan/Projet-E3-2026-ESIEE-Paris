#pragma once

#include "CANMessageManager.hpp"
#include "UWBModuleActions.hpp"

/**
 * Fonction qui ordonne à l'ancre dont l'id est passé en paramètre d'attendre un message du Hub pour se transformer en tag, puis d'envoyer en boucle ses distances par rapport à l'ancre statique (la seule qui reste une ancre sur les 4)
 * @param pMyAnchorId Identifiant de l'ancre qui appelle cette fonction
 */
void runInitializationPhase(uint8_t pMyAnchorId);

/**
 * Fonction qui exécute le protocole complet de l'initialisation des positions des ancres (côté ancre). Entre autre, tant que le hub n'a pas envoyé l'ordre HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL, on execute la fonction runInitializationPhase
 * @param pMyAnchorId Identifiant de l'ancre qui appelle cette fonction
 */
void runCompleteInitialisationPhase(uint8_t pMyAnchorId);