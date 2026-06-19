#pragma once

#include "UWBMessageManager.hpp"
#include "LowConsumptionMode.hpp"


/**
 * Initialise les périphériques propres à la XIAO (USB, Capteurs...)
 */
void initialiserXiao();

/**
 * Initialise la liaison et les broches de contrôle du module UWB
 */
void initialiserUWB();