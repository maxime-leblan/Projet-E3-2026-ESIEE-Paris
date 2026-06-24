#pragma once

#include <Arduino.h>
#include <string>
#include <vector>

// Regex conservées pour le fonctionnement hors-calibration
#define UWB_MESSAGE_ORDER_REGEX "[0-9]+:[0-9]+:[0-9]+"
#define UWB_MESSAGE_DISTANCE_REGEX "[0-9]+:[0-9]+:[0-9]+:([+-]?(?=\\.\\d|\\d)(?:\\d+)?(?:\\.?\\d*))(?:[Ee]([+-]?\\d+))?"

#define UWB_MESSAGE_SENDER_ID_INDEX 0
#define UWB_MESSAGE_RECEIVER_ID_INDEX 1
#define UWB_MESSAGE_ORDER_TYPE_INDEX 2
#define UWB_MESSAGE_DISTANCE_INDEX 3

// Structure propre pour stocker une trame UWB Range décodée (Définie UNE SEULE FOIS ici)
struct UWBFrameRange {
    bool isValid;
    int tid;
    int mask;
    int seq;
    int distances[8];
};

/**
 * Extrait PROPREMENT les données d'une trame AT+RANGE=...
 */
UWBFrameRange parseUWBRangeMessage(const std::string& pRawMessage);

// --- Les anciennes fonctions gardées pour la compatibilité ---
std::vector<float> getFloatDataFromString(const std::string& texte, const std::string& patternRegex);
float getDistanceFromUWBMessage(std::vector<float> pUWBMessageData);
int getOrderTypeFromUWBMessage(std::vector<float> pUWBMessageData);
int getReceiverIdFromUWBMessage(std::vector<float> pUWBMessageData);
int getSenderIdFromUWBMessage(std::vector<float> pUWBMessageData);

