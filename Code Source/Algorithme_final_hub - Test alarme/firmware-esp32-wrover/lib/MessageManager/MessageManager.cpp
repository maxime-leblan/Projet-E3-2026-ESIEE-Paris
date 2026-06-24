#include "MessageManager.hpp"

InitEntry::InitEntry() {}

InitEntry::InitEntry(int pUartNumber, int pRXGPIONumber, int pTXGPIONumber, int pBaud, HardwareSerial & pUARTChannel)
{
    aUartNumber = pUartNumber;
    aRXGPIONumber = pRXGPIONumber;
    aTXGPIONumber = pTXGPIONumber;
    aBaud = pBaud;
    aUARTChannel = pUARTChannel;
}

MessageEntry::MessageEntry(int pMACAdressId)
{
    aMACAdressId = pMACAdressId;
}

MessageEntry::MessageEntry(HardwareSerial & pSerialDestination, String pMessage)
{
    aSerialPort = pSerialDestination;
    aMessage = pMessage;
}

MessageEntry::MessageEntry(HardwareSerial & pSerialSource, String & pMessageReceived)
{
    aSerialPort = pSerialSource;
    aMessageReceived = pMessageReceived;
}

void initReceiverCommunication(InitEntry & pInit)
{
    #ifdef COMMUNICATION_MODE
    #else
        initUARTReceiver(pInit.aUartNumber, pInit.aRXGPIONumber, pInit.aTXGPIONumber, pInit.aBaud, pInit.aUARTChannel);
    #endif
}

void initTransmitterCommunication(InitEntry & pInit)
{
    #ifdef COMMUNICATION_MODE
        initWifi();
    #else
        initUARTTransmitter(pInit.aUartNumber, pInit.aRXGPIONumber, pInit.aTXGPIONumber, pInit.aBaud, pInit.aUARTChannel);
    #endif
}

void receiveData(MessageEntry pMessage)
{
    #ifdef COMMUNICATION_MODE
    #else
        receiveDataUART(pMessage.aSerialPort, pMessage.aMessageReceived);
    #endif
}

void sendData(MessageEntry pMessage)
{
    #ifdef COMMUNICATION_MODE
        sendDataWifi(pMessage.aMACAdressId);
    #else
        sendDataUART(pMessage.aSerialPort, pMessage.aMessage);
    #endif
}

