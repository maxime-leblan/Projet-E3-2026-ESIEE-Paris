#include "OLEDManager.hpp"

TwoWire I2C_OLED_BUS = TwoWire(1);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED_BUS, OLED_RESET);

// État par défaut (l'ID de l'ancre commence à -1)
ScreenState currentState = {-1, "DEMARRAGE", "En attente...", -1, {0, 0, 0, 0}};

void initOLED(int pAnchorId) {
    currentState.anchorId = pAnchorId; // On mémorise qui l'on est
    
    I2C_OLED_BUS.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        while(1) {
            Serial.println(">>> ERREUR : Ecran OLED introuvable ! Verifiez l'alimentation.");
            delay(1000);
        }
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
}

void updateCANAction(String title, String details) {
    currentState.canActionTitle = title;
    currentState.canActionDetails = details;
    refreshScreen();
}

void updateUWBData(int tagId, uint16_t d0, uint16_t d1, uint16_t d2, uint16_t d3) {
    currentState.currentTagId = tagId;
    currentState.distances[0] = d0;
    currentState.distances[1] = d1;
    currentState.distances[2] = d2;
    currentState.distances[3] = d3;
    refreshScreen();
}

void refreshScreen() {
    // --- LIMITEUR DE FPS ---
    static unsigned long lastRefresh = 0;
    // Si moins de 100 ms se sont écoulées depuis le dernier dessin, on abandonne
    if (millis() - lastRefresh < 100) {
        return; 
    }
    lastRefresh = millis(); // On valide qu'on va dessiner

    display.clearDisplay();

    // ==========================================
    // ZONE HAUTE : ÉVÈNEMENTS CAN BUS / SYSTÈME
    // ==========================================
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(currentState.canActionTitle);
    
    display.setCursor(0, 10);
    display.print(currentState.canActionDetails);

    // Ligne de séparation horizontale
    display.drawLine(0, 20, 128, 20, SSD1306_WHITE);

    // ==========================================
    // ZONE BASSE : TÉLÉMÉTRIE UWB
    // ==========================================
    display.setCursor(0, 26);
    
    if (currentState.anchorId != -1) {
        display.printf("ANCRE %d | ", currentState.anchorId);
    }
    
    if (currentState.currentTagId != -1) {
        display.printf("Tag %d", currentState.currentTagId);
    } else {
        display.print("Attente");
    }

    char buffer[64];
    
    display.setCursor(0, 40);
    snprintf(buffer, sizeof(buffer), "A0:%4d   A1:%4d", currentState.distances[0], currentState.distances[1]);
    display.print(buffer);

    display.setCursor(0, 52);
    snprintf(buffer, sizeof(buffer), "A2:%4d   A3:%4d", currentState.distances[2], currentState.distances[3]);
    display.print(buffer);

    display.display(); // Cette fonction lourde n'est appelée que 10 fois par seconde maximum
}