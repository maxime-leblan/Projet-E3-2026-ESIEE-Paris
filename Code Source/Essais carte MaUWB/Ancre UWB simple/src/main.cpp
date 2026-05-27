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

    // 1. Alimentation des périphériques
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    delay(200);

    // 2. Initialisation de l'écran
    I2C_OLED.begin(I2C_SDA, I2C_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println("Erreur OLED"); 
        while(1); 
    }
    updateScreen("ANCRE 0", "Demarrage...");

    // 3. Configuration du module UWB en Ancre
    Serial.println("Configuration de l'Ancre UWB...");
    sendATCommand("AT+RESTORE", 2000);
    sendATCommand("AT+SETCFG=0,1,0,1", 2000); // ID:0, Role:Anchor(1), Rate:850K(0), Filter:ON(1)
    sendATCommand("AT+SETCAP=10,25,1", 2000); // Mode paquet étendu
    sendATCommand("AT+SAVE", 1000);
    sendATCommand("AT+RESTART", 2000);

    updateScreen("ANCRE 0", "En attente du Tag...");
}

void loop() {
    if (UWBSerial.available()) {
        String data = UWBSerial.readStringUntil('\n');
        data.trim();

        // Quand l'Ancre reçoit une donnée radio, elle l'imprime sous la forme :
        // AT+RDATA=...,...,...,...,D:1.25m
        int dataIndex = data.indexOf("D:");
        if (dataIndex != -1) {
            // On extrait tout ce qui se trouve après "D:"
            String distanceValue = data.substring(dataIndex + 2);
            Serial.println("Distance reçue : " + distanceValue);
            
            // Affichage local sur l'Ancre
            updateScreen("ANCRE 0 (Fixe)", distanceValue);
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
    display.setTextSize(1); 
    display.println(value);
    display.display();
}