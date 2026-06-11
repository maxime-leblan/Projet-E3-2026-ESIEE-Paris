#include "HubDataStorage.hpp"

UWBModuleList initAnchors(int pNbAnchors)
{
    UWBModuleList vAnchors;
    UWBModule vNewAnchor;

    for (int i = 0; i < pNbAnchors; i++)
    {
    vNewAnchor = UWBModule(i);
    vAnchors.addModule(vNewAnchor.getId(), vNewAnchor);
    }

    return vAnchors;
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