#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- CONFIGURATION MATÉRIELLE MAKERFABS ---
#define I2C_SDA 39
#define I2C_SCL 38
#define POWER_PIN 43
#define UWB_RX 16
#define UWB_TX 17

TwoWire I2C_OLED = TwoWire(1);
Adafruit_SSD1306 display(128, 64, &I2C_OLED, -1);
HardwareSerial UWBSerial(1);

// --- FONCTIONS ---
String sendATCommand(String command, const int timeout);
void updateScreen(String role, String value);

void setup() {
    Serial.begin(115200);
    UWBSerial.begin(115200, SERIAL_8N1, UWB_RX, UWB_TX);

    // 1. Alimentation des périphériques (Écran + UWB)
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    delay(200); // Laisse le temps au STM32 de démarrer

    // 2. Initialisation de l'écran
    I2C_OLED.begin(I2C_SDA, I2C_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println("Erreur OLED"); 
        while(1); 
    }
    updateScreen("TAG 0", "Demarrage...");

    // 3. Configuration du module UWB en Tag
    Serial.println("Configuration du Tag UWB...");
    sendATCommand("AT+RESTORE", 2000);
    sendATCommand("AT+SETCFG=0,0,0,1", 2000); // ID:0, Role:Tag(0), Rate:850K(0), Filter:ON(1)
    sendATCommand("AT+SETCAP=10,25,1", 2000); // Mode paquet étendu pour envoyer des données
    sendATCommand("AT+SAVE", 1000);
    sendATCommand("AT+RESTART", 2000);

    updateScreen("TAG 0", "Pret. Recherche...");
}

void loop() {
    // Le module STM32 envoie continuellement la distance quand il trouve une ancre
    if (UWBSerial.available()) {
        String data = UWBSerial.readStringUntil('\n');
        data.trim();

        // Si la trame contient "m" (pour mètres) ou "range:"
        if (data.indexOf("m") != -1) {
            Serial.println("Trame recue : " + data);
            
            // Affichage local sur le Tag
            updateScreen("TAG 0 (Mobile)", data);
            
            // Envoi de la donnée à l'Ancre
            // Format du message envoyé : D:1.25m
            String msg = "D:" + data;
            sendATCommand("AT+DATA=" + String(msg.length()) + "," + msg, 200);
        }
    }
}

String sendATCommand(String command, const int timeout) {
    String response = "";
    UWBSerial.println(command);
    long int time = millis();
    while ((time + timeout) > millis()) {
        while (UWBSerial.available()) {
            response += (char)UWBSerial.read();
        }
    }
    return response;
}

void updateScreen(String role, String value) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println(role);
    display.println("---------------------");
    display.println("");
    display.setTextSize(1); // Mettre à 2 si le texte est trop petit
    display.println(value);
    display.display();
}