#pragma once 

/*
Pour le Hub CAN :
GPIO_25 TXD
GPIO_34 RXD
*/

#include <esp_now.h>// https://github.com/espressif/esp-idf/blob/master/components/esp_wifi/include/esp_now.h
#include <WiFi.h>

// Structure example to send data
// Must match the receiver structure
typedef struct Message {
  String aMessage;
  int aModuleId;
  char* aSenderName;
} Message;

// Variable contenant le message envoyé par le hub
Message myData;
// Variable contenant le dernier message reçu par le hub
Message dataRcv;


/**
 * Configure puis renvoie un canal de communication UART dont le numéro d'identification est passé en paramètre
 * @param pUartNumber Numéro du canal de communication UART. Doit valoir 0, 1 ou 2.
 * @param pRXGPIONumber Numéro GPIO de la broche utilisée pour la réception des données par UART
 * @param TXGPIONumber Numéro GPIO de la broche utilisée pour la transmission des données par UART
 * @param pBaud Vitesse de transmission des données série, exprimée en bits par seconde (bps), souvent à 115200 ou 9600 pour les ESP32
 * @return Un canal de communication UART configuré.
 */
HardwareSerial initUARTReceiver(int pUartNumber, int pRXGPIONumber, int TXGPIONumber, int pBaud);

/**
 * Configure puis renvoie un canal de communication UART dont le numéro d'identification est passé en paramètre
 * @param pUartNumber Numéro du canal de communication UART. Doit valoir 0, 1 ou 2.
 * @param pRXGPIONumber Numéro GPIO de la broche utilisée pour la réception des données par UART
 * @param TXGPIONumber Numéro GPIO de la broche utilisée pour la transmission des données par UART
 * @param pBaud Vitesse de transmission des données série, exprimée en bits par seconde (bps), souvent à 115200 ou 9600 pour les ESP32
 * @return Un canal de communication UART configuré.
 */
HardwareSerial initUARTTransmitter(int pUartNumber, int pRXGPIONumber, int TXGPIONumber, int pBaud);

/**
 * Vérifie si des données ont été envoyées par le canal UART passé en paramètre, et si c'est le cas, renvoie le message reçu.
 * @param pSerialSource Référence de la variable contenant le canal UART par lequel la source a envoyé son message
 * @return Le message réceptionné sous forme de chaîne de caractère
 */
String receiveDataUART(HardwareSerial & pSerialSource);

/**
 * Envoie le contenu de du message passé en paramètre au canal UART passé en paramètre
 * @param pSerialDestination Référence de la variable contenant le canal UART par lequel doit passer le message pour atteindre le destinataire
 * @param pMessage Message à envoyer au destinataire
*/
void sendDataUART(HardwareSerial & pSerialDestination, String pMessage);