#include "UWBModuleActions.hpp"

float readDistanceFromUWB(int pModuleId, Stream & pUWBSerial)
{
    // 1. Demander au module UWB de calculer la distance
    // La commande AT+RANGE retourne une chaine de caractères complexe
    String response;
    receiveUWBDistanceMessage(pUWBSerial, response);
    std::string vStdStrResponse = std::string(response.c_str(), response.length());

    // 2. Utiliser votre fonction existante pour extraire la distance
    // Note : Supposons que getDistanceFromAnchor prenne la réponse brute 
    // et l'ID cible pour isoler la bonne valeur dans la liste
    float distance = getDistanceFromAnchor(getDataFromString(vStdStrResponse, TAG_DATA_REGEX), pModuleId);

    if (distance <= 0.0f) {
        Serial.printf("[UWB] Erreur : Aucune distance valide obtenue pour l'ancre %d\n", pModuleId);
    }

    return distance;
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

void initUWBModule(int pAnchorId)
{
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

    Serial.println("Configuration de l'Ancre UWB...");
    sendATCommand("AT+RESTORE", 2000);
    sendATCommand("AT+SETCFG=" + String(pAnchorId) + ",1,0,1", 2000); // ID:0, Role:Ancre(1), Rate:850K(0), Filter:ON(1)
    sendATCommand("AT+SETCAP=10,25,1", 2000); // Mode paquet étendu pour envoyer des données
    sendATCommand("AT+SAVE", 1000);
    sendATCommand("AT+RESTART", 2000);

    updateScreen("ANCRE 0", "En attente du Tag...");
}

void toggleUWBMode(int pAnchorId)
{
    Serial.println("\n[UWB] Tentative d'inversion du mode (Tag <-> Ancre)...");

    // 1. Demander la configuration actuelle au module
    String currentCfg = sendATCommand("AT+GETCFG?", 1000);
    
    int currentRole = 1; 
    int currentRate = 0; 
    int currentFilter = 1; 

    // 2. Analyser la réponse
    int index = currentCfg.indexOf("AT+GETCFG=");
    if (index != -1) {
        String dataPart = currentCfg.substring(index + 10); 
        dataPart.trim();

        int firstComma = dataPart.indexOf(',');
        int secondComma = dataPart.indexOf(',', firstComma + 1);
        int thirdComma = dataPart.indexOf(',', secondComma + 1);

        if (firstComma != -1 && secondComma != -1) {
            String roleStr = dataPart.substring(firstComma + 1, secondComma);
            currentRole = roleStr.toInt();
            
            if (thirdComma != -1) {
                currentRate = dataPart.substring(secondComma + 1, thirdComma).toInt();
                currentFilter = dataPart.substring(thirdComma + 1).toInt();
            }
        }
    }

    // 3. Déterminer le nouveau rôle
    int newRole = (currentRole == 1) ? 0 : 1;
    
    if (newRole == 0) {
        Serial.printf("[UWB] Passage de Ancre vers mode TAG (ID: %d)\n", pAnchorId);
        updateScreen("CONFIG", "Mode: TAG");
    } else {
        Serial.printf("[UWB] Passage de Tag vers mode ANCRE (ID: %d)\n", pAnchorId);
        updateScreen("CONFIG", "Mode: ANCRE");
    }

    // 4. Appliquer la nouvelle configuration, sauvegarder et redémarrer
    String setCommand = "AT+SETCFG=" + String(pAnchorId) + "," + String(newRole) + "," + String(currentRate) + "," + String(currentFilter);
    sendATCommand(setCommand, 1500);
    sendATCommand("AT+SAVE", 1000);

    // Vider le tampon série de l'UWB avant le redémarrage pour nettoyer les résidus
    while(UWBSerial.available()) { UWBSerial.read(); }

    // Envoi de l'ordre de redémarrage
    UWBSerial.println("AT+RESTART");
    Serial.println("[UWB] Commande AT+RESTART envoyee. Attente du redemarrage materiel...");

    // =========================================================================
    // OPTIMISATION : ATTENTE ACTIVE NON-BLOQUANTE DU REDÉMARRAGE (TIMEOUT)
    // =========================================================================
    unsigned long startRestartTime = millis();
    const unsigned long restartTimeout = 3000; // Sécurité : max 3 secondes d'attente
    bool bootLogDetected = false;
    String bootResponse = "";

    while (millis() - startRestartTime < restartTimeout) 
    {
        // On lit le flux série au fur et à mesure que le STM32 redémarre
        while (UWBSerial.available()) 
        {
            char c = UWBSerial.read();
            bootResponse += c;
            bootLogDetected = true; // Des données de boot arrivent !
            startRestartTime = millis(); // On réinitialise le timeout tant que le module parle
        }

        // Si le module a commencé à envoyer son bios/boot log, puis s'est tu 
        // pendant plus de 200ms, c'est qu'il a fini sa phase de boot et est prêt.
        if (bootLogDetected && (millis() - startRestartTime > 200)) 
        {
            Serial.println("[UWB] Module stable détecté !");
            break; 
        }

        // Optionnel : Permet de laisser l'ESP32 gérer d'autres tâches légères du système si nécessaire
        yield(); 
    }

    // Affichage de contrôle pour le débogage de la phase de boot
    Serial.println("--- Début du log de redémarrage UWB ---");
    Serial.println(bootResponse);
    Serial.println("--- Fin du log de redémarrage UWB ---");
    
    Serial.println("[UWB] Changement de mode effectif et fonctionnel.\n");
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