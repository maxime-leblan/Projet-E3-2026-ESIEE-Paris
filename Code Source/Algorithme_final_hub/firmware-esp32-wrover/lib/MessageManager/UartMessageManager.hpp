#pragma once 

/*
Pour le Hub CAN :
GPIO_33 TXD
GPIO_32 RXD
*/

#include <Arduino.h>

/**
 * @brief Configure et initialise un canal UART pour la réception de données.
 * * Cette fonction associe un canal matériel de l'ESP32 à des broches GPIO spécifiques
 * et configure la vitesse en bauds pour écouter les messages entrants.
 * * @param pUartNumber Numéro du canal matériel UART (0, 1 ou 2).
 * @param pRXGPIONumber Numéro de la broche GPIO utilisée pour la réception (RX).
 * @param TXGPIONumber Numéro de la broche GPIO utilisée pour la transmission (TX).
 * @param pBaud Vitesse de transmission en bits par seconde (ex: 115200).
 * @param pUARTChannel Référence vers l'objet HardwareSerial qui sera initialisé.
 */
void initUARTReceiver(int pUartNumber, int pRXGPIONumber, int TXGPIONumber, int pBaud, HardwareSerial & pUARTChannel);

/**
 * @brief Configure et initialise un canal UART pour la transmission de données.
 * * Cette fonction prépare un canal matériel de l'ESP32 pour envoyer des paquets de données
 * vers un périphérique externe.
 * * @param pUartNumber Numéro du canal matériel UART (0, 1 ou 2).
 * @param pRXGPIONumber Numéro de la broche GPIO utilisée pour la réception (RX).
 * @param TXGPIONumber Numéro de la broche GPIO utilisée pour la transmission (TX).
 * @param pBaud Vitesse de transmission en bits par seconde (ex: 115200).
 * @param pUARTChannel Référence vers l'objet HardwareSerial qui sera initialisé.
 */
void initUARTTransmitter(int pUartNumber, int pRXGPIONumber, int TXGPIONumber, int pBaud, HardwareSerial & pUARTChannel);

/**
 * @brief Vérifie de manière non bloquante la présence de données et extrait le message.
 * * Si des données sont disponibles dans le buffer de réception, cette fonction lit les caractères
 * jusqu'à rencontrer un caractère de fin de ligne ('\\n') et remplit la chaîne passée en paramètre.
 * * @param pSerialSource Référence vers le canal UART à écouter.
 * @param pMessageReceived Référence vers la chaîne de caractères qui stockera le message reçu.
 */
void receiveDataUART(HardwareSerial & pSerialSource, String & pMessageReceived);

/**
 * @brief Transmet une chaîne de caractères suivie d'un saut de ligne via le canal UART spécifié.
 * * @param pSerialDestination Référence vers le canal UART par lequel envoyer le message.
 * @param pMessage Chaîne de caractères contenant le message (ex: chaîne JSON).
 */
void sendDataUART(HardwareSerial & pSerialDestination, String pMessage);

