#include "Messages.hpp"

bool parseUWBMessage(String rawData, MessageAncreHub& outMessage) {
    // 1. Recherche de l'ID du Tag (tid:X)
    int tidIndex = rawData.indexOf("tid:");
    if (tidIndex == -1) return false; // Trame incomplète ou invalide
    
    int commaAfterTid = rawData.indexOf(",", tidIndex);
    String tidStr = rawData.substring(tidIndex + 4, commaAfterTid);
    outMessage.tag_id = tidStr.toInt();

    // 2. Recherche de la parenthèse des distances (Support dynamique)
    // On cherche en priorité "r:(" tel que généré par le Tag optimisé
    int rangeStart = rawData.indexOf("r:(");
    int offset = 3; // Nombre de caractères à sauter pour "r:("
    
    // Rétrocompatibilité : Si "r:(" n'est pas trouvé, on cherche l'ancien format "range:("
    if (rangeStart == -1) {
        rangeStart = rawData.indexOf("range:(");
        offset = 7; // Nombre de caractères à sauter pour "range:("
        
        // Si aucun des deux n'est trouvé, la trame est invalide
        if (rangeStart == -1) return false; 
    }
    
    int rangeEnd = rawData.indexOf(")", rangeStart);
    
    // On extrait uniquement les chiffres grâce à l'offset calculé ci-dessus
    String rangeStr = rawData.substring(rangeStart + offset, rangeEnd); 
    // rangeStr contient maintenant par exemple : "77,0,96,0"

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