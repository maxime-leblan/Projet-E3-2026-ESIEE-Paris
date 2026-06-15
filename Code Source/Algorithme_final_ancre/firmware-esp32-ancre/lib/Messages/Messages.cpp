#include "Messages.hpp"

bool parseUWBMessage(String rawData, MessageAncreHub& outMessage) {
    // 1. Recherche de l'ID du Tag (tid:X)
    int tidIndex = rawData.indexOf("tid:");
    if (tidIndex == -1) return false; // Trame incomplète ou invalide
    
    int commaAfterTid = rawData.indexOf(",", tidIndex);
    String tidStr = rawData.substring(tidIndex + 4, commaAfterTid);
    outMessage.tag_id = tidStr.toInt();

    // 2. Recherche de la parenthèse des distances range:(d0,d1,d2...)
    int rangeStart = rawData.indexOf("range:(");
    if (rangeStart == -1) return false;
    
    int rangeEnd = rawData.indexOf(")", rangeStart);
    String rangeStr = rawData.substring(rangeStart + 7, rangeEnd); 
    // rangeStr contient maintenant par exemple : "33,67,26,0,0,0,0,0"

    // 3. Découpage des 4 premières distances
    int startIdx = 0;
    for (int i = 0; i < 4; i++) {
        int commaIdx = rangeStr.indexOf(',', startIdx);
        if (commaIdx == -1) commaIdx = rangeStr.length(); // Gestion du dernier élément
        
        String valStr = rangeStr.substring(startIdx, commaIdx);
        
        // Conversion de la String en float + passage de centimètres à MÈTRES (/100)
        outMessage.distances[i] = valStr.toFloat() / 100.0f; 
        
        startIdx = commaIdx + 1; // Décalage du curseur après la virgule
    }

    outMessage.type = MSG_DISTANCES;
    return true; // Le colis outMessage est prêt à être envoyé
}

