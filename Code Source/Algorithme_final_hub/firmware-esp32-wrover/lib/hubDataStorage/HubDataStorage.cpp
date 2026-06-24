#include "HubDataStorage.hpp"

std::map<int, V3> loadMapData(const char* pNamespaceName, const char* pKeyName, size_t pMaxElementCount) {
    std::map<int, V3> vResultMap;
    
    // 1. On crée un tableau plat temporaire pour recevoir les octets de la Flash
    std::vector<HubMapEntry> vEntries(pMaxElementCount);
    
    // 2. On appelle la fonction générique précédente pour remplir ce tableau
    size_t vElementsRead = loadData(pNamespaceName, pKeyName, vEntries.data(), pMaxElementCount);
    
    // 3. On reconstruit le dictionnaire à partir du tableau plat
    for (size_t vI = 0; vI < vElementsRead; ++vI) {
        vResultMap[vEntries[vI].aKey] = vEntries[vI].aValue;
    }
    
    return vResultMap;
}

UWBModuleList initAnchors(const char* pAnchorsNamespace, int pNbAnchors)
{
    Preferences vPrefs;
    UWBModuleList vAnchors;
    UWBModule vNewAnchor;

    Serial.println("[EXTRACTION FLASH] Début de l'extraction de données concernant les Ancres du Hub...");

    // Ouvre le namespace en mode Lecture/Écriture
    vPrefs.begin(pAnchorsNamespace, false);

    // Vérifie si la clé témoin existe
    bool vNamespaceExists = vPrefs.isKey("initDone");

    if (!vNamespaceExists) {
        Serial.println("[EXTRACTION FLASH] Namespace Ancre vide ou nouveau. Initialisation...");

        // On attribue aux modules la position (0, 0, 0) par défaut
        for (int i = 0; i < pNbAnchors; i++)
        {
            vNewAnchor = UWBModule(i);
            vAnchors.addModule(vNewAnchor.getId(), vNewAnchor);
        }

        // On enregistre déjà ces positions temporaires dans la Flash au cas où le programme se coupe en plein milieu
        saveMapData(pAnchorsNamespace, pAnchorsNamespace, vAnchors.giveModulePositionList());

        // On confirme qu'on a bien enregistré les positions des ancres
        vPrefs.putBool("initDone", true); // Crée la clé témoin
    }
    else {
        Serial.println("[EXTRACTION FLASH] Namespace Ancre existant. Lecture des données...");
        
        std::map<int, V3> vAnchorsPosition = loadMapData(pAnchorsNamespace, pAnchorsNamespace, ANCHORS_NUMBER);

        for (auto it = vAnchorsPosition.begin(); it != vAnchorsPosition.end(); it++)
        {
            vNewAnchor = UWBModule(it->first, it->second);
            vAnchors.addModule(vNewAnchor.getId(), vNewAnchor);
        }
    }

    vPrefs.end();
    String vText = String((vAnchors.toString()).c_str());
    Serial.println("[EXTRACTION FLASH] Coordonnées des Ancres extraites :\n" + vText);

    return vAnchors;
}

vector<V3> initSafeZone(const char* pSafeZoneNamespace, int pNbSafeZonePoints)
{
    Preferences vPrefs;
    vector<V3> vSafeZone;

    Serial.println("[EXTRACTION FLASH] Début de l'extraction de données concernant la SafeZone du Hub...");

    // Ouvre le namespace en mode Lecture/Écriture
    vPrefs.begin(pSafeZoneNamespace, false);

    // Vérifie si la clé témoin existe
    bool vNamespaceExists = vPrefs.isKey("initDone");

    if (vNamespaceExists)
    {
        Serial.println("[EXTRACTION FLASH] Namespace SafeZone existant. Lecture des données...");

        // On redimensionne le vecteur pour allouer l'espace en mémoire RAM
        vSafeZone.resize(pNbSafeZonePoints);

        // On passe le pointeur vers le premier élément du tableau (.data())
        loadData(pSafeZoneNamespace, pSafeZoneNamespace, vSafeZone.data(), pNbSafeZonePoints);
    }
    else
    {
        Serial.println("[EXTRACTION FLASH] Namespace SafeZone vide ou nouveau. Lancer la calibration de la zone de sécurité " \
            "avant de pouvoir utiliser le prototype.");
        
        // On ajoute 1 unique élément à vSafeZone pour indiquer que la zone de sécurité n'a pas encore été initialisée
        vSafeZone.push_back(V3(0, 0, 0));
    }

    
    Serial.println("[EXTRACTION FLASH] Coordonnées des Ancres extraites :\n");

    return vSafeZone;
}

void initHub()
{
    Serial.println("\n--- TEST ESP32 WROVER COUPLÉ À PLATFORMIO ---");

    // 1. Vérification de la RAM interne classique
    Serial.printf("RAM interne libre : %d octets\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));


    // 2. Vérification de la PSRAM (La force du WROVER)
    if (psramInit()) {
    Serial.println("-> PSRAM détectée et activée avec succès !");
    Serial.printf("PSRAM totale : %d octets\n", ESP.getPsramSize());
    Serial.printf("PSRAM libre  : %d octets\n", ESP.getFreePsram());
    } else {
    Serial.println("-> Échec de l'activation de la PSRAM. Vérifie tes build_flags.");
    }
}

void resetFlash()
{
    nvs_flash_erase(); // efface la partition NVS
    nvs_flash_init(); // initialise la partition NVS
    while(true); // boucle infinie pour empêcher le programme principale de faire autre chose ensuite
}

void saveMapData(const char* pNamespaceName, const char* pKeyName, const std::map<int, V3>& pMap) {
    // 1. On crée un tableau dynamique temporaire (contigu en mémoire)
    std::vector<HubMapEntry> vEntries;
    vEntries.reserve(pMap.size());

    // 2. On extrait les données du dictionnaire pour les mettre à plat
    for (const auto& vPair : pMap) {
        HubMapEntry vEntry;
        vEntry.aKey = vPair.first;
        vEntry.aValue = vPair.second;
        vEntries.push_back(vEntry);
    }

    // 3. On appelle la fonction générique précédente avec notre tableau plat
    saveData(pNamespaceName, pKeyName, vEntries.data(), vEntries.size());
}