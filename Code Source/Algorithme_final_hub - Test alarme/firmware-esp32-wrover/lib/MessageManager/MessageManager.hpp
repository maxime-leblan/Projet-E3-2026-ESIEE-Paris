#pragma once

// Définition de la macro pour activer une option spécifique
// Cela peut être fait ici ou via le compilateur (-DMY_OPTION)

#include "WifiMessageManager.hpp"
#include "UartMessageManager.hpp"

HardwareSerial gDefaultValue1;
String gDefaultValue2;

struct InitEntry{
    // Partie pour le Wifi (vide)

    // Partie pour l'UART
    int aUartNumber = -1;
    int aRXGPIONumber = -1;
    int aTXGPIONumber = -1;
    int aBaud = -1;
    HardwareSerial & aUARTChannel = gDefaultValue1;

    /**
     * Constructeur pour la partie Wifi
     */
    InitEntry();

    /**
     * Constructeur pour la partie UART
     * @param pUartNumber Numéro du canal de communication UART. Doit valoir 0, 1 ou 2.
     * @param pRXGPIONumber Numéro GPIO de la broche utilisée pour la réception des données par UART
     * @param TXGPIONumber Numéro GPIO de la broche utilisée pour la transmission des données par UART
     * @param pBaud Vitesse de transmission des données série, exprimée en bits par seconde (bps), souvent à 115200 ou 9600 pour les ESP32
     * @param pUARTChannel Référence à la variable contenant le canal de communication UART que l'on veut configurer
    */
    InitEntry(int pUartNumber, int pRXGPIONumber, int pTXGPIONumber, int pBaud, HardwareSerial & pUARTChannel);
};

struct MessageEntry{
    // Partie pour le Wifi
    int aMACAdressId = -1;

    // Partie pour l'UART
    HardwareSerial & aSerialPort = gDefaultValue1;
    String aMessage = "";
    String & aMessageReceived = gDefaultValue2;

    /**
     * Constructeur pour la partie Wifi
     * @param pMACAdressId Identifiant du tableau global broadcastAddress correspondant à l'adresse MAC du destinataire
     */
    MessageEntry(int pMACAdressId);

    /**
     * Premier constructeur pour la partie UART (nécessaire pour utiliser la fonction sendData)
     * @param pSerialDestination Référence de la variable contenant le canal UART par lequel doit passer le message pour atteindre le destinataire
     * @param pMessage Message à envoyer au destinataire
     */
    MessageEntry(HardwareSerial & pSerialDestination, String pMessage);

    /**
     * Second constructeur pour la partie UART (nécessaire pour utiliser la fonction receiveData)
     * @param pSerialSource Référence de la variable contenant le canal UART par lequel la source a envoyé son message
     * @param pMessageReceived Référence de la variable contenant le message réceptionné sous forme de chaîne de caractère
     */
    MessageEntry(HardwareSerial & pSerialSource, String & pMessageReceived);
};

void initReceiverCommunication(InitEntry & pInit);

void initTransmitterCommunication(InitEntry & pInit);

void receiveData(MessageEntry pMessage);

void sendData(MessageEntry pMessage);