#include <Arduino.h>

#define UWB_RST_PIN D0 // Ta broche de Reset

void setup() {
    // 1. Initialisation de l'USB vers le PC
    Serial.begin(115200);
    while(!Serial) { delay(10); } // Attente de l'ouverture du moniteur série

    // 2. Libération du Reset du module UWB (CRUCIAL)
    pinMode(UWB_RST_PIN, OUTPUT);
    digitalWrite(UWB_RST_PIN, HIGH); // On le maintient HAUT pour laisser la STM32 tourner
    
    // 3. Initialisation de l'UART matériel vers l'UWB (D6 et D7)
    Serial1.begin(115200); 

    Serial.println("=== PONT SERIE XIAO <-> UWB PRET ===");
    Serial.println("1. Reglez le moniteur serie sur 'Les deux, NL et CR' (Both NL & CR)");
    Serial.println("2. Tapez 'AT' et appuyez sur Entree.");
    
    // Redémarrage matériel forcé pour essayer de capter le log de boot de la STM32
    Serial.println("\n[Action] Redemarrage du module UWB via D0...");
    digitalWrite(UWB_RST_PIN, LOW);
    delay(50);
    digitalWrite(UWB_RST_PIN, HIGH);
}

void loop() {
    // Si tu tapes quelque chose sur ton PC, la Xiao l'envoie à l'UWB
    if (Serial.available()) {
        char c = Serial.read();
        Serial1.write(c);
    }
    
    // Si l'UWB parle, la Xiao l'affiche sur ton PC
    if (Serial1.available()) {
        char c = Serial1.read();
        Serial.write(c);
    }
}