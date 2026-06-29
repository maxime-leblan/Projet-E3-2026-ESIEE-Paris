#include "LowConsumptionMode.hpp"

// ====================================================================
// GESTION UWB
// ====================================================================

void veilleUWB() {
    Serial.println("[UWB] Envoi de la commande de mise en veille...");
    Serial1.println("AT+SLEEP=65535"); // 65535 = Sommeil permanent dans le firmware Makerfabs
    Serial1.flush();                  // On attend que la commande soit totalement envoyée
    delay(50); 
}

void reveilUWB() {
    Serial.println("[UWB] Envoi de l'interruption de réveil (Drop-down)...");
    
    // Le firmware Makerfabs demande un "drop-down" (passage à l'état BAS) pour se réveiller
    digitalWrite(UWB_WAKEUP_PIN, LOW);
    delay(15); // Une impulsion de 15 ms suffit largement
    digitalWrite(UWB_WAKEUP_PIN, HIGH); // On repasse à l'état HAUT par défaut
    
    delay(100); // Petit temps d'attente pour laisser le microcontrôleur de l'UWB redémarrer
    Serial.println("[UWB] Module UWB réveillé et prêt.");
}

// ====================================================================
// GESTION XIAO BLE
// ====================================================================

void veilleXiao(uint32_t duree_ms) {
    Serial.println("[XIAO] Entrée en veille pour " + String(duree_ms) +" ms...\n");
    Serial.flush(); 
    
    Serial1.end(); 
    delay(duree_ms); 
}

void reveilXiao() {
    Serial1.begin(115200); 
    Serial.println("[XIAO] Mode actif restauré.");
}

// ====================================================================
// GESTION DE L'ALIMENTATION ET BATTERIE (BARE-METAL NRF52)
// ====================================================================

void initialiserLectureBatterie() {
    // On configure la broche matérielle P0.14 en sortie
    NRF_P0->DIRSET = (1 << PHYSICAL_PIN_VBAT_ENABLE);
    // On la met à l'état HAUT par défaut pour couper le pont diviseur (économie d'énergie)
    NRF_P0->OUTSET = (1 << PHYSICAL_PIN_VBAT_ENABLE);
    
    // Configuration de la résolution analogique à 12 bits
    analogReadResolution(12);
}

void activerChargeRapide100mA() {
    // Configuration de la broche P0.13 en sortie
    NRF_P0->DIRSET = (1 << PHYSICAL_PIN_CHARGE_SPEED);
    // On la met à l'état BAS pour activer la seconde résistance du chargeur BQ25101
    NRF_P0->OUTCLR = (1 << PHYSICAL_PIN_CHARGE_SPEED);
}

void activerChargeNormale50mA() {
    // Pour revenir à 50mA, on passe la broche P0.13 en entrée (Haute Impédance / Flottante).
    // Le DIRCLR (Direction Clear) déconnecte la résistance interne de la broche vers la masse.
    NRF_P0->DIRCLR = (1 << PHYSICAL_PIN_CHARGE_SPEED);
}

bool calculBatteryLow() {
    // 1. On active le MOSFET pour permettre la lecture (Mise à LOW de P0.14)
    NRF_P0->OUTCLR = (1 << PHYSICAL_PIN_VBAT_ENABLE);
    delay(10); // Laisse le temps à la tension de se stabiliser

    // 2. Lecture de la tension de batterie via la broche analogique
    int rawValue = analogRead(ANALOG_VBAT_READ_PIN); 
    
    // Calcul selon le pont diviseur matériel de la carte XIAO BLE
    float voltage = ((rawValue * 3.6) / 4096.0) * 1.51 * 2.0; 

    // 3. On désactive le MOSFET après la lecture pour économiser l'énergie (Mise à HIGH de P0.14)
    NRF_P0->OUTSET = (1 << PHYSICAL_PIN_VBAT_ENABLE);

    // Vérification si la tension est inférieure à un seuil critique
    return voltage < 3.5 ; 
}