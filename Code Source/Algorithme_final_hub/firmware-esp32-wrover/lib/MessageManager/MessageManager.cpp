#include "MessageManager.hpp"

InitEntry::InitEntry() {}

InitEntry::InitEntry(int pUartNumber, int pRXGPIONumber, int pTXGPIONumber, int pBaud)
{
    aUartNumber = pUartNumber;
    aRXGPIONumber = pRXGPIONumber;
    aTXGPIONumber = pTXGPIONumber;
    aBaud = pBaud;
}

MessageEntry::MessageEntry(int pMACAdressId)
{
    aMACAdressId = pMACAdressId;
}

MessageEntry::MessageEntry(HardwareSerial & pSerialDestination, String pMessage)
{
    aSerialDestination = pSerialDestination;
    aMessage = pMessage;
}

void sendData(MessageEntry pMessage)
{
    #ifdef COMMUNICATION_MODE
        sendDataWifi(pMessage.aMACAdressId);
    #else
        sendDataUART(pMessage.aSerialDestination, pMessage.aMessage);
    #endif
}

