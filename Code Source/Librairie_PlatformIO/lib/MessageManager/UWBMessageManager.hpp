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
 * Décode un message UWB envoyé à l'origine en CAN par le Hub puis convertit en message UWB par une Ancre en utilisant les regex spécifiques par type et stocke le résultat dans la structure UWBMessage. Cette fonction est uniquement utilisable par un Tag
 */
bool decodeUWBMessage(const String &pRawMessage, UWBMessage &outMessage);

/**
 * Tente de réceptionner un message personnalisé envoyé entre une ancre et un tag via UWB
 * @return True si on a bien réceptionné un message, false sinon
 */
bool receiveUWBMessage(Stream &pUWBSerial, String &outRawMessage);

/**
 * Tente de réceptionner le message classique contenant l'id du tag émetteur avec toutes les distances aux ancres via UWB. Cette fonction est utilisé par les ancres. La fonction est compatible avec la XIAO (tag)
 * @return True si on a bien réceptionné un message, false sinon
 */
bool receiveUWBDistanceMessage(Stream &pUWBSerial, String &outRawMessage);

/**
 * Tente d'extraire un message de distance UWB du moniteur série pUWBSerial du Tag qui a appelé la fonction. Renvoie la String extraite dans outRawMessage.
 * @return Le message contenant les distances extraites, ou la chaine vide "" si rien n'a été lu.
 */
void readDistancesInTagSerial(Stream & pUWBSerial, Stream & pSerial, String &outRawMessage);

/**
 * Configure le module UWB pour permettre l'envoi de messages personnalisés. La fonction est compatible avec la XIAO (tag)
 */
void configureUWBForMessaging(Stream &pUWBSerial);

/**
 * Envoie la distance au Tag par rapport à la zone de sécurité
 * Format: <ID_Emetteur>:<ID_Destinataire>:3:<Distance>
 */
void sendDistanceToTag(Stream &pUWBSerial, uint8_t pSenderID, uint8_t pReceiverID, float pDistance);

/**
 * 
 */
void sendDistancesToAnchor(Stream & pUWBSerial, Stream & pSerial);

/**
 * Envoie un ordre au Tag
 * Format: <ID_Emetteur>:<ID_Destinataire>:<Type_Ordre>
 */
void sendOrderToTag(Stream &pUWBSerial, uint8_t pSenderID, uint8_t pReceiverID, uint8_t pOrderType);

/**
 * Envoie des commandes au module UWB par le port pUWBSerial et affiche du texte de débug dans pSerial
 */
String sendATCommand(String command, Stream & pSerial, Stream & pUWBSerial);