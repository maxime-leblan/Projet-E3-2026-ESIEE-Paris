#include "UWBDataManager.hpp"
#include <regex>
#include <Arduino.h> // Ajout crucial pour utiliser Serial sur l'ESP32

UWBFrameRange parseUWBRangeMessage(const std::string& pRawMessage) {
    UWBFrameRange frame;
    frame.isValid = false;

    // sscanf va calquer ce patron exact sur la chaîne. S'il arrive à extraire les 11 valeurs
    // (le tid, le mask, le seq, et les 8 distances), il renverra 11.
    int parsedItems = sscanf(pRawMessage.c_str(), 
        "AT+RANGE=tid:%d,mask:%x,seq:%d,range:(%d,%d,%d,%d,%d,%d,%d,%d)",
        &frame.tid, 
        &frame.mask, 
        &frame.seq,
        &frame.distances[0], &frame.distances[1], &frame.distances[2], &frame.distances[3],
        &frame.distances[4], &frame.distances[5], &frame.distances[6], &frame.distances[7]
    );

    if (parsedItems == 11) {
        frame.isValid = true;
    } else {
        // Remplacement de std::cerr par Serial.println pour que tu puisses le voir sur ton moniteur
        Serial.printf("[UWB PARSING] Erreur de format UWB. Elements lus : %d/11\n", parsedItems);
    }

    return frame;
}

// ====================================================================
// ANCIENNES FONCTIONS (Conservées pour getFloatDataFromString)
// ====================================================================
std::vector<float> getFloatDataFromString(const std::string& texte, const std::string& patternRegex) {
    std::vector<float> entiersExtraits;
    try {
        std::regex regex(patternRegex);
        auto debut_match = std::sregex_iterator(texte.begin(), texte.end(), regex);
        auto fin_match = std::sregex_iterator();

        for (std::sregex_iterator i = debut_match; i != fin_match; ++i) {
            std::smatch match = *i;
            try {
                entiersExtraits.push_back(std::stof(match.str()));
            }
            catch (const std::exception& e) {
                // Erreur de conversion ignorée silencieusement
            }
        }
    }
    catch (const std::regex_error& e) {
        // Remplacement de std::cerr par Serial.print
        Serial.print("[REGEX ERREUR] Erreur de syntaxe : ");
        Serial.println(e.what());
    }
    return entiersExtraits;
}

float getDistanceFromUWBMessage(std::vector<float> pUWBMessageData) { return pUWBMessageData[UWB_MESSAGE_DISTANCE_INDEX]; }
int getOrderTypeFromUWBMessage(std::vector<float> pUWBMessageData) { return (int)pUWBMessageData[UWB_MESSAGE_ORDER_TYPE_INDEX]; }
int getReceiverIdFromUWBMessage(std::vector<float> pUWBMessageData) { return (int)pUWBMessageData[UWB_MESSAGE_RECEIVER_ID_INDEX]; }
int getSenderIdFromUWBMessage(std::vector<float> pUWBMessageData) { return (int)pUWBMessageData[UWB_MESSAGE_SENDER_ID_INDEX]; }

