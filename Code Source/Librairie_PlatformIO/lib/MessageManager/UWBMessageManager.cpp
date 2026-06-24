#include "UWBMessageManager.hpp"

#include "UWBMessageManager.hpp"

#include "UWBMessageManager.hpp"

bool decodeUWBMessage(const String &pRawMessage, UWBMessage &outMessage, Stream & pSerial) {
    String messageATraiter = pRawMessage;
    messageATraiter.trim();

    if (messageATraiter.length() == 0) return false;

    // ====================================================================
    // CAS 1 : C'est une trame locale de Ping/Pong générée par le module
    // (L'Ancre en a BESOIN, le Tag l'ignorera dans son main)
    // ====================================================================
    if (messageATraiter.indexOf("AT+RANGE") != -1) {
        
        // Sécurité : Si deux messages se collent, on coupe au 2ème
        int secondRange = messageATraiter.indexOf("AT+RANGE", messageATraiter.indexOf("AT+RANGE") + 8);
        if (secondRange != -1) {
            messageATraiter = messageATraiter.substring(0, secondRange);
            messageATraiter.trim();
        }

        outMessage.senderId = 0;
        outMessage.receiverId = 0;
        outMessage.orderType = 0;
        outMessage.dataValue = 0.0f;
        outMessage.pressionPa = 0; // On a retiré le Tag TX, donc plus de pression RF
        outMessage.aIsStandardDistanceMessage = true; 

        // pSerial.println("[UWB DECOD] Trame AT+RANGE locale détectée.");
        return true;
    }

    // ====================================================================
    // CAS 2 : C'est un message UWB personnalisé (AT+RDATA ou +RDATA)
    // ====================================================================
    int rdataIndex = messageATraiter.indexOf("AT+RDATA=");
    if (rdataIndex == -1) rdataIndex = messageATraiter.indexOf("+RDATA=");

    if (rdataIndex != -1) {
        int commaCount = 0;
        int payloadStart = -1;
        // Correction du warning unsigned/signed
        for (unsigned int i = rdataIndex; i < messageATraiter.length(); i++) {
            if (messageATraiter[i] == ',') {
                commaCount++;
                if (commaCount == 4) {
                    payloadStart = i + 1;
                    break;
                }
            }
        }
        
        if (payloadStart != -1) {
            messageATraiter = messageATraiter.substring(payloadStart);
            messageATraiter.trim();
            // pSerial.println("[UWB DECOD] Payload utile extrait : " + messageATraiter);
        } else {
            return false;
        }
    }

    // ====================================================================
    // CAS 3 : Décodage du Payload via Regex
    // ====================================================================
    std::vector<float> extractedData = getFloatDataFromString(messageATraiter.c_str(), "[+-]?([0-9]*[.])?[0-9]+");

    if (extractedData.size() >= 3) {
        outMessage.senderId = getSenderIdFromUWBMessage(extractedData);
        outMessage.receiverId = getReceiverIdFromUWBMessage(extractedData);
        outMessage.orderType = getOrderTypeFromUWBMessage(extractedData);
        outMessage.aIsStandardDistanceMessage = false;

        if (extractedData.size() >= 4) {
            outMessage.dataValue = getDistanceFromUWBMessage(extractedData);
            pSerial.printf("[UWB DECOD] Payload DISTANCE : Emetteur=%d | Destinataire=%d | Ordre=%d | Dist=%.2f\n", 
                           outMessage.senderId, outMessage.receiverId, outMessage.orderType, outMessage.dataValue);
        } else {
            outMessage.dataValue = 0.0f;
            pSerial.printf("[UWB DECOD] Payload ORDRE : Emetteur=%d | Destinataire=%d | Ordre=%d\n", 
                           outMessage.senderId, outMessage.receiverId, outMessage.orderType);
        }
        return true;
    }

    return false;
}

bool receiveUWBMessage(Stream &pUWBSerial, String &outRawMessage, Stream & pSerial) {
    if (pUWBSerial.available()) {
        String input = pUWBSerial.readStringUntil('\n');
        input.trim(); // Nettoie les caractères invisibles (comme \r)

        //pSerial.println("[UWB]  Message brut reçu du module UWB : " + input);
        outRawMessage = input;
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

void sendDistancesWithPressionToAnchor(Stream & pUWBSerial, Stream & pSerial, String & pRawRangeMessage, float pPression_hPa) {
    pSerial.println("\nON SKIP CARRMENT L'ENVOI\n");
    return;
    // 3. Construction de la commande d'envoi radio
    // La syntaxe est : AT+DATA=<LongueurDuMessage>,<MessageBrut>

    String vPressionStr = String((long)round(pPression_hPa * 100.0)); 
    String vMessage = vPressionStr + " & " + pRawRangeMessage;
    String atCommand = "AT+DATA=" + String(vMessage.length()) + "," + vMessage;
    
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

String sendATCommandWithResult(String command, Stream & pSerial, Stream & pUWBSerial) {
    String resultData = "";
    
    pSerial.print("Envoi au module UWB (attente resultat) -> "); 
    pSerial.println(command);
    
    // 1. On vide le buffer des anciens messages fantômes
    while(pUWBSerial.available()) { pUWBSerial.read(); }
    
    // 2. On envoie la commande
    pUWBSerial.println(command);
    
    // 3. On lit la réponse ligne par ligne
    uint32_t startTime = millis();
    const uint32_t maxGuardTimeout = 1000; // 1 seconde de sécurité maximum

    while ((millis() - startTime) < maxGuardTimeout) {
        if (pUWBSerial.available()) {
            // On lit le texte jusqu'au prochain saut de ligne
            String line = pUWBSerial.readStringUntil('\n');
            line.trim(); // On supprime les espaces et les retours chariots invisibles (\r)
            
            // Si on lit "OK" ou "ERROR", la conversation est terminée, on sort de la boucle
            if (line == "OK" || line == "ERROR") {
                break;
            }
            
            // Si la ligne n'est pas vide ET qu'elle n'est pas l'écho de notre propre commande
            if (line.length() > 0 && line != command) {
                // C'est la ligne contenant la donnée utile ! On la sauvegarde.
                resultData = line;
            }
        }
    }
    
    pSerial.print("Résultat extrait <- "); 
    pSerial.println(resultData != "" ? resultData : "[Aucun resultat/Erreur]"); 
    
    return resultData;
}