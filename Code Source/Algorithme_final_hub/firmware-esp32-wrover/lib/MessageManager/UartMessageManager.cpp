#include "UartMessageManager.hpp"

HardwareSerial initUartReceiver(int pUartNumber, int pRXGPIONumber, int TXGPIONumber, int pBaud)
{
    HardwareSerial vMySerial(pUartNumber);

    vMySerial.begin(pBaud, SERIAL_8N1, pRXGPIONumber, TXGPIONumber);  // UART setup
  
    Serial.println("ESP32 UART Receiver initialised");

    return vMySerial;
}

HardwareSerial initUartTransmitter(int pUartNumber, int pRXGPIONumber, int TXGPIONumber, int pBaud)
{
    HardwareSerial vMySerial(pUartNumber);

    vMySerial.begin(pBaud, SERIAL_8N1, pRXGPIONumber, TXGPIONumber);  // UART setup
  
    Serial.println("ESP32 UART Transmitter initialised");

    return vMySerial;
}

String receiveData(HardwareSerial & pSerialSource)
{
    // Check if data is available to read
  if (pSerialSource.available()) {
    // Read data and display it
    String message = pSerialSource.readStringUntil('\n');
    Serial.println("Received: " + message);
  }
}

void sendData(HardwareSerial & pSerialDestination, String pMessage)
{
    // Send message over UART
    pSerialDestination.println(pMessage);

    Serial.println("Sent: " + pMessage);
}