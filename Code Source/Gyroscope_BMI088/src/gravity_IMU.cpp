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

/* --- Variables Partagées et Synchronisation --- */
static SemaphoreHandle_t xGravityMutex = NULL;
static FusionVector sharedGravity = FUSION_VECTOR_ZERO;

// Timer matériel et Sémaphore de réveil pour l'ordonnancement précis
static hw_timer_t * imuTimer = NULL;
static volatile SemaphoreHandle_t timerSemaphore = NULL;

/* --- Variables d'Étalonnage (Privées) --- */
static volatile bool requestTare = false;
static volatile bool requestReset = false;
static bool isTared = false;
static float R[3][3] = { {1,0,0}, {0,1,0}, {0,0,1} }; // Matrice Identité par défaut

/* --- Interruption Matérielle (ISR) à 400 Hz --- */
// IRAM_ATTR force cette fonction critique à rester en RAM interne ultra-rapide
void IRAM_ATTR onTimerISR() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  // On donne le jeton pour débloquer instantanément la tâche IMU
  xSemaphoreGiveFromISR(timerSemaphore, &xHigherPriorityTaskWoken);
  
  // Si l'IMUTask a une priorité plus haute que ce que faisait le processeur,
  // on force FreeRTOS à changer de contexte immédiatement.
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

/* --- La Tâche FreeRTOS (Privée) --- */
static void IMUTask(void *pvParameters) {
  FusionBias bias;
  FusionAhrs ahrs;
  FusionBiasInitialise(&bias);
  FusionAhrsInitialise(&ahrs);

  unsigned long previousTime = micros();

  // Variables de profilage
  unsigned long maxExecutionTimeUs = 0;
  int displayCounter = 0;

  for(;;) {
    // 1. ENDORMISSEMENT TOTAL (0% CPU)
    // La tâche reste bloquée ici. Le Cœur 0 est libre pour le bus CAN ou le Watchdog.
    if (xSemaphoreTake(timerSemaphore, portMAX_DELAY) == pdTRUE) {
      
      unsigned long currentTime = micros();
      float deltaTime = (float)(currentTime - previousTime) / 1000000.0f;
      previousTime = currentTime;

      // >>> DÉBUT DU PROFILAGE <<<
      unsigned long startComputation = micros();

      // 2. Lecture SPI matérielle
      accel.readSensor();
      gyro.readSensor();

      // Conversion en unités système (Radians et G)
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

      // 3. Algorithme de Fusion (Madgwick AHRS)
      gyroscope = FusionBiasUpdate(&bias, gyroscope);
      FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, deltaTime);
      FusionVector localGravity = FusionAhrsGetGravity(&ahrs);

      // 4. Gestion du Tare (Calcul Matriciel de Rodrigues)
      if (requestTare) {
        float ax = localGravity.axis.x;
        float ay = localGravity.axis.y;
        float az = localGravity.axis.z;

        float c = -az; 
        if (c > -0.999f) { // Protection singularité
          float vx = -ay;
          float vy = ax;
          float h = 1.0f / (1.0f + c);

          R[0][0] = c + h * vx * vx;  R[0][1] = h * vx * vy;      R[0][2] = vy;
          R[1][0] = h * vx * vy;      R[1][1] = c + h * vy * vy;  R[1][2] = -vx;
          R[2][0] = -vy;              R[2][1] = vx;               R[2][2] = c;
        } else {
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

      // 5. Application de la matrice (Projection)
      FusionVector finalGravity = localGravity;
      if (isTared) {
        finalGravity.axis.x = R[0][0]*localGravity.axis.x + R[0][1]*localGravity.axis.y + R[0][2]*localGravity.axis.z;
        finalGravity.axis.y = R[1][0]*localGravity.axis.x + R[1][1]*localGravity.axis.y + R[1][2]*localGravity.axis.z;
        finalGravity.axis.z = R[2][0]*localGravity.axis.x + R[2][1]*localGravity.axis.y + R[2][2]*localGravity.axis.z;
      }

      // 6. Sauvegarde Thread-Safe (Mutex atomique)
      if (xGravityMutex != NULL) {
        if (xSemaphoreTake(xGravityMutex, 0) == pdTRUE) {
          sharedGravity = finalGravity;
          xSemaphoreGive(xGravityMutex);
        }
      }

      // >>> FIN DU PROFILAGE <<<
      unsigned long executionTimeUs = micros() - startComputation;

      if (executionTimeUs > maxExecutionTimeUs) {
        maxExecutionTimeUs = executionTimeUs;
      }

      // Affichage statistique toutes les 2 secondes
      displayCounter++;
      if (displayCounter >= 800) {
        Serial.print("[PROFILING] Temps d'exéc actuel: ");
        Serial.print(executionTimeUs);
        Serial.print(" us | Pire cas (WCET): ");
        Serial.print(maxExecutionTimeUs);
        Serial.print(" us | Marge restante (CPU Libre): ");
        Serial.print(2500 - executionTimeUs);
        Serial.println(" us");
        
        displayCounter = 0;
      }
    }
  }
}

/* --- API Publique d'Initialisation --- */
void initIMUSystem() {
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CS_ACCEL);
  if (accel.begin() < 0 || gyro.begin() < 0) {
    Serial.println("[IMU] Erreur d'initialisation matérielle du BMI088 !");
    while(1);
  }

  // Application des filtres matériels (Critère de Nyquist respecté pour 400Hz)
  accel.setOdr(Bmi088Accel::ODR_400HZ_BW_145HZ);
  gyro.setOdr(Bmi088Gyro::ODR_400HZ_BW_47HZ);
  accel.setRange(Bmi088Accel::RANGE_6G);
  gyro.setRange(Bmi088Gyro::RANGE_500DPS);

  xGravityMutex = xSemaphoreCreateMutex();
  timerSemaphore = xSemaphoreCreateBinary();

  if (xGravityMutex != NULL && timerSemaphore != NULL) {
    // Allocation de 8 Ko sur le Cœur 0 en priorité Haute (2)
    xTaskCreatePinnedToCore(IMUTask, "IMU_Task", 8192, NULL, 2, NULL, 0);

    // Configuration du Timer Matériel de l'ESP32
    // Prescaler de 80 = 1 tick par microseconde (Horloge de base à 80MHz)
    imuTimer = timerBegin(0, 80, true); 
    timerAttachInterrupt(imuTimer, &onTimerISR, true); 
    timerAlarmWrite(imuTimer, 2500, true); // Déclenchement à 2500 µs
    timerAlarmEnable(imuTimer); 

    Serial.println("[IMU] Bloc d'acquisition ISR 400Hz stabilisé sur le Cœur 0.");
  }
}

/* --- API Publique de Lecture --- */
bool getGravityVector(FusionVector* outGravity) {
  if (xGravityMutex == NULL || outGravity == NULL) return false;
  
  if (xSemaphoreTake(xGravityMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    *outGravity = sharedGravity;
    xSemaphoreGive(xGravityMutex);
    return true;
  }
  return false;
}

/* --- Déclencheurs depuis le Cœur 1 --- */
void setTareCalibration() { requestTare = true; }
void clearTareCalibration() { requestReset = true; }