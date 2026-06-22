#include <Arduino.h>
#include <Adafruit_TinyUSB.h> // Indispensable pour satisfaire le linker USB du framework nRF52 Adafruit

// ==========================================
// CONFIGURATION MATÉRIELLE ET RÉSEAU
// ==========================================
#define PIN_RESET D0
#define SERIAL_LOG Serial
#define SERIAL_UWB Serial1

// Macro définissant l'ID unique de ce Tag (0 ou 1 pour ton architecture VigiZone)
#define TAG_ID 0

// ==========================================

void sendCommand(const char* cmd) {
    pinMode(LED_GREEN, OUTPUT);
    SERIAL_LOG.print(">> ");
    SERIAL_LOG.println(cmd);
    SERIAL_UWB.println(cmd);
    digitalWrite(LED_GREEN, LOW);
}

void setup() {
    // Initialisation du port USB CDC
    SERIAL_LOG.begin(115200);
    while (!SERIAL_LOG && millis() < 3000);

    SERIAL_LOG.println("\n=== INITIALISATION DU TAG UWB ===");
    SERIAL_LOG.print("ID du Tag compile : ");
    SERIAL_LOG.println(TAG_ID);

    // 1. Reset matériel obligatoire (3 secondes)
    SERIAL_LOG.println("Reset materiel en cours...");
    pinMode(PIN_RESET, OUTPUT);
    digitalWrite(PIN_RESET, LOW);
    delay(3100);
    pinMode(PIN_RESET, INPUT); // Haute impédance
    delay(1000);

    // Initialisation de l'UART matériel vers le DW3000
    SERIAL_UWB.begin(115200);

    // 2. Configuration automatique via les commandes AT
    char cfgCmd[32];
    // Rôle: 0 (Tag), Débit: 1 (6.8M), Filtre: 0 (OFF pour précision brute)
    sprintf(cfgCmd, "AT+SETCFG=%d,0,1,0", TAG_ID); 
    
    sendCommand(cfgCmd);
    sendCommand("AT+SETCAP=2,10,0"); // 2 Tags max, 10ms par slot (Fréquence 50Hz)
    sendCommand("AT+SETPAN=1111");   // Identifiant du réseau privé
    sendCommand("AT+SETRPT=1");      // Activation de l'envoi automatique des distances
    sendCommand("AT+SAVE");          // Écriture définitive en EEPROM/Flash
    sendCommand("AT+RESTART");       // Redémarrage logiciel du DW3000
    delay(1500);

    SERIAL_LOG.println("=== TAG PRET : EN ATTENTE DES ANCRES ===");
}

void loop() {
    // Lecture continue, non-bloquante, du buffer matériel RX
    if (SERIAL_UWB.available()) {
        String ligne = SERIAL_UWB.readStringUntil('\n');
        ligne.trim();

        // Filtrage pour ne traiter que les trames de télémétrie
        if (ligne.startsWith("AT+RANGE")) {
            int tid, r0, r1, r2, r3;
            
            // Parsing C natif (sscanf) : extrêmement rapide et peu gourmand en RAM
            // On extrait l'ID du tag émetteur et les distances brutes vers les ancres 0 à 3
            int matched = sscanf(ligne.c_str(), "AT+RANGE=tid:%d, mask:%*x, seq: %*d, range: (%d,%d,%d,%d", 
                                 &tid, &r0, &r1, &r2, &r3);

            // On s'assure que la trame lue correspond bien à notre Tag
            if (matched == 5 && tid == TAG_ID) {
                SERIAL_LOG.print("[TAG ");
                SERIAL_LOG.print(tid);
                SERIAL_LOG.print("] Distances -> Ancre0: ");
                SERIAL_LOG.print(r0 > 0 ? String(r0) + "cm" : "---");
                SERIAL_LOG.print(" | Ancre1: ");
                SERIAL_LOG.print(r1 > 0 ? String(r1) + "cm" : "---");
                SERIAL_LOG.print(" | Ancre2: ");
                SERIAL_LOG.print(r2 > 0 ? String(r2) + "cm" : "---");
                SERIAL_LOG.print(" | Ancre3: ");
                SERIAL_LOG.println(r3 > 0 ? String(r3) + "cm" : "---");
            }
        }
    }
}