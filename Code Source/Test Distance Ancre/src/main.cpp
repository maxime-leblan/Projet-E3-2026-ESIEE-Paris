#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuration de l'Ancre (À modifier pour chaque carte : 0, 1, 2, 3)
#define ANCHOR_ID 2

// Définitions matérielles ESP32-S3 Makerfabs 
#define RESET_PIN 16
#define IO_RXD2 18
#define IO_TXD2 17
#define I2C_SDA 39
#define I2C_SCL 38

HardwareSerial SERIAL_AT(2);
Adafruit_SSD1306 display(128, 64, &Wire, -1); 

// Tableau pour stocker la dernière distance connue des 2 Tags
int distancesTags[2] = {-1, -1}; 

void sendCommand(const char* cmd) {
    Serial.println(cmd);
    SERIAL_AT.println(cmd);
    delay(300);
}

void updateOLED() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // En-tête
    display.setCursor(0, 0);
    display.printf("Ancre UWB - ID: %d\n", ANCHOR_ID);
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    // Affichage des distances
    display.setTextSize(2);
    
    display.setCursor(0, 18);
    if (distancesTags[0] > 0) display.printf("T0: %d cm\n", distancesTags[0]);
    else display.printf("T0: -- cm\n");

    display.setCursor(0, 40);
    if (distancesTags[1] > 0) display.printf("T1: %d cm\n", distancesTags[1]);
    else display.printf("T1: -- cm\n");

    display.display();
}

void setup() {
    Serial.begin(115200);
    
    // Initialisation de l'OLED 
    Wire.begin(I2C_SDA, I2C_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("Erreur SSD1306"));
        for (;;);
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println("Demarrage Ancre...");
    display.display();

    // Reset matériel du DW3000 
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW);
    delay(3100);
    pinMode(RESET_PIN, INPUT);
    delay(1000);

    SERIAL_AT.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2); 

    // Configuration automatique
    char cfgCmd[32];
    sprintf(cfgCmd, "AT+SETCFG=%d,1,1,0", ANCHOR_ID); // Anchor, 6.8M, Filtre OFF 
    sendCommand(cfgCmd);
    sendCommand("AT+SETCAP=2,10,1"); // 2 Tags max, 10ms par slot 
    sendCommand("AT+SETPAN=1111");   // Réseau VigiZone 
    sendCommand("AT+SETRPT=1");      // Active l'auto-report des distances 
    sendCommand("AT+SAVE");          // Sauvegarde en Flash 
    sendCommand("AT+RESTART");       // Redémarrage 
    delay(1500);

    updateOLED();
}

void loop() {
    if (SERIAL_AT.available()) {
        String ligne = SERIAL_AT.readStringUntil('\n');
        ligne.trim();

        if (ligne.startsWith("AT+RANGE")) {
            int tid, ranges[4];
            
            int matched = sscanf(ligne.c_str(), "AT+RANGE=tid:%d, mask:%*x, seq: %*d, range: (%d,%d,%d,%d", 
                                 &tid, &ranges[0], &ranges[1], &ranges[2], &ranges[3]);

            if (matched == 5 && tid < 2) { // On s'assure que le Tag ID est 0 ou 1
                // On extrait uniquement la distance qui correspond à NOTRE ID d'ancre
                int maDistance = ranges[ANCHOR_ID];
                
                // On met à jour le tableau si la distance est valide
                if (maDistance > 0) {
                    distancesTags[tid] = maDistance;
                    updateOLED(); // Rafraîchit l'écran avec la nouvelle valeur
                }
            }
        }
    }
}