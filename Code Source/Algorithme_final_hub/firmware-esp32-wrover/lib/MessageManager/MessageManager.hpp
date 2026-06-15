#pragma once

// Définition de la macro pour activer une option spécifique
// Cela peut être fait ici ou via le compilateur (-DMY_OPTION)

#define MY_OPTION

#ifdef MY_OPTION
    // Inclure l'en-tête de la première bibliothèque si la macro est définie
    #include "WifiMessageManager.hpp"
#else
    // Inclure l'en-tête de la seconde bibliothèque par défaut
    #include "WifiMessageManager.hpp"
#endif

void sendData();