#include <Arduino.h>
#include "BMI088.h"
#include "gravity_IMU.h"

/* --- Configuration Matérielle VSPI (Privée) --- */
static const int SPI_SCK  = 18;
static const int SPI_MISO = 19;
static const int SPI_MOSI = 23;
static const int CS_ACCEL = 26;
static const int CS_GYRO  = 22;

static Bmi088Accel accel(SPI, CS_ACCEL);
static Bmi088Gyro gyro(SPI, CS_GYRO);

/* --- Variables Partagées de l'IMU (Privées) --- */
static SemaphoreHandle_t xGravityMutex = NULL;
static FusionVector sharedGravity = FUSION_VECTOR_ZERO;

/* --- Variables d'Étalonnage (Privées) --- */
static volatile bool requestTare = false;
static volatile bool requestReset = false;
static bool isTared = false;
static float R[3][3] = { {1,0,0}, {0,1,0}, {0,0,1} }; // Matrice Identité par défaut

/* --- La Tâche FreeRTOS (Privée) --- */
static void IMUTask(void *pvParameters) {
  FusionBias bias;
  FusionAhrs ahrs;
  FusionBiasInitialise(&bias);
  FusionAhrsInitialise(&ahrs);

  const unsigned long periodUs = 2500; // 400 Hz
  unsigned long previousTime = micros();
  unsigned long nextWakeTimeUs = previousTime + periodUs;
  int watchdogCounter = 0;

  while (true) {
    unsigned long currentTime = micros();
    float deltaTime = (float)(currentTime - previousTime) / 1000000.0f;
    previousTime = currentTime;

    accel.readSensor();
    gyro.readSensor();

    FusionVector gyroscope = {
      .axis = {
          .x = gyro.getGyroX_rads() * 57.2958f,
          .y = gyro.getGyroY_rads() * 57.2958f,
          .z = gyro.getGyroZ_rads() * 57.2958f
      }
    };

    FusionVector accelerometer = {
      .axis = {
          .x = accel.getAccelX_mss() / 9.81f,
          .y = accel.getAccelY_mss() / 9.81f,
          .z = accel.getAccelZ_mss() / 9.81f
      }
    };

    gyroscope = FusionBiasUpdate(&bias, gyroscope);
    FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, deltaTime);
    FusionVector localGravity = FusionAhrsGetGravity(&ahrs);

    // --- GESTION DU TARE (CALCUL MATRICIEL) ---
    if (requestTare) {
      float ax = localGravity.axis.x;
      float ay = localGravity.axis.y;
      float az = localGravity.axis.z;

      // Objectif : Aligner le vecteur (ax, ay, az) avec (0, 0, -1)
      float c = -az; // Cosinus de l'angle (Produit scalaire avec l'axe Z négatif)
      
      // Contre-argument physique : singularité si le capteur est fixé parfaitement à l'envers
      if (c > -0.99f) { 
        float vx = -ay;
        float vy = ax;
        float h = 1.0f / (1.0f + c);

        // Remplissage de la matrice de rotation de Rodrigues
        R[0][0] = c + h * vx * vx;  R[0][1] = h * vx * vy;      R[0][2] = vy;
        R[1][0] = h * vx * vy;      R[1][1] = c + h * vy * vy;  R[1][2] = -vx;
        R[2][0] = -vy;              R[2][1] = vx;               R[2][2] = c;
      } else {
        // Retournement manuel 180° autour de X
        R[0][0] = 1; R[0][1] = 0;  R[0][2] = 0;
        R[1][0] = 0; R[1][1] = -1; R[1][2] = 0;
        R[2][0] = 0; R[2][1] = 0;  R[2][2] = -1;
      }
      isTared = true;
      requestTare = false;
    } 
    else if (requestReset) {
      isTared = false;
      requestReset = false;
    }

    // --- APPLICATION DE LA MATRICE (PROJECTION) ---
    FusionVector finalGravity = localGravity;
    if (isTared) {
      finalGravity.axis.x = R[0][0]*localGravity.axis.x + R[0][1]*localGravity.axis.y + R[0][2]*localGravity.axis.z;
      finalGravity.axis.y = R[1][0]*localGravity.axis.x + R[1][1]*localGravity.axis.y + R[1][2]*localGravity.axis.z;
      finalGravity.axis.z = R[2][0]*localGravity.axis.x + R[2][1]*localGravity.axis.y + R[2][2]*localGravity.axis.z;
    }

    // --- SAUVEGARDE MUTEX ---
    if (xGravityMutex != NULL) {
      if (xSemaphoreTake(xGravityMutex, 0) == pdTRUE) {
        sharedGravity = finalGravity;
        xSemaphoreGive(xGravityMutex);
      }
    }

    watchdogCounter++;
    if (watchdogCounter >= 100) {
      vTaskDelay(pdMS_TO_TICKS(1)); 
      watchdogCounter = 0;
    }

    while(micros() < nextWakeTimeUs) {
      taskYIELD();
    }
    nextWakeTimeUs += periodUs;
  }
}

/* --- API Publique --- */
void initIMUSystem() {
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CS_ACCEL);
  if (accel.begin() < 0 || gyro.begin() < 0) {
    Serial.println("[IMU] Erreur d'initialisation du BMI088 !");
    while(1);
  }

  accel.setOdr(Bmi088Accel::ODR_400HZ_BW_145HZ);
  gyro.setOdr(Bmi088Gyro::ODR_400HZ_BW_47HZ);
  accel.setRange(Bmi088Accel::RANGE_6G);
  gyro.setRange(Bmi088Gyro::RANGE_500DPS);

  xGravityMutex = xSemaphoreCreateMutex();
  if (xGravityMutex != NULL) {
    xTaskCreatePinnedToCore(IMUTask, "IMU_Task", 8192, NULL, 2, NULL, 0);
  }
}

bool getGravityVector(FusionVector* outGravity) {
  if (xGravityMutex == NULL || outGravity == NULL) return false;
  if (xSemaphoreTake(xGravityMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    *outGravity = sharedGravity;
    xSemaphoreGive(xGravityMutex);
    return true;
  }
  return false;
}

// Déclencheurs depuis le Cœur 1
void setTareCalibration() {
  requestTare = true;
}

void clearTareCalibration() {
  requestReset = true;
}