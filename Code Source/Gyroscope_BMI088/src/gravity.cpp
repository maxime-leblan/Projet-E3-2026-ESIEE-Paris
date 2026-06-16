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

  for(;;) {
    unsigned long currentTime = micros();
    float deltaTime = (float)(currentTime - previousTime) / 1000000.0f;
    previousTime = currentTime;

    // Lecture matérielle
    accel.readSensor();
    gyro.readSensor();

    // Conversion
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

    // Mathématiques de Madgwick
    gyroscope = FusionBiasUpdate(&bias, gyroscope);
    FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, deltaTime);
    FusionVector localGravity = FusionAhrsGetGravity(&ahrs);

    // Sauvegarde Thread-Safe
    if (xGravityMutex != NULL) {
      if (xSemaphoreTake(xGravityMutex, 0) == pdTRUE) {
        sharedGravity = localGravity;
        xSemaphoreGive(xGravityMutex);
      }
    }

    // Gestion du Watchdog
    watchdogCounter++;
    if (watchdogCounter >= 100) {
      vTaskDelay(pdMS_TO_TICKS(1)); 
      watchdogCounter = 0;
    }

    // Cadencement strict
    while(micros() < nextWakeTimeUs) {
      taskYIELD();
    }
    nextWakeTimeUs += periodUs;
  }
}

/* --- Implémentation des Fonctions Publiques --- */

void initIMUSystem() {
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CS_ACCEL);

  if (accel.begin() < 0 || gyro.begin() < 0) {
    Serial.println("[IMU] Erreur d'initialisation du BMI088 !");
    while(1); // Arrêt critique
  }

  // Configuration optimale validée expérimentalement
  accel.setOdr(Bmi088Accel::ODR_400HZ_BW_145HZ);
  gyro.setOdr(Bmi088Gyro::ODR_400HZ_BW_47HZ);

  xGravityMutex = xSemaphoreCreateMutex();

  if (xGravityMutex != NULL) {
    xTaskCreatePinnedToCore(
      IMUTask, "IMU_Task", 8192, NULL, 2, NULL, 0 // Cœur 0
    );
    Serial.println("[IMU] Systeme d'acquisition 400Hz demarre sur le Coeur 0.");
  }
}

bool getGravityVector(FusionVector* outGravity) {
  if (xGravityMutex == NULL || outGravity == NULL) return false;

  // Attente maximum de 5ms pour ne pas bloquer le Cœur 1
  if (xSemaphoreTake(xGravityMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    *outGravity = sharedGravity; // Copie des données via le pointeur
    xSemaphoreGive(xGravityMutex);
    return true;
  }
  return false;
}