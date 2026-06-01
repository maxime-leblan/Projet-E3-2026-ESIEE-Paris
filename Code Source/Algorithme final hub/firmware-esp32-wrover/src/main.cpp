#include <Arduino.h>

void setup() {
  // Initialisation du port série à la vitesse configurée dans platformio.ini
  Serial.begin(115200);
  delay(2000); 
  
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

void loop() {
  Serial.println("L'ESP32 WROVER communique parfaitement avec PlatformIO.");
  delay(5000); // Attend 5 secondes
}