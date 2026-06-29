#include <Arduino.h>
#include <SPI.h>

#define CS_PIN 7 // Ton fil vert/gris

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("\n--- Test Brut SPI (BMP581) ---");
  
  // Initialisation manuelle de la broche CS
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH); // Désactive le capteur par défaut
  
  // Allumage du bus SPI de la carte nRF52840
  SPI.begin();
}

void loop() {
  Serial.println("Recherche du capteur...");
  
  // Le BMP581 communique à 1MHz en Mode 0
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  
  // 1. On active le capteur (CS vers le bas)
  digitalWrite(CS_PIN, LOW); 
  
  // 2. On demande à lire le registre 0x01 (CHIP_ID)
  // En SPI, on met le 1er bit à 1 pour lire -> 0x01 devient 0x81
  SPI.transfer(0x81); 
  
  // 3. Les capteurs Bosch exigent la lecture d'un octet "poubelle" (Dummy byte)
  SPI.transfer(0x00); 
  
  // 4. On lit enfin la vraie réponse du capteur
  byte chip_id = SPI.transfer(0x00); 
  
  // 5. On désactive le capteur (CS vers le haut)
  digitalWrite(CS_PIN, HIGH); 
  SPI.endTransaction();
  
  // Affichage du résultat
  Serial.print("Code d'identification reçu : 0x");
  if (chip_id < 16) Serial.print("0");
  Serial.println(chip_id, HEX);
  
  if (chip_id == 0x50) {
    Serial.println("VICTOIRE ! Le bus SPI matériel fonctionne parfaitement.");
  } else if (chip_id == 0xFF || chip_id == 0x00) {
    Serial.println("ECHEC : Le capteur est physiquement aveugle ou muet.");
  } else {
    Serial.println("ECHEC : La donnée est corrompue (faux contact ?)");
  }
  
  Serial.println("-------------------------");
  delay(3000);
}