#include "UWBMessageManager.hpp"

bool decodeUWBMessage(const String &pRawMessage, UWBMessage &outMessage, Stream & pSerial) {

    // On vérifie si on a reçu un simple message de distance du module UWB lui-même
    if (pRawMessage.indexOf("AT+RANGE") != -1)
    {
        outMessage.senderId = 0;
        outMessage.receiverId = 0;
        outMessage.orderType = 0;
        outMessage.dataValue = 0.0f; // Pas de donnée supplémentaire pour les ordres 1 et 2
        outMessage.aIsStandardDistanceMessage = true;

        pSerial.println("[UWB DECOD] Message décodé de type AT+RANGE");
        return true;
    }

    // 1. Essayer de décoder comme un message de type DISTANCE (3)
    std::vector<float> distData = getFloatDataFromString(pRawMessage.c_str(), UWB_MESSAGE_DISTANCE_REGEX);
    if (!distData.empty()) {
        outMessage.senderId = getSenderIdFromUWBMessage(distData);
        outMessage.receiverId = getReceiverIdFromUWBMessage(distData);
        outMessage.orderType = getOrderTypeFromUWBMessage(distData);
        outMessage.dataValue = getDistanceFromUWBMessage(distData); // La distance est le 4ème élément
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
        outMessage.dataValue = 0.0f; // Pas de donnée supplémentaire pour les ordres 1 et 2
        outMessage.aIsStandardDistanceMessage = false;

        pSerial.println("[UWB DECOD] Message décodé de type 'Ordre du Hub pour le tag'");
        pSerial.println("[UWB DECOD] Type d'ordre = " + String(outMessage.orderType));
        pSerial.println("[UWB DECOD] ID émetteur = " + String(outMessage.senderId));
        pSerial.println("[UWB DECOD] ID destinataire = " + String(outMessage.receiverId));
        return true;
    }

    return false; // Échec du décodage si aucune regex ne correspond
}

bool receiveUWBMessage(Stream &pUWBSerial, String &outRawMessage, Stream & pSerial) {
    if (pUWBSerial.available()) {
        String input = pUWBSerial.readStringUntil('\n');
        input.trim(); // Nettoie les caractères invisibles (comme \r)

        pSerial.println("[UWB]  Message brut reçu du module UWB : " + input);
        outRawMessage = input;
        pSerial.println("[UWB] Résultat stocké dans la variable de sortie : " + outRawMessage);
        return true;
    }
    return false;
}

/*
bool receiveUWBDistanceMessage(Stream &pUWBSerial, Stream & pSerial , String &outRawMessage)
{
    if (pUWBSerial.available()) {
        String input = pUWBSerial.readStringUntil('\n');
        input.trim(); // Nettoie les caractères invisibles (comme \r)
        pSerial.println("[UWB]  Message brut reçu du module UWB : " + input);
        outRawMessage = input;
        pSerial.println("[UWB] Résultat stocké dans la variable de sortie : " + outRawMessage);
        return true;
    }
    return false;
}
    */

/*
void readDistancesInTagSerial(Stream & pUWBSerial, Stream & pSerial, String &outRawMessage)
{
    if (pUWBSerial.available()) {
        String data = pUWBSerial.readStringUntil('\n');
        data.trim();

        outRawMessage = data;

        if (data != "")
        {
            pSerial.println("Tag vient de lire le message distance de son UWBSerial : " + data);
        }
        else
        {
            pSerial.println("Le UWBSerial du Tag est vide, distance impossible à lire : " + data);
        }
    }
    else 
    {
        pSerial.println("Pas de message dans la file du tag");
    }
}
*/

void sendOrderToTag(Stream &pUWBSerial, Stream & pSerial, uint8_t pSenderID, uint8_t pReceiverID, uint8_t pOrderType) {
    // Construction du message : ex "0:4:1"
    String message = String(pSenderID) + ":" + String(pReceiverID) + ":" + String(pOrderType);
    String atCommand = "AT+DATA=" + String(message.length()) + "," + message;
    pUWBSerial.println(atCommand);

    pSerial.println("[UWB SEND A->T] Ancre " + String(pSenderID) + " a envoyé à Tag " + String(pReceiverID) + "le message : " + atCommand);
}

void sendDistanceToTag(Stream &pUWBSerial, Stream & pSerial, uint8_t pSenderID, uint8_t pReceiverID, float pDistance) {
    // Construction du message : ex "0:4:3:5.45"
    String message = String(pSenderID) + ":" + String(pReceiverID) + ":3:" + String(pDistance, 2);
    String atCommand = "AT+DATA=" + String(message.length()) + "," + message;
    pUWBSerial.println(atCommand);

    pSerial.println("[UWB SEND A->T] Ancre " + String(pSenderID) + " a envoyé à Tag " + String(pReceiverID) + "le message : " + atCommand);

}

void sendDistancesToAnchor(Stream & pUWBSerial, Stream & pSerial, String & pRawRangeMessage) {
    // 3. Construction de la commande d'envoi radio
    // La syntaxe est : AT+DATA=<LongueurDuMessage>,<MessageBrut>
    String atCommand = "AT+DATA=" + String(pRawRangeMessage.length()) + "," + pRawRangeMessage;
    
    // 4. Envoi au module UWB pour diffusion aux ancres
    pUWBSerial.println(atCommand);
    
    // Affichage de confirmation dans le moniteur série
    pSerial.println("[UWB SEND T->A] Tag a envoyé : " + atCommand);

}

/*
void configureUWBForMessaging(Stream &pUWBSerial) {
    // AT+SETCAP=(x1),(x2),(x3)
    // x3 = 1 active le mode "Extended packet" indispensable pour AT+DATA
    // On garde des valeurs par défaut pour x1 et x2 (ex: 10, 10)
    
    pUWBSerial.println("AT+SETCAP=10,10,1");
    delay(500); // Temps pour que le module traite la commande
    
    pUWBSerial.println("AT+SAVE");
    delay(500); // Temps pour la sauvegarde en mémoire flash
}
    */

String sendATCommand(String command, Stream & pSerial, Stream & pUWBSerial) {
    String response = "";
    pSerial.print("Envoi au module UWB -> "); 
    pSerial.println(command);
    
    while(pUWBSerial.available()) { pUWBSerial.read(); }
    pUWBSerial.println(command);
    
    uint32_t startTime = millis();
    const uint32_t maxGuardTimeout = 1000; // Augmenté à 1 seconde de sécurité

    while ((millis() - startTime) < maxGuardTimeout) {
        if (pUWBSerial.available()) {
            char c = (char)pUWBSerial.read();
            response += c;
            
            // CORRECTION : On attend UNIQUEMENT le OK ou ERROR final, on ignore les sauts de lignes simples.
            if (response.endsWith("OK\r\n") || response.endsWith("ERROR\r\n")) {
                delay(5); // On laisse 5ms pour ramasser les derniers caractères résiduels
                while (pUWBSerial.available()) { response += (char)pUWBSerial.read(); }
                break; 
            }
        }
    }
    
    pSerial.print("Réponse du module UWB <- "); 
    pSerial.println(response); 
    return response;
}

