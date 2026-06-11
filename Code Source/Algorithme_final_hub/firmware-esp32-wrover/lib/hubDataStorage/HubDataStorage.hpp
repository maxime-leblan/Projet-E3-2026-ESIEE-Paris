#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>

#include "../../../Algorithme-trilateration/Trilateration.h"

#define ANCHORS_NUMBER 4

/*
Instanciation de la librairie Preferences pour gérer l'écriture/la lecture dans la Flash de l'ESP32-WROVER
*/
Preferences preference;

/*
Initialise des ancres par défauts qui n'ont pas de positions et renvoie la liste de ces ancres
pNbAnchors - nombre d'ancres à initialiser (valant ANCHORS_NUMBER par défaut)
*/
UWBModuleList initAnchors(int pNbAnchors=ANCHORS_NUMBER);

/*
Affiche des messages dans le moniteur série pour vérifier que la PSRAM est bien active
*/
void initHub();

/*
Réinitialise complètement la Flash en supprimant tous les namespaces qui étaient stockés sur la partition principale.
ATTENTION : faire cette action uniquement pour nettoyer définitivement la Flash de toutes ces données, pour pouvoir ensuite la réutiliser comme neuve.
==> Il est IMPERATIF de recharger un autre programme qui n'utilise pas cette fonction (resetFlash()) dans l'ESP32 pour qu'elle
ne réinitialise pas sa mémoire Flash à chaque mise sous tension.
*/
void resetFlash();