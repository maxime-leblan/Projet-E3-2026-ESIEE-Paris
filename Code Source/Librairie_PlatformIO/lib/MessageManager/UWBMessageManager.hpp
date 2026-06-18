#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "UWBDataManager.hpp"

// Dans UWBDataManager.hpp
struct UWBMessage {
    int senderId;
    int receiverId;
    int orderType;
    float dataValue; // Utilisé pour la distance ou toute autre donnée numérique
};

/**
 * Décode un message UWB en utilisant les regex spécifiques par type
 * et stocke le résultat dans la structure UWBMessage.
 */
bool decodeUWBMessage(const String &pRawMessage, UWBMessage &outMessage);

/**
 * Tente de réceptionner un message envoyé entre une ancre et un tag via UWB
 * @return True si on a bien réceptionné un message, false sinon
 */
bool receiveUWBMessage(HardwareSerial &pUWBSerial, String &outRawMessage);

/**
 * Configure le module UWB pour permettre l'envoi de messages personnalisés
 */
void configureUWBForMessaging(HardwareSerial &pUWBSerial);

/**
 * Envoie la distance au Tag par rapport à la zone de sécurité
 * Format: <ID_Emetteur>:<ID_Destinataire>:3:<Distance>
 */
void sendDistanceToTag(HardwareSerial &pUWBSerial, uint8_t pSenderID, uint8_t pReceiverID, float pDistance);

/**
 * Envoie un ordre au Tag
 * Format: <ID_Emetteur>:<ID_Destinataire>:<Type_Ordre>
 */
void sendOrderToTag(HardwareSerial &pUWBSerial, uint8_t pSenderID, uint8_t pReceiverID, uint8_t pOrderType);