#pragma once

#include "CANMessageManager.hpp"
#include "GridLibrary.hpp"
#include <unordered_map>
#include <vector>
#include <string>

#define ITERATIONS 100
#define LEARNING_RATE 0.01

/**
 * Orchestre le protocole de calibration matérielle par permutation des rôles Ancre/Tag,
 * puis calcule les positions 3D via descente de gradient.
 */
void initAnchorsPosition(UWBModuleList & pAnchors);

/**
 * Diffuse un ordre global d'initialisation sur le bus CAN.
 */
void sendToAnchorsInitialisationPhaseSignal(UWBModuleList & pAnchors, uint8_t pSignalType);

/**
 * Force explicitement le rôle matériel d'un module (Ancre ou Tag) via le bus CAN.
 */
void setAnchorRole(uint8_t pAnchorId, uint8_t pRoleOrder);