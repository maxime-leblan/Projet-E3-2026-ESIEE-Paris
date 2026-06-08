#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- CONFIGURATION MATÉRIELLE MAKERFABS ---
#define I2C_SDA 39
#define I2C_SCL 38
#define POWER_PIN 42 // anciennement 43
#define UWB_RX 18 
#define UWB_TX 17 

TwoWire I2C_OLED = TwoWire(1);
Adafruit_SSD1306 display(128, 64, &I2C_OLED, -1);
HardwareSerial UWBSerial(1);

// --- FONCTIONS ---
String sendATCommand(String command, const int timeout);
void updateScreen(String role, String value);

void setup() {
    Serial.begin(115200); // ou 115200
    
    // --- PROTECTION ESP32-S3 ---
    // On attend max 4 secondes sans rien faire pour laisser le temps au port USB
    // de se stabiliser sur le PC et au moniteur série de s'ouvrir.
    // Attente de la connexion effective du moniteur série du PC (Max 4 secondes)
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime < 4000)) {
        delay(10);
    } 
    
    Serial.println("\n=================================");
    Serial.println("ESP32-S3 : Demarrage du programme...");
    Serial.println("=================================");

    // 1. Alimentation des périphériques
    Serial.println("Etape 1 : Activation de la puissance (POWER_PIN)...");
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    
    // On met un délai de 2 secondes pour laisser le courant se stabiliser 
    // sur l'écran et le module UWB avant de leur parler
    delay(2000); 

    Serial.println("Etape 2 : Initialisation du port serie UWB...");
    UWBSerial.begin(115200, SERIAL_8N1, UWB_RX, UWB_TX);

    // 2. Initialisation de l'écran
    Serial.println("Etape 3 : Tentative de connexion a l'ecran OLED...");
    I2C_OLED.begin(I2C_SDA, I2C_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        // Si ca plante, on ne bloque plus en silence ! On le dit en boucle :
        while(1) {
            Serial.println(">>> ERREUR : Ecran OLED introuvable ! Verifiez l'alimentation.");
            delay(1000);
        }
    }
    
    Serial.println("OLED initialise avec succes !");
    updateScreen("INITIALISATION", "Configuration UWB...");

    // 3. Configuration du module UWB en Tag
    Serial.println("Configuration du Tag UWB...");
    sendATCommand("AT+RESTORE", 2000);
    sendATCommand("AT+SETCFG=1,0,0,1", 2000); // ID:1, Role:Tag(0), Rate:850K(0), Filter:ON(1)
    sendATCommand("AT+SETCAP=10,25,1", 2000); // Mode paquet étendu pour envoyer des données
    sendATCommand("AT+SAVE", 1000);
    sendATCommand("AT+RESTART", 2000);

    updateScreen("TAG 1", "Pret. Recherche...");
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
            updateScreen("TAG 1 (Mobile)", data);
            
            // Envoi de la donnée à l'Ancre
            // Format du message envoyé : D:1.25m
            String msg = "D:" + data;
            sendATCommand("AT+DATA=" + String(msg.length()) + "," + msg, 200);
        }
    }
}

String sendATCommand(String command, const int timeout) {
    String response = "";
    Serial.print("Envoi au module UWB -> "); 
    Serial.println(command); // Affiche la commande envoyée
    
    UWBSerial.println(command);
    long int time = millis();
    while ((time + timeout) > millis()) {
        while (UWBSerial.available()) {
            response += (char)UWBSerial.read();
        }
    }
    
    Serial.print("Réponse du module UWB <- "); 
    Serial.println(response); // Crucial pour voir si le module répond "OK" ou "ERR"
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