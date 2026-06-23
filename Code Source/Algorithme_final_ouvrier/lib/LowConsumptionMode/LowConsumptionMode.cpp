#include "LowConsumptionMode.hpp"

/**
 * Met le module UWB en veille prolongée (Consommation minimale)
 */
void veilleUWB() {
    Serial.println("[UWB] Envoi de la commande de mise en veille...");
    Serial1.println("AT+SLEEP=65535"); // 65535 = Sommeil permanent dans le firmware Makerfabs
    Serial1.flush();                  // On attend que la commande soit totalement envoyée
    delay(50); 
}

/**
 * Réveille le module UWB via une impulsion matérielle sur son UART2 RX
 */
void reveilUWB() {
    Serial.println("[UWB] Envoi de l'interruption de réveil (Drop-down)...");
    
    // Le firmware Makerfabs demande un "drop-down" (passage à l'état BAS) pour se réveiller
    digitalWrite(UWB_WAKEUP_PIN, LOW);
    delay(15); // Une impulsion de 15 ms suffit largement
    digitalWrite(UWB_WAKEUP_PIN, HIGH); // On repasse à l'état HAUT par défaut
    
    delay(100); // Petit temps d'attente pour laisser le microcontrôleur de l'UWB redémarrer
    Serial.println("[UWB] Module UWB réveillé et prêt.");
}


// --- FONCTIONS POUR LA XIAO BLE ---

/**
 * Met la XIAO BLE en sommeil basse consommation (System ON) pendant une durée précise
 * @param duree_ms Durée de la veille en millisecondes
 */
void veilleXiao(uint32_t duree_ms) {
    Serial.println("[XIAO] Entrée en veille pour " + String(duree_ms) +" ms...\n");
    Serial.flush(); // On vide le buffer série pour éviter les corruptions d'affichage
    
    // Crucial pour l'énergie : On désactive le port série matériel de l'UWB
    // car laisser un périphérique UART actif consomme du courant inutilement
    Serial1.end(); 

    // NOTE : Si vous avez votre capteur de pression BMP581 en SPI, 
    // c'est ici qu'il faudrait idéalement appeler une fonction pour le mettre en standby.

    // Sur l'architecture nRF52, le delay() bascule automatiquement 
    // le processeur en mode veille "System ON" via FreeRTOS
    delay(duree_ms); 
}

/**
 * Relance les périphériques de la XIAO après son réveil automatique
 */
void reveilXiao() {
    // On réactive la liaison Série vers le module UWB
    Serial1.begin(115200); 
    
    // NOTE : C'est ici qu'on réveillerait le capteur BMP581 si on l'avait éteint
    
    Serial.println("[XIAO] Mode actif restauré.");
}

bool calculBatteryLow() {
    // On active le MOSFET pour permettre la lecture de la tension de batterie
    digitalWrite(PIN_VBAT_ENABLE, LOW);
    delay(10); 

    // Lecture de la tension de batterie via la broche analogique
    int rawValue = analogRead(32); 
    float voltage = ((rawValue * 3.6) / 4096.0) * 1.51* 2.0 ; // Ajustement pour la tension réelle de la batterie (en tenant compte du diviseur de tension genre en SAH 1.51 c pour une res de 100k et 51k

    // On désactive le MOSFET après la lecture pour économiser l'énergie
    digitalWrite(PIN_VBAT_ENABLE, HIGH);

    // Vérification si la tension est inférieure à un seuil critique (par exemple 3.3V)
    return voltage < 3.5; // Retourne true si la batterie est faible, false sinon
}

void chargebattery() {
// 1. Détection matérielle du chargeur : 
  // On lit le bit 0 du registre USBREGSTATUS du nRF52840 pour savoir si VBUS est alimenté (5V)
  //veilleUWB(); 
  bool isChargerPlugged = (NRF_POWER->USBREGSTATUS & 0x01);
  bool isCharging = (digitalRead(PIN_CHARGE_STATUS) == LOW);

  //static bool uwbAsleepByCharger = false;

  // On ne clignote QUE si le chargeur est branché ET que la puce est en train de charger
  if (isChargerPlugged && isCharging) {

    //if (!uwbAsleepByCharger) {
    //        veilleUWB(); // Appel de votre fonction existante
    //        uwbAsleepByCharger = true;
    //        Serial.println("[CHARGE] UWB mis en veille (Charge en cours).");
    //    }

    static uint32_t lastBlinkTime = 0;
    static bool ledState = false;
    


    if (millis() - lastBlinkTime > 500) { 
      lastBlinkTime = millis();
      ledState = !ledState;
      
      // Logique inversée du XIAO (LOW = Allumé)
      digitalWrite(LED_RED_CARTE, ledState ? LOW : HIGH); 
    }
  } else {
    // Si le chargeur est débranché ou que la charge est 100% terminée, on éteint la LED
    // --- GESTION DE L'UWB : Réveil unique ---
    //    if (uwbAsleepByCharger) {
    //        reveilUWB(); // Appel de votre fonction existante
    //        uwbAsleepByCharger = false;
    //        Serial.println("[CHARGE] Fin de charge ou déconnexion : Réveil de l'UWB.");
    //    }
    digitalWrite(LED_RED_CARTE, HIGH);  
  }
}