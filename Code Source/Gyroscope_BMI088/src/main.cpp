#include <Arduino.h>
#include "BMI088.h"

/* --- Configuration du câblage VSPI --- */
const int SPI_SCK  = 18;
const int SPI_MISO = 19;
const int SPI_MOSI = 23;
const int CS_ACCEL = 26;
const int CS_GYRO  = 22;

/* 
 * Instanciation des objets pour l'accéléromètre et le gyroscope.
 * On passe l'objet matériel 'SPI' (qui correspond au VSPI natif sur l'ESP32) 
 * et la broche Chip Select correspondante.
 */
Bmi088Accel accel(SPI, CS_ACCEL);
Bmi088Gyro gyro(SPI, CS_GYRO);

void setup() {
  // Initialisation du moniteur série
  Serial.begin(115200);
  while(!Serial) {} 
  delay(1000); // Laisse le temps à la console série de s'ouvrir
  
  Serial.println("--- Démarrage du test BMI088 ---");

  // Démarrage explicite du bus SPI avec tes broches
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CS_ACCEL);

  // Initialisation de la puce Accéléromètre
  int statusAccel = accel.begin();
  if (statusAccel < 0) {
    Serial.print("Erreur fatale : Accéléromètre non détecté. Code erreur: ");
    Serial.println(statusAccel);
    while(1); // Blocage du système en cas d'échec
  }
  Serial.println("Accéléromètre initialisé.");

  // Initialisation de la puce Gyroscope
  int statusGyro = gyro.begin();
  if (statusGyro < 0) {
    Serial.print("Erreur fatale : Gyroscope non détecté. Code erreur: ");
    Serial.println(statusGyro);
    while(1); // Blocage du système en cas d'échec
  }
  Serial.println("Gyroscope initialisé.");
  Serial.println("---------------------------------");
}

void loop() {
  // 1. Demande de lecture synchrone sur le bus SPI
  accel.readSensor();
  gyro.readSensor();

  // 2. Récupération et formatage des données
  // L'accélération est renvoyée en m/s^2 et la rotation en rad/s
  Serial.print("ACCEL (m/s^2) | X: ");
  Serial.print(accel.getAccelX_mss(), 2);
  Serial.print(" \tY: ");
  Serial.print(accel.getAccelY_mss(), 2);
  Serial.print(" \tZ: ");
  Serial.print(accel.getAccelZ_mss(), 2);

  Serial.print("   ||   GYRO (rad/s) | X: ");
  Serial.print(gyro.getGyroX_rads(), 2);
  Serial.print(" \tY: ");
  Serial.print(gyro.getGyroY_rads(), 2);
  Serial.print(" \tZ: ");
  Serial.println(gyro.getGyroZ_rads(), 2);

  // 3. Temporisation : 100ms pour ne pas saturer l'affichage série (~10 Hz)
  delay(100); 
}