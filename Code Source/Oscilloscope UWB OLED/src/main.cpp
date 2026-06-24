#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- CONFIGURATION MATÉRIELLE MAKERFABS ESP32-S3 ---
#define POWER_PIN 42         
#define RESET_WAKEUP_PIN 16  
#define UWB_RX 18            
#define UWB_TX 17            
#define I2C_SDA 39           
#define I2C_SCL 38           

// --- OBJETS GLOBAUX ---
HardwareSerial UWBSerial(2); // UART2 d'usine Makerfabs
TwoWire I2C_OLED_BUS = TwoWire(1);
Adafruit_SSD1306 display(128, 64, &I2C_OLED_BUS, -1);

// --- PROTO-FONCTIONS ---
void executeHardReset();
void updateOLED(String l1, String l2);

void setup() {
    // 1. Liaison USB-PC
    Serial.begin(115200);
    
    // Attente optionnelle mais propre pour ouvrir le moniteur série
    uint32_t startTimer = millis();
    while (!Serial && (millis() - startTimer < 2000)) { delay(10); }

    Serial.println("\n[SETUP] ===================================");
    Serial.println("[SETUP] Initialisation du systeme...");

    // 2. Initialisation OLED
    I2C_OLED_BUS.begin(I2C_SDA, I2C_SCL);
    if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        updateOLED("PONT SERIE", "Initialisation...");
    }

    // 3. Alimentation logique du module UWB
    Serial.println("[SETUP] Activation POWER_PIN (Pin 42)...");
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    delay(100);

    // 4. Configuration de l'UART UWB (Buffer étendu à 2048 octets)
    UWBSerial.setRxBufferSize(2048);
    UWBSerial.begin(115200, SERIAL_8N1, UWB_RX, UWB_TX);
    
    Serial.println("[SETUP] UART2 UWB configure a 115200 bauds.");
    Serial.println("[SETUP] ===================================\n");

    // Lancement du Hard Reset initial pour réveiller le module et afficher son Boot Log
    executeHardReset();
}

void loop() {
    // Tampon statique local pour accumuler la commande du PC sans bloquer la loop
    static String pcCommandBuffer = "";

    // =================================================================
    // BOUCLE 1 : PC -> UWB (Lecture non-bloquante flux continu)
    // =================================================================
    while (Serial.available() > 0) {
        char cPC = (char)Serial.read();
        
        // On stocke le caractère pour analyser l'ordre de reset
        if (cPC != '\r' && cPC != '\n') {
            pcCommandBuffer += cPC;
        }

        // On envoie INSTANTANÉMENT le caractère brut au module UWB
        UWBSerial.write(cPC);

        // Si on détecte une fin de ligne, on analyse le buffer accumulé
        if (cPC == '\n' || cPC == '\r') {
            pcCommandBuffer.trim();
            
            if (pcCommandBuffer.equalsIgnoreCase("reset")) {
                executeHardReset();
            }
            
            pcCommandBuffer = ""; // On vide le tampon pour la prochaine commande
        }
    }

    // =================================================================
    // BOUCLE 2 : UWB -> PC (Lecture non-bloquante flux continu)
    // =================================================================
    while (UWBSerial.available() > 0) {
        char cUWB = (char)UWBSerial.read();
        
        // On recrache immédiatement le caractère brut sur l'USB du PC
        Serial.write(cUWB);
    }
}

// =================================================================
// GESTION DU HARD RESET MATÉRIEL EXAUSTIF
// =================================================================
void executeHardReset() {
    Serial.println("\n[HARD RESET] >>> DEBUT DU PROTOCOLE MATERIEL (3 SECONDES) <<<");
    updateOLED("SYSTEME", "Reset en cours...");

    // 1. Mise à la masse stricte de la broche 11 du module pendant 3s
    pinMode(RESET_WAKEUP_PIN, OUTPUT);
    digitalWrite(RESET_WAKEUP_PIN, LOW);
    
    for (int i = 3; i > 0; i--) {
        Serial.printf("[HARD RESET] Broche active a l'etat BAS... %d\n", i);
        delay(1000);
    }

    // 2. CORRECTION CRITIQUE : Passage à l'état HAUT obligatoire pour lancer le processeur UWB
    digitalWrite(RESET_WAKEUP_PIN, HIGH);
    Serial.println("[HARD RESET] Broche passee a l'etat HAUT. Initialisation de l'antenne...");

    // 3. Attente de stabilisation électrique de l'antenne
    delay(500);

    Serial.println("[HARD RESET] Module operationnel. Ecoute du port serie ouverte...\n");
    updateOLED("PONT SERIE", "Pret. Tapez AT?");
    
    // Purge des éventuels résidus électriques
    while (UWBSerial.available()) { UWBSerial.read(); }
    while (Serial.available()) { Serial.read(); }
}

void updateOLED(String ligne1, String ligne2) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("--- " + ligne1 + " ---");
    
    display.drawLine(0, 12, 128, 12, SSD1306_WHITE);
    
    display.setCursor(0, 25);
    display.print(ligne2);
    
    display.display();
}