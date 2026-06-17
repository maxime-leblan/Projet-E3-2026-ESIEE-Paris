#pragma once

#include "../MessageManager/CANMessageManager.hpp"
#include "GridLibrary.hpp"

// On définit des macros contenant les paramètres que l'on veut utiliser pour la descente de gradient
#define ITERATIONS 100
#define LEARNING_RATE 0.01

/**
 * Renvoie un tableau contenant toutes les distances entre l'Ancre passée en paramètre et les autres Ancres existantes
 * @param pAnchorId Identifiant de l'Ancre dont on veut récupérer toutes les distances par rapport aux autres ancres
 * @param pAnchors Liste des Ancres existantes. La fonction en a besoin pour déterminer les autres Ancres existantes pour calculer les distances à l'ancre dont l'id passé en paramètre.
 */
std::unordered_map<string, float> getAnchorDistances(int pAnchorId, UWBModuleList pAnchors);

/**
 * Attribue les positions de départ de chaque Ancre dans le repère d'origine stocké par le Hub
 * @param pAnchors Référence à la variable contenant la liste des Ancres du véhicule
 */
void initAnchorsPosition(UWBModuleList & pAnchors);

/**
 * Envoie un message à toutes les Ancres dont l'id est passé en paramètre pour leur ordonner d'inverser leur mode de comportement. Elles peuvent passer du mode Ancre -> Tag ou du mode Tag -> Ancre.
 * @param pAnchorsId Liste des identifiants de toutes les Ancres devant changer d'état
 */
void toggleAnchorsMode(std::vector<int> pAnchorsId);

