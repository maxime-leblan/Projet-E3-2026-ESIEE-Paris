#include "UartMessageManager.hpp"

void initUARTReceiver(int pUartNumber, int pRXGPIONumber, int TXGPIONumber, int pBaud, HardwareSerial & pUARTChannel)
{
    
  pUARTChannel.setRxBufferSize(2048);

  pUARTChannel.begin(pBaud, SERIAL_8N1, pRXGPIONumber, TXGPIONumber);  // UART setup
  
  Serial.println("ESP32 UART Receiver initialised");
}

void initUARTTransmitter(int pUartNumber, int pRXGPIONumber, int TXGPIONumber, int pBaud, HardwareSerial & pUARTChannel)
{
  pUARTChannel.setRxBufferSize(2048);

  pUARTChannel.begin(pBaud, SERIAL_8N1, pRXGPIONumber, TXGPIONumber);  // UART setup
  
  Serial.println("ESP32 UART Transmitter initialised");
}

void receiveDataUART(HardwareSerial & pSerialSource, String & pMessageReceived)
{
  // Check if data is available to read
  if (pSerialSource.available()) {
    // Read data and display it
    String message = pSerialSource.readStringUntil('\n');
    Serial.println("Received: " + message);
    pMessageReceived = message;
  }
}

void sendDataUART(HardwareSerial & pSerialDestination, String pMessage)
{
  // Send message over UART
  pSerialDestination.println(pMessage);

  //Serial.println("Sent: " + pMessage);
}