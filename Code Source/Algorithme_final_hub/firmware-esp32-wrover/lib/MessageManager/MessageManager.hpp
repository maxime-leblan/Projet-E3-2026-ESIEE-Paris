#pragma once

// Définition de la macro pour activer une option spécifique
// Cela peut être fait ici ou via le compilateur (-DMY_OPTION)

// On définit une macro indiquand qu'on est en mode de communication Wifi, sinon on est en mode de communication UART
// #define COMMUNICATION_MODE 1

#ifdef COMMUNICATION_MODE
    // Inclure l'en-tête de la première bibliothèque si la macro est définie
    #include "WifiMessageManager.hpp"
#else
    // Inclure l'en-tête de la seconde bibliothèque par défaut
    #include "UartMessageManager.hpp"
#endif

struct InitEntry{
    // Partie pour le Wifi (vide)

    // Partie pour l'UART
    int aUartNumber = -1;
    int aRXGPIONumber = -1;
    int aTXGPIONumber = -1;
    int aBaud = -1;

    /**
     * Constructeur pour la partie Wifi
     */
    InitEntry();

    /**
     * Constructeur pour la partie UART
     */
    InitEntry(int pUartNumber, int pRXGPIONumber, int pTXGPIONumber, int pBaud);
};

HardwareSerial gDefaultValue;

struct MessageEntry{
    // Partie pour le Wifi
    int aMACAdressId = -1;

    // Partie pour l'UART
    HardwareSerial & aSerialDestination = gDefaultValue;
    String aMessage = "";

    /**
     * Constructeur pour la partie Wifi
     */
    MessageEntry(int pMACAdressId);

    /**
     * Constructeur pour la partie UART
     */
    MessageEntry(HardwareSerial & pSerialDestination, String pMessage);
};

void sendData(MessageEntry pMessage);