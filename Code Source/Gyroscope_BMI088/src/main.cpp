#include <Arduino.h>
#include "BMI088.h"
#include "Fusion.h" // Inclut automatiquement tous les autres en-têtes

/* --- Configuration Matérielle VSPI --- */
const int SPI_SCK  = 18;
const int SPI_MISO = 19;
const int SPI_MOSI = 23;
const int CS_ACCEL = 26;
const int CS_GYRO  = 22;

Bmi088Accel accel(SPI, CS_ACCEL);
Bmi088Gyro gyro(SPI, CS_GYRO);

/* --- Objets de la bibliothèque Fusion --- */
FusionBias bias; // Structure pour compenser la dérive du gyroscope
FusionAhrs ahrs; // Structure principale de l'algorithme AHRS

// Chronométrage pour l'intégration mathématique
unsigned long previousTime;

void setup() {
  Serial.begin(115200);
  while(!Serial) {} 
  delay(1000);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CS_ACCEL);

  if (accel.begin() < 0 || gyro.begin() < 0) {
    Serial.println("Erreur d'initialisation du BMI088");
    while(1);
  }

  // Initialisation des structures avec les paramètres par défaut
  FusionBiasInitialise(&bias);
  FusionAhrsInitialise(&ahrs);

  previousTime = micros();
  Serial.println("--- Filtre AHRS Démarré ---");
}

void loop() {
  // 1. Calcul précis du temps écoulé (dt) en secondes
  unsigned long currentTime = micros();
  float deltaTime = (float)(currentTime - previousTime) / 1000000.0f;
  previousTime = currentTime;

  // 2. Lecture des registres matériels
  accel.readSensor();
  gyro.readSensor();

  // 3. Formatage des données dans les structures FusionVector
  // Remplissage explicite via la sous-structure 'axis' définie dans FusionMath.h
  FusionVector gyroscope = {
    .axis = {
        .x = gyro.getGyroX_rads() * 57.2958f, // rad/s -> deg/s
        .y = gyro.getGyroY_rads() * 57.2958f,
        .z = gyro.getGyroZ_rads() * 57.2958f
    }
  };

  FusionVector accelerometer = {
    .axis = {
        .x = accel.getAccelX_mss() / 9.81f, // m/s^2 -> g
        .y = accel.getAccelY_mss() / 9.81f,
        .z = accel.getAccelZ_mss() / 9.81f
    }
  };

  // 4. Application de l'algorithme d'étalonnage dynamique continu
  gyroscope = FusionBiasUpdate(&bias, gyroscope);

  // 5. Mise à jour de l'orientation spatiale (sans magnétomètre)
  FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, deltaTime);

  // 6. Extraction directe du vecteur Gravité
  FusionVector gravity = FusionAhrsGetGravity(&ahrs);

  // 7. Affichage sur le moniteur série
  Serial.print("Vecteur Gravite | X: ");
  Serial.print(gravity.axis.x, 3);
  Serial.print(" \tY: ");
  Serial.print(gravity.axis.y, 3);
  Serial.print(" \tZ: ");
  Serial.println(gravity.axis.z, 3);

  delay(10); // Limite la boucle à ~100Hz
}