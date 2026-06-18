#include "UWBMessageManager.hpp"

bool decodeUWBMessage(const String &pRawMessage, UWBMessage &outMessage) {
    // 1. Essayer de décoder comme un message de type DISTANCE (3)
    std::vector<float> distData = getFloatDataFromString(pRawMessage.c_str(), UWB_MESSAGE_DISTANCE_REGEX);
    if (!distData.empty()) {
        outMessage.senderId = getSenderIdFromUWBMessage(distData);
        outMessage.receiverId = getReceiverIdFromUWBMessage(distData);
        outMessage.orderType = getOrderTypeFromUWBMessage(distData);
        outMessage.dataValue = getDistanceFromUWBMessage(distData); // La distance est le 4ème élément
        return true;
    }

    // 2. Si ce n'est pas une distance, essayer de décoder comme un ORDRE (1 ou 2)
    std::vector<float> orderData = getFloatDataFromString(pRawMessage.c_str(), UWB_MESSAGE_ORDER_REGEX);
    if (!orderData.empty()) {
        outMessage.senderId = getSenderIdFromUWBMessage(orderData);
        outMessage.receiverId = getReceiverIdFromUWBMessage(orderData);
        outMessage.orderType = getOrderTypeFromUWBMessage(orderData);
        outMessage.dataValue = 0.0f; // Pas de donnée supplémentaire pour les ordres 1 et 2
        return true;
    }

    return false; // Échec du décodage si aucune regex ne correspond
}

bool receiveUWBMessage(Stream &pUWBSerial, String &outRawMessage) {
    if (pUWBSerial.available()) {
        String input = pUWBSerial.readStringUntil('\n');
        if (input.startsWith("+RDATA")) {
            // Le format AT+RDATA contient souvent des infos système, 
            // on cherche la partie "données" que vous avez envoyée
            outRawMessage = input; 
            return true;
        }
    }
    return false;
}

bool receiveUWBDistanceMessage(Stream &pUWBSerial, String &outRawMessage)
{
    if (pUWBSerial.available()) {
        String input = pUWBSerial.readStringUntil('\n');
        // On cherche le mot clé RANGE ou range de votre TAG_DATA_REGEX
        if (input.startsWith("AT+RANGE") || input.indexOf("range:") != -1) {
            outRawMessage = input;
            return true;
        }
    }
    return false;
}

void sendOrderToTag(Stream &pUWBSerial, uint8_t pSenderID, uint8_t pReceiverID, uint8_t pOrderType) {
    // Construction du message : ex "0:4:1"
    String message = String(pSenderID) + ":" + String(pReceiverID) + ":" + String(pOrderType);
    
    pUWBSerial.print("AT+DATA=");
    pUWBSerial.print(message.length());
    pUWBSerial.print(",");
    pUWBSerial.println(message);
}

void sendDistanceToTag(Stream &pUWBSerial, uint8_t pSenderID, uint8_t pReceiverID, float pDistance) {
    // Construction du message : ex "0:4:3:5.45"
    String message = String(pSenderID) + ":" + String(pReceiverID) + ":3:" + String(pDistance, 2);
    
    pUWBSerial.print("AT+DATA=");
    pUWBSerial.print(message.length());
    pUWBSerial.print(",");
    pUWBSerial.println(message);
}

void configureUWBForMessaging(Stream &pUWBSerial) {
    // AT+SETCAP=(x1),(x2),(x3)
    // x3 = 1 active le mode "Extended packet" indispensable pour AT+DATA
    // On garde des valeurs par défaut pour x1 et x2 (ex: 10, 10)
    
    pUWBSerial.println("AT+SETCAP=10,10,1");
    delay(500); // Temps pour que le module traite la commande
    
    pUWBSerial.println("AT+SAVE");
    delay(500); // Temps pour la sauvegarde en mémoire flash
}
