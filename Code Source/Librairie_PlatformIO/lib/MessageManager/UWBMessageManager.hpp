#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "UWBDataManager.hpp"

// Structure des messages "applicatifs" (Ordres et distances de sécurité)
struct UWBMessage {
    bool aIsStandardDistanceMessage; 
    int senderId;
    int receiverId;
    int orderType;
    float dataValue; 
};

bool decodeUWBMessage(const String &pRawMessage, UWBMessage &outMessage, Stream & pSerial);

bool receiveUWBMessage(Stream &pUWBSerial, String &outRawMessage, Stream & pSerial);

void sendDistanceToTag(Stream &pUWBSerial, Stream & pSerial, uint8_t pSenderID, uint8_t pReceiverID, float pDistance);

void sendDistancesToAnchor(Stream & pUWBSerial, Stream & pSerial, String & pRawRangeMessage);

void sendOrderToTag(Stream &pUWBSerial, Stream & pSerial, uint8_t pSenderID, uint8_t pReceiverID, uint8_t pOrderType);

String sendATCommand(String command, Stream & pSerial, Stream & pUWBSerial);

String sendATCommandWithResult(String command, Stream & pSerial, Stream & pUWBSerial);

