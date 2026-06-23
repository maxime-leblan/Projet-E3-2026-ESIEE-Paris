#pragma once

#include "CANMessageManager.hpp"
#include "GridLibrary.hpp"
#include <unordered_map>
#include <vector>
#include <string>

// Paramètres pour la descente de gradient
#define ITERATIONS 100
#define LEARNING_RATE 0.01

/**
 * Renvoie un dictionnaire contenant les distances entre l'Ancre statique passée en paramètre et les autres modules (temporairement Tags).
 * @param pStaticAnchorId Identifiant de l'Ancre qui reste fixe et qui effectue les mesures.
 * @param pAnchors Liste complète des modules du système.
 */
std::unordered_map<std::string, float> getAnchorDistances(int pStaticAnchorId, UWBModuleList pAnchors);

/**
 * Orchestre le protocole de calibration matérielle puis attribue les positions calculées par descente de gradient.
 * @param pAnchors Référence à la variable contenant la liste des Ancres du véhicule.
 */
void initAnchorsPosition(UWBModuleList & pAnchors);

/**
 * Envoie à toutes les Ancres un signal indiquant le début ou la fin de la phase d'initialisation.
 * @param pAnchors Référence à la variable contenant la liste des Ancres.
 * @param pSignalType HUB_ORDER_START_ANCHOR_INIT_POSITION_PROTOCOL ou HUB_ORDER_END_ANCHOR_INIT_POSITION_PROTOCOL.
 */
void sendToAnchorsInitialisationPhaseSignal(UWBModuleList & pAnchors, int pSignalType);

/**
 * Envoie l'ordre CAN de basculement de mode aux modules listés.
 * @param pAnchorsId Liste des identifiants des modules devant changer d'état (Ancre <-> Tag).
 * @param pStaticAnchorId Identifiant de l'Ancre qui reste fixe pendant l'opération.
 */
void toggleAnchorsMode(std::vector<int> pAnchorsId, uint8_t pStaticAnchorId);

