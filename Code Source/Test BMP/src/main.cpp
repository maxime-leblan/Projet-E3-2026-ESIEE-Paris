#include <SPI.h>
#include <Adafruit_BMP5xx.h>

// Définition de la broche CS (Chip Select)
#define BMP_CS 10

Adafruit_BMP5xx bmp;

float getPression() {
  // 1. Première lecture "fantôme" pour vider le registre (on ne stocke pas le résultat)
  bmp.performReading();
  
  // 2. On laisse 50 millisecondes au capteur pour faire sa vraie mesure
  delay(50); 
  
  // 3. Deuxième lecture (la vraie valeur)
  if (bmp.performReading()) {
    // On retourne la pression convertie en hPa
    return (float)(bmp.pressure);
  } else {
    // Code d'erreur si la communication lâche
    return -1.0; 
  }
}

void setup() {
  Serial.begin(115200);
  
  // Attendre que le port série s'ouvre
  while (!Serial) delay(10);
  Serial.println("\nDémarrage du capteur BMP581 en mode SPI...");

  // Initialisation du capteur en SPI matériel
  // On passe le numéro de la broche CS à la fonction begin()
  // On précise la broche CS ET on force l'utilisation du bus SPI
  if (!bmp.begin(BMP_CS, &SPI)) {
    Serial.println("Capteur introuvable !");
    Serial.println("Vérifiez le câblage :");
    Serial.println("- SCK  -> 13");
    Serial.println("- SDO  -> 12");
    Serial.println("- SDI  -> 11");
    Serial.println("- CS   -> 10");
    while (1) delay(10); // Bloque le programme en cas d'erreur
  }
  
  Serial.println("Capteur SPI détecté avec succès !");
}

void loop() {
  // 1. On appelle la fonction et on stocke le résultat dans une variable float
  float maPression = getPression();

  // 2. On vérifie que la lecture a bien fonctionné (différent de -1.0)
  if (maPression != -1.0) {
    Serial.print("Valeur récupérée : ");
    Serial.print(maPression, 10); // Affiche avec 4 chiffres significatifs
    Serial.println(" hPa");
  } else {
    Serial.println("Erreur : Impossible de récupérer la pression.");
  }

  delay(1000);
}