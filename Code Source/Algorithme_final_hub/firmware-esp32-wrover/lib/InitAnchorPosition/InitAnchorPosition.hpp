#pragma once

#include <vector>

#include "../MessageManager/CANMessageManager.hpp"

/**
 * Envoie un message à toutes les Ancres dont l'id est passé en paramètre pour leur ordonner d'inverser leur mode de comportement. Elles peuvent passer du mode Ancre -> Tag ou du mode Tag -> Ancre.
 * @param pAnchorsId Liste des identifiants de toutes les Ancres devant changer d'état
 */
void toggleAnchorsMode(std::vector<int> pAnchorsId);
