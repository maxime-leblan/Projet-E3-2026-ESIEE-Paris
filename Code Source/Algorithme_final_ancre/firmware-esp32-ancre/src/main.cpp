#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_now.h>
#include <WiFi.h>

#include "Librairie_PlatformIO/lib/MessageManager/CANMessageManager.hpp"

#include "Messages.hpp" // Notre dictionnaire de colis

// --- CONFIGURATION MATÉRIELLE MAKERFABS ---
#define I2C_SDA 39
#define I2C_SCL 38
#define POWER_PIN 42
#define UWB_RX 18
#define UWB_TX 17

TwoWire I2C_OLED = TwoWire(1);
Adafruit_SSD1306 display(128, 64, &I2C_OLED, -1);
HardwareSerial UWBSerial(1);

// --- VARIABLES GLOBALES ---
bool isTagMode = false; // Par défaut, on est une Ancre
MessageAncreHub dataToSend;
esp_now_peer_info_t peerInfo;
int monIdUWB = -1;

// --- PROTOTYPES DES FONCTIONS ---
String sendATCommand(String command, const int timeout);
void updateScreen(String role, String value);
void switchRoleToTag();
void switchRoleToAnchor();

// ==========================================
// CALLBACKS ESP-NOW
// ==========================================

// Quand l'Ancre a fini d'envoyer un message au Hub
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "ESP-NOW: Envoi OK" : "ESP-NOW: Echec");
}

// Quand l'Ancre reçoit un ordre du Hub (pour devenir un Tag)
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    MessageHubAncre receivedCmd;
    memcpy(&receivedCmd, incomingData, sizeof(receivedCmd));
    
    if (receivedCmd.command == 1 && !isTagMode) {
        Serial.println("Ordre du Hub : Passage en mode tag");
        switchRoleToTag();
    } 
    else if (receivedCmd.command == 0 && isTagMode) {
        Serial.println("Ordre du Hub : Retour en mode ancre");
        switchRoleToAnchor();
    } else if (receivedCmd.command == 2) {
        monIdUWB = receivedCmd.uwb_id;

        Serial.printf("Ordre du Hub : Je suis l'ancre %d .\n", monIdUWB);

        switchRoleToAnchor();
    }
}

// ==========================================
// SETUP
// ==========================================
void setup() {
    Serial.begin(115200);
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime < 4000)) delay(10);
    
    Serial.println("\n=== DEMARRAGE ESP32-S3 ===");

    // 1. Alimentation
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    delay(2000);  

    // 2. Initialisation UWB et Ecran
    UWBSerial.begin(115200, SERIAL_8N1, UWB_RX, UWB_TX);
    I2C_OLED.begin(I2C_SDA, I2C_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        while(1) { Serial.println("Erreur OLED"); delay(1000); }
    }

    // 3. INITIALISATION ESP-NOW (Remplacement du CAN)
    WiFi.mode(WIFI_STA); // Mode Station (requis pour ESP-NOW)
    if (esp_now_init() != ESP_OK) {
        Serial.println("Erreur init ESP-NOW");
        updateScreen("ERREUR", "ESP-NOW Fail");
        while(1) delay(100);
    }
    
    // Enregistrement des fonctions callbacks
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    // Ajout du Hub comme "Contact"
    memcpy(peerInfo.peer_addr, HUB_MAC_ADDRESS, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add Hub peer");
    }

    // 4. On attend de voir l'HUB pour faire quoi que cela soit.
    updateScreen("DEMARRAGE", "Recherche du Hub...");

    // 5. Envoi du message d'Init au Hub (Le Hub notera notre adresse MAC)
    dataToSend.type = MSG_INIT;
    esp_now_send(HUB_MAC_ADDRESS, (uint8_t *) &dataToSend, sizeof(dataToSend));
}

// ==========================================
// BOUCLE PRINCIPALE
// ==========================================
void loop() {
    // Ne rien faire tant qu'on a pas reçu son ID.
    if (monIdUWB == -1) {
        return;
    }
    // Écoute des données venant du module UWB
    if (UWBSerial.available()) {
        String data = UWBSerial.readStringUntil('\n');
        data.trim();

        // 1. SI ON EST UNE ANCRE (On lit les distances et on les envoie au Hub)
        if (!isTagMode) {
            // Si la trame contient "AT+RANGE", on appelle notre super fonction
            if (data.indexOf("AT+RANGE") != -1) {
                // Le Parser remplit automatiquement dataToSend avec l'ID et les mètres
                if (parseUWBMessage(data, dataToSend)) {
                    
                    // Petit affichage sur l'OLED pour vérifier
                    String oledText = "Tag:" + String(dataToSend.tag_id) + " | D0:" + String(dataToSend.distances[0], 2) + "m";
                    updateScreen("ANCRE " + String(MessageAncreHub.tag_id) + " (Active)", oledText);

                    // On envoie le paquet au Hub via ESP-NOW
                    esp_now_send(HUB_MAC_ADDRESS, (uint8_t *) &dataToSend, sizeof(dataToSend));
                    // Méthode CAN (pour plus tard)
                }
            }
        }

        
        // 2. SI ON EST UN TAG (On fait notre vie de Tag)
        else {
            int dataIndex = data.indexOf("range:(");
            if (dataIndex != -1) {
                String distanceValue = data.substring(dataIndex + 7);
                updateScreen("TAG (Mobile)", distanceValue);
                // Un Tag normal diffuse déjà en UWB, on a juste à afficher l'écran
            }
        }
    }
}

// ==========================================
// FONCTIONS UWB / ROLES
// ==========================================

void switchRoleToAnchor() {
    isTagMode = false;
    updateScreen("INIT ANCRE " + String(monIdUWB),"Config UWB...");
    sendATCommand("AT+RESTORE", 2000);
    sendATCommand("AT+SETCFG=" + String(monIdUWB) + ",1,0,1", 2000);
    sendATCommand("AT+SETCAP=10,25,1", 2000); 
    sendATCommand("AT+SAVE", 1000);
    sendATCommand("AT+RESTART", 2000);
    updateScreen("ANCRE " + String(monIdUWB), "Attente Tag...");
}

void switchRoleToTag() {
    isTagMode = true;
    updateScreen("INIT TAG", "Config UWB...");
    sendATCommand("AT+RESTORE", 2000);
    sendATCommand("AT+SETCFG=" + String(monIdUWB) + ",0,0,1", 2000); // Role: Tag (0)
    sendATCommand("AT+SETCAP=10,25,1", 2000); 
    sendATCommand("AT+SAVE", 1000);
    sendATCommand("AT+RESTART", 2000);
    updateScreen("TAG", "Calcul...");
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
    Serial.println("AT CMD: " + command + " | REP: " + response);
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
    display.println(value);
    display.display();
}

