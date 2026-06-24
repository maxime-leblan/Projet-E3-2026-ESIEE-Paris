#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>

#include "Trilateration.h"

#define ANCHORS_NUMBER 4
#define SAFEZONE_POINTS_NUMBER 64

struct HubMapEntry {
    int aKey;
    V3 aValue;
};

/**
 * Charge les données depuis la Flash NVS et reconstruit le dictionnaire <int, V3>.
 */
std::map<int, V3> loadMapData(const char* pNamespaceName, const char* pKeyName, size_t pMaxElementCount);

/**
 * Lit de manière générique n'importe quel type de donnée ou tableau depuis la Flash NVS.
 * @return Le nombre d'éléments (et non d'octets) récupérés avec succès.
 */
template <typename T>
size_t loadData(const char* pNamespaceName, const char* pKeyName, T* pDataDestination, size_t pMaxElementCount = 1) {
    Preferences vPrefs;
    
    // Ouverture du namespace en mode lecture seule
    vPrefs.begin(pNamespaceName, true);

    // Récupération de la taille totale stockée en Flash
    size_t vStoredSize = vPrefs.getBytesLength(pKeyName);
    if (vStoredSize == 0) {
        vPrefs.end();
        return 0;
    }

    // Calcul du nombre d'éléments présents en Flash
    size_t vStoredElementCount = vStoredSize / sizeof(T);
    
    // Sécurité pour ne pas dépasser la capacité de la destination
    size_t vElementsToRead = (vStoredElementCount < pMaxElementCount) ? vStoredElementCount : pMaxElementCount;
    size_t vBytesToRead = vElementsToRead * sizeof(T);

    // Lecture des données brutes
    vPrefs.getBytes(pKeyName, reinterpret_cast<uint8_t*>(pDataDestination), vBytesToRead);

    vPrefs.end();
    return vElementsToRead;
}

/**
 * Sauvegarde de manière générique n'importe quel type de donnée ou tableau dans la Flash NVS.
 * @return Le nombre d'octets réellement écrits.
 */
template <typename T>
size_t saveData(const char* pNamespaceName, const char* pKeyName, const T* pData, size_t pElementCount) {
    Preferences vPrefs;
    size_t vTotalSize = pElementCount * sizeof(T);

    // Ouverture du namespace en mode écriture
    vPrefs.begin(pNamespaceName, false);

    // Écriture du bloc de données brutes
    size_t vBytesWritten = vPrefs.putBytes(pKeyName, reinterpret_cast<const uint8_t*>(pData), vTotalSize);

    vPrefs.end();
    return vBytesWritten;
}

/*
Initialise des ancres par défauts qui n'ont pas de positions et renvoie la liste de ces ancres
pAnchorsNamespace - nom du namespace et de la clé permettant d'accéder aux ancres dans la Flash
pNbAnchors - nombre d'ancres à initialiser (valant ANCHORS_NUMBER par défaut)
*/
UWBModuleList initAnchors(const char* pAnchorsNamespace, int pNbAnchors=ANCHORS_NUMBER);

/*
Initialise la zone de sécurité dans le cas où celle-ci est déja existante dans la Flash.
pSafeZoneNamespace - nom du namespace et de la clé permettant d'accéder à la zone de sécurité dans la Flash
pNbSafeZonePoints - nombre de points formant la zone de sécurité
*/
vector<V3> initSafeZone(const char* pSafeZoneNamespace, int pNbSafeZonePoints=SAFEZONE_POINTS_NUMBER);

/*
Affiche des messages dans le moniteur série pour vérifier que la PSRAM est bien active
*/
void initRamHub();

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
void saveMapData(const char* pNamespaceName, const char* pKeyName, const std::map<int, V3>& pMap);