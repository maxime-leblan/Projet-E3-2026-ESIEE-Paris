#include "UWBMessageManager.hpp"

bool decodeUWBMessage(const String &pRawMessage, UWBMessage &outMessage, Stream & pSerial) {

    // On vérifie si on a reçu un simple message de distance du module UWB lui-même
    if (pRawMessage.indexOf("AT+RANGE") != -1)
    {
        outMessage.senderId = 0;
        outMessage.receiverId = 0;
        outMessage.orderType = 0;
        outMessage.dataValue = 0.0f; 
        outMessage.aIsStandardDistanceMessage = true;

        // Gardé silencieux pour ne pas polluer l'écran en calibration
        // pSerial.println("[UWB DECOD] Message décodé de type AT+RANGE");
        return true;
    }

    // 1. Essayer de décoder comme un message de type DISTANCE (3)
    std::vector<float> distData = getFloatDataFromString(pRawMessage.c_str(), UWB_MESSAGE_DISTANCE_REGEX);
    if (!distData.empty()) {
        outMessage.senderId = getSenderIdFromUWBMessage(distData);
        outMessage.receiverId = getReceiverIdFromUWBMessage(distData);
        outMessage.orderType = getOrderTypeFromUWBMessage(distData);
        outMessage.dataValue = getDistanceFromUWBMessage(distData); 
        outMessage.aIsStandardDistanceMessage = false;

        pSerial.println("[UWB DECOD] Message décodé de type 'Distance entre tag et véhicule'. Contenu :");
        pSerial.println("[UWB DECOD] Distance = " + String(outMessage.dataValue));
        pSerial.println("[UWB DECOD] Type d'ordre = " + String(outMessage.orderType));
        pSerial.println("[UWB DECOD] ID émetteur = " + String(outMessage.senderId));
        pSerial.println("[UWB DECOD] ID destinataire = " + String(outMessage.receiverId));
        return true;
    }

    // 2. Si ce n'est pas une distance, essayer de décoder comme un ORDRE (1 ou 2)
    std::vector<float> orderData = getFloatDataFromString(pRawMessage.c_str(), UWB_MESSAGE_ORDER_REGEX);
    if (!orderData.empty()) {
        outMessage.senderId = getSenderIdFromUWBMessage(orderData);
        outMessage.receiverId = getReceiverIdFromUWBMessage(orderData);
        outMessage.orderType = getOrderTypeFromUWBMessage(orderData);
        outMessage.dataValue = 0.0f; 
        outMessage.aIsStandardDistanceMessage = false;

        pSerial.println("[UWB DECOD] Message décodé de type 'Ordre du Hub pour le tag'");
        pSerial.println("[UWB DECOD] Type d'ordre = " + String(outMessage.orderType));
        pSerial.println("[UWB DECOD] ID émetteur = " + String(outMessage.senderId));
        pSerial.println("[UWB DECOD] ID destinataire = " + String(outMessage.receiverId));
        return true;
    }

    return false; 
}

bool receiveUWBMessage(Stream &pUWBSerial, String &outRawMessage, Stream & pSerial) {
    if (pUWBSerial.available()) {
        // On lit jusqu'au saut de ligne
        String input = pUWBSerial.readStringUntil('\n');

        // =========================================================
        // DEBOGAGE BRUT BRUT (Niveau UART)
        // =========================================================
        if (input.length() > 0) {
            pSerial.print("[UART BRUT] Taille: ");
            pSerial.print(input.length());
            pSerial.print(" | Chaine: >>>");
            pSerial.print(input);
            pSerial.println("<<<");
        }
        // =========================================================

        input.trim(); // Nettoie les caractères invisibles (comme \r ou les espaces)
        
        outRawMessage = input;
        return true;
    }
    return false;
}

void sendOrderToTag(Stream &pUWBSerial, Stream & pSerial, uint8_t pSenderID, uint8_t pReceiverID, uint8_t pOrderType) {
    String message = String(pSenderID) + ":" + String(pReceiverID) + ":" + String(pOrderType);
    String atCommand = "AT+DATA=" + String(message.length()) + "," + message;
    pUWBSerial.println(atCommand);

    pSerial.println("[UWB SEND A->T] Ancre " + String(pSenderID) + " a envoyé à Tag " + String(pReceiverID) + "le message : " + atCommand);
}

void sendDistanceToTag(Stream &pUWBSerial, Stream & pSerial, uint8_t pSenderID, uint8_t pReceiverID, float pDistance) {
    String message = String(pSenderID) + ":" + String(pReceiverID) + ":3:" + String(pDistance, 2);
    String atCommand = "AT+DATA=" + String(message.length()) + "," + message;
    pUWBSerial.println(atCommand);

    pSerial.println("[UWB SEND A->T] Ancre " + String(pSenderID) + " a envoyé à Tag " + String(pReceiverID) + "le message : " + atCommand);
}

void sendDistancesToAnchor(Stream & pUWBSerial, Stream & pSerial, String & pRawRangeMessage) {
    String atCommand = "AT+DATA=" + String(pRawRangeMessage.length()) + "," + pRawRangeMessage;
    pUWBSerial.println(atCommand);
    pSerial.println("[UWB SEND T->A] Tag a envoyé : " + atCommand);
}

String sendATCommand(String command, Stream & pSerial, Stream & pUWBSerial) {
    String response = "";
    pSerial.print("Envoi au module UWB -> ");
    pSerial.println(command);
    
    while(pUWBSerial.available()) { pUWBSerial.read(); }
    pUWBSerial.println(command);
    
    uint32_t startTime = millis();
    const uint32_t maxGuardTimeout = 1000; 

    while ((millis() - startTime) < maxGuardTimeout) {
        if (pUWBSerial.available()) {
            char c = (char)pUWBSerial.read();
            response += c;
            
            if (response.endsWith("OK\r\n") || response.endsWith("ERROR\r\n")) {
                delay(5); 
                while (pUWBSerial.available()) { response += (char)pUWBSerial.read(); }
                break;
            }
        }
    }
    
    pSerial.print("Réponse du module UWB <- ");
    pSerial.println(response);
    return response;
}

String sendATCommandWithResult(String command, Stream & pSerial, Stream & pUWBSerial) {
    String resultData = "";
    
    pSerial.print("Envoi au module UWB (attente resultat) -> ");
    pSerial.println(command);
    
    while(pUWBSerial.available()) { pUWBSerial.read(); }
    pUWBSerial.println(command);
    
    uint32_t startTime = millis();
    const uint32_t maxGuardTimeout = 1000; 

    while ((millis() - startTime) < maxGuardTimeout) {
        if (pUWBSerial.available()) {
            String line = pUWBSerial.readStringUntil('\n');
            line.trim(); 
            
            if (line == "OK" || line == "ERROR") {
                break;
            }
            
            if (line.length() > 0 && line != command) {
                resultData = line;
            }
        }
    }
    
    pSerial.print("Résultat extrait <- ");
    pSerial.println(resultData != "" ? resultData : "[Aucun resultat/Erreur]");
    
    return resultData;
}

