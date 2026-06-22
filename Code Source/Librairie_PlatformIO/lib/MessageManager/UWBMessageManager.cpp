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
        if (input.startsWith("AT+RDATA")) {
            // Le format AT+RDATA contient souvent des infos système, 
            // on cherche la partie "données" que vous avez envoyée
            outRawMessage = input; 
            return true;
        }
    }
    return false;
}

bool receiveUWBDistanceMessage(Stream &pUWBSerial, Stream & pSerial , String &outRawMessage)
{
    if (pUWBSerial.available()) {
        String input = pUWBSerial.readStringUntil('\n');
        input.trim(); // Nettoie les caractères invisibles (comme \r)
        pSerial.println("[UWB]  Message brut reçu du module UWB : " + input);
        // On cherche le mot clé RANGE ou range de votre TAG_DATA_REGEX
        if (input.startsWith("AT+RDATA") || input.indexOf("range:") != -1) {
            outRawMessage = input;
            return true;
        }
    }
    //pSerial.println("[UWB] Aucun message de distance reçu du module UWB.");
    return false;
}

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

void sendDistancesToAnchor(Stream & pUWBSerial, Stream & pSerial) {
    // 1. On vérifie si le module UWB matériel a généré une nouvelle ligne
    if (pUWBSerial.available()) {
        String rawData = pUWBSerial.readStringUntil('\n');
        rawData.trim(); // Nettoie les caractères invisibles (comme \r)

        // 2. Sécurité : on vérifie que la ligne ressemble à une distance.
        // D'après votre capture, les trames contiennent "an" (pour ancre) et "m" (pour mètre).
        // Cela évite de ré-envoyer par erreur les "OK" ou les messages de démarrage.
        if (rawData.indexOf("an") != -1 && rawData.indexOf("m") != -1) {
            
            // Affichage dans le moniteur série du Tag (pour votre suivi)
            pSerial.println("Tag a lu : " + rawData);

            // 3. Construction de la commande d'envoi radio
            // La syntaxe est : AT+DATA=<LongueurDuMessage>,<MessageBrut>
            String atCommand = "AT+DATA=" + String(rawData.length()) + "," + rawData;
            
            // 4. Envoi au module UWB pour diffusion aux ancres
            pUWBSerial.println(atCommand);
            
            // Affichage de confirmation dans le moniteur série
            pSerial.println("Tag a envoye par radio -> " + atCommand);
        }
    }
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

String sendATCommand(String command, Stream & pSerial, Stream & pUWBSerial) {
    String response = "";
    pSerial.print("Envoi au module UWB -> "); 
    pSerial.println(command);
    
    // 1. On vide le buffer de réception des trames précédentes pour éviter les résidus
    while(pUWBSerial.available()) { pUWBSerial.read(); }

    // 2. Envoi de la commande
    pUWBSerial.println(command);
    
    // 3. Attente intelligente de la réponse
    uint32_t startTime = millis();
    const uint32_t maxGuardTimeout = 400; // Sécurité maximale de 400ms si le module crash ou est débranché

    while ((millis() - startTime) < maxGuardTimeout) {
        if (pUWBSerial.available()) {
            char c = (char)pUWBSerial.read();
            response += c;
            
            // DÈS QU'ON DÉTECTE LA FIN DE LA RÉPONSE OFFICIELLE DU FIRMWARE MAKERFABS :
            // Le firmware répond toujours par "OK\r\n" ou "ERR...\r\n" ou "range:...\r\n"
            if (response.endsWith("OK\r\n") || response.endsWith("ERROR\r\n") || response.indexOf("\r\n") != -1) {
                // Optionnel : On peut laisser un micro delay (1-2ms) pour être sûr d'avoir tout ramassé
                delay(2);
                while (pUWBSerial.available()) { response += (char)pUWBSerial.read(); }
                break; // ON SORT TOUT DE SUITE ! Pas de temps perdu.
            }
        }
    }
    
    pSerial.print("Réponse du module UWB <- "); 
    pSerial.println(response); 
    return response;
}
