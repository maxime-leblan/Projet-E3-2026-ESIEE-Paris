#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>

#include "../../../Algorithme-trilateration/Trilateration.h"

#define ANCHORS_NUMBER 4

struct HubMapEntry {
    int aKey;
    V3 aValue;
};

/**
 * Charge les données depuis la Flash NVS et reconstruit le dictionnaire <int, V3>.
 */
std::unordered_map<int, V3> loadMapData(const char* pNamespaceName, const char* pKeyName, size_t pMaxElementCount);

/**
 * Lit de manière générique n'importe quel type de donnée ou tableau depuis la Flash NVS.
 * @return Le nombre d'éléments (et non d'octets) récupérés avec succès.
 */
template <typename T>
size_t loadData(const char* pNamespaceName, const char* pKeyName, T* pDataDestination, size_t pMaxElementCount = 1);

/**
 * Sauvegarde de manière générique n'importe quel type de donnée ou tableau dans la Flash NVS.
 * @return Le nombre d'octets réellement écrits.
 */
template <typename T>
size_t saveData(const char* pNamespaceName, const char* pKeyName, const T* pData, size_t pElementCount = 1);

/*
Initialise des ancres par défauts qui n'ont pas de positions et renvoie la liste de ces ancres
pNbAnchors - nombre d'ancres à initialiser (valant ANCHORS_NUMBER par défaut)
*/
UWBModuleList initAnchors(const char* pAnchorsNamespace, int pNbAnchors=ANCHORS_NUMBER);

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

/**
 * Convertit et sauvegarde un dictionnaire <int, V3> dans la Flash NVS.
 */
void saveMapData(const char* pNamespaceName, const char* pKeyName, const std::unordered_map<int, V3>& pMap);