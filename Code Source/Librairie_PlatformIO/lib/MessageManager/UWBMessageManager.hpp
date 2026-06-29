#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "UWBDataManager.hpp"

// Dans UWBDataManager.hpp
struct UWBMessage {
    bool aIsStandardDistanceMessage; // indique si le message est un message AT+RANGE ou non
    int senderId;
    int receiverId;
    int orderType;
    int pressionPa;
    float dataValue; // Utilisé pour la distance ou toute autre donnée numérique
};

/**
 * Décode un message UWB envoyé à l'origine en CAN par le Hub puis convertit en message UWB par une Ancre en utilisant les regex spécifiques par type et stocke le résultat dans la structure UWBMessage. Cette fonction est uniquement utilisable par un Tag
 */
bool decodeUWBMessage(const String &pRawMessage, UWBMessage &outMessage, Stream & pSerial);

/**
 * Tente de réceptionner un message personnalisé envoyé entre une ancre et un tag via UWB
 * @return True si on a bien réceptionné un message, false sinon
 */
bool receiveUWBMessage(Stream &pUWBSerial, String &outRawMessage, Stream & pSerial);

/**
 * Tente de réceptionner le message classique contenant l'id du tag émetteur avec toutes les distances aux ancres via UWB. Cette fonction est utilisé par les ancres. La fonction est compatible avec la XIAO (tag)
 * @return True si on a bien réceptionné un message, false sinon
 */
//bool receiveUWBDistanceMessage(Stream &pUWBSerial, Stream &pSerial, String &outRawMessage);

/**
 * Tente d'extraire un message de distance UWB du moniteur série pUWBSerial du Tag qui a appelé la fonction. Renvoie la String extraite dans outRawMessage.
 * @return Le message contenant les distances extraites, ou la chaine vide "" si rien n'a été lu.
 */
//void readDistancesInTagSerial(Stream & pUWBSerial, Stream & pSerial, String &outRawMessage);

/**
 * Configure le module UWB pour permettre l'envoi de messages personnalisés. La fonction est compatible avec la XIAO (tag)
 */
//void configureUWBForMessaging(Stream &pUWBSerial);

/**
 * Envoie la distance au Tag par rapport à la zone de sécurité
 * Format: <ID_Emetteur>:<ID_Destinataire>:3:<Distance>
 */
void sendDistanceToTag(Stream &pUWBSerial, Stream & pSerial, uint8_t pSenderID, uint8_t pReceiverID, float pDistance);

/**
 * 
 */
void sendDistancesToAnchor(Stream & pUWBSerial, Stream & pSerial, String & pRawRangeMessage);

/**
 * Nouvelle fonction qui envoie la pression en début de string "[Pression] & AT+RANGE..."
 */
void sendDistancesWithPressionToAnchor(Stream & pUWBSerial, Stream & pSerial, String & pRawRangeMessage, float pPression_hPa);

/**
 * Envoie un ordre au Tag
 * Format: <ID_Emetteur>:<ID_Destinataire>:<Type_Ordre>
 */
void sendOrderToTag(Stream &pUWBSerial, Stream & pSerial, uint8_t pSenderID, uint8_t pReceiverID, uint8_t pOrderType);

/**
 * Envoie des commandes au module UWB par le port pUWBSerial et affiche du texte de débug dans pSerial
 */
String sendATCommand(String command, Stream & pSerial, Stream & pUWBSerial);

/**
 * Envoie une commande AT au module UWB, attend la réponse, et extrait la ligne de données utile.
 * Utile pour les commandes d'interrogation (finissant par "?").
 * @param command La commande à envoyer (ex: "AT+GETCFG?")
 * @param pSerial Le port série pour l'affichage de debug (ex: Serial)
 * @param pUWBSerial Le port série physique relié au module UWB (ex: UWBSerial)
 * @return La String contenant les données (ex: "AT+GETCFG=4,0,1,0"), ou une chaîne vide "" si échec.
 */
String sendATCommandWithResult(String command, Stream & pSerial, Stream & pUWBSerial);