#include <Arduino.h>

#define UWB_RST_PIN D0

// --- INVERSION LOGICIELLE DES BROCHES ---
// Dans ton tableau : RX_XIAO = D7, TX_XIAO = D6
// On teste l'inversion : RX_XIAO = D6, TX_XIAO = D7
#define TEST_RX_PIN D6
#define TEST_TX_PIN D7

unsigned long dernierPing = 0;

void setup() {
    Serial.begin(115200);
    while(!Serial) { delay(10); }

    // Libération du Reset
    pinMode(UWB_RST_PIN, OUTPUT);
    digitalWrite(UWB_RST_PIN, HIGH);
    
    // Magie du nRF52840 : On réattribue les broches UART matériellement !
    Serial1.setPins(TEST_RX_PIN, TEST_TX_PIN);
    Serial1.begin(115200); 

    Serial.println("\n=== MODE AUTO-PING (TX/RX INVERSES LOGICIELLEMENT) ===");
    Serial.println("Si le module repond OK, c'est que les pistes etaient inversees sur le PCB !");
    
    // Redémarrage matériel 
    Serial.println("\n[Action] Redemarrage du module UWB via D0...");
    digitalWrite(UWB_RST_PIN, LOW);
    delay(50);
    digitalWrite(UWB_RST_PIN, HIGH);
}

void loop() {
    // Ping automatique toutes les 3 secondes
    if (millis() - dernierPing > 3000) {
        Serial.println("[Xiao] -> AT");
        Serial1.print("AT\r\n"); 
        dernierPing = millis();
    }
    
    // Écoute en continu de la réponse du module UWB
    while (Serial1.available()) {
        char c = Serial1.read();
        Serial.write(c);
    }
}