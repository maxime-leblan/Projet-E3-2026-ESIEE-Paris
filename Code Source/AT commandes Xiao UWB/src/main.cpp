#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

// Définition des broches pour la XIAO nRF52840
#define PIN_RESET D0 
#define SERIAL_LOG Serial 
#define SERIAL_UWB Serial1 

void setup() {
    pinMode(LED_GREEN, OUTPUT);
    digitalWrite(LED_GREEN, LOW); // Allumé pour montrer que la carte tourne

    // Initialisation de l'USB
    SERIAL_LOG.begin(115200);
    while (!SERIAL_LOG && millis() < 3000); 

    SERIAL_LOG.println("\n--- PONT SERIE UWB ACTIF ---");

    // Reset matériel du module UWB pour le démarrer proprement
    SERIAL_LOG.println("Reset du module UWB (3s)...");
    pinMode(PIN_RESET, OUTPUT);
    digitalWrite(PIN_RESET, LOW); 
    delay(3100); 
    pinMode(PIN_RESET, INPUT); 
    delay(1000); 

    SERIAL_LOG.println("Module pret. Tape tes commandes AT !");

    // Initialisation de l'UART UWB
    SERIAL_UWB.begin(115200);
}

void loop() {
    // 1. Lecture des commandes depuis ton PC -> Envoi au module UWB
    if (SERIAL_LOG.available() > 0) {
        String cmd = SERIAL_LOG.readStringUntil('\n');
        cmd.trim(); 
        
        if (cmd.length() > 0) {
            SERIAL_LOG.print(">> ");
            SERIAL_LOG.println(cmd);
            SERIAL_UWB.println(cmd);
        }
    }

    // 2. Lecture des réponses du module UWB -> Affichage sur ton PC
    if (SERIAL_UWB.available() > 0) {
        SERIAL_LOG.write(SERIAL_UWB.read());
    }
}