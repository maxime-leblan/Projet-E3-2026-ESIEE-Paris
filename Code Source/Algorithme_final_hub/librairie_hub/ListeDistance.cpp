#include "ListeDistanceLibrary.hpp"
#include "HTTPClient.h"
#include <ArduinoJson.h>

namespace ListeDistance {

    // --- Étape 1 : Calcul du centre des 4 ancres ---
    V3 obtenirCentreAncres(UWBModuleList& pAncres) {
        // On récupère la liste des identifiants des ancres présentes
        std::vector<int> idList = pAncres.giveModuleIdList();
        
        if (idList.empty()) {
            return V3(0.0f, 0.0f, 0.0f); // Sécurité si la liste est vide
        }

        float sumX = 0.0f;
        float sumY = 0.0f;
        float sumZ = 0.0f;

        // On additionne les coordonnées de chaque ancre
        for (int id : idList) {
            V3 posAncre = pAncres.getModule(id).getPosition();
            sumX += posAncre.getX();
            sumY += posAncre.getY();
            sumZ += posAncre.getZ();
        }

        // On fait la moyenne pour trouver le centre exact
        float nbAncres = (float)idList.size();
        return V3(sumX / nbAncres, sumY / nbAncres, sumZ / nbAncres);
    }

    // --- Étape 2 : Calcul, Tri et Envoi UART ---
    void envoyerDistancesTrieesUART(
        UWBModuleList& pAncres, 
        const std::unordered_map<int, V3>& pPositionsTags) 
    {
        // Si aucun tag n'a été détecté/calculé, on ne transmet rien
        if (pPositionsTags.empty()) {
            Serial.println("UART_HUB: Aucun tag détecté.");
            return;
        }

        // 1. On récupère le point central des 4 ancres
        V3 centreAncres = obtenirCentreAncres(pAncres);

        // 2. On crée un tableau dynamique pour stocker nos résultats avant le tri
        std::vector<TagTrie> listeA_Trier;

        // 3. On parcourt le dictionnaire de tous les tags calculés
        for (const auto& paire : pPositionsTags) {
            int tagId = paire.first;
            V3 posTag = paire.second;

            // Calcul du vecteur différence (utilise l'opérateur - défini dans V3.h)
            V3 diff = posTag - centreAncres;
            
            // Calcul de la distance réelle grâce à la norme du vecteur (défini dans V3.h)
            float distance = diff.norm(); 

            // On enregistre le tag dans notre liste
            listeA_Trier.push_back({tagId, distance, posTag});
        }

        // 4. ALGORITHME DE TRI (De la plus petite distance à la plus grande)
        // Utilisation de std::sort combiné à une fonction lambda C++
        std::sort(listeA_Trier.begin(), listeA_Trier.end(), [](const TagTrie& a, const TagTrie& b) {
            return a.distanceAuCentre < b.distanceAuCentre; // Trie par ordre croissant
        });

        /*
        // 5. ENVOI DES DONNÉES SUR LE PORT UART (Serial)
        Serial.println("--- DEBUT TRAME UART HUB ---");
        Serial.printf("Centre Ancres calculé: X=%.2f Y=%.2f Z=%.2f\n", centreAncres.getX(), centreAncres.getY(), centreAncres.getZ());
        
        for (const auto& tag : listeA_Trier) {
            Serial.printf("TAG_ID:%d | DIST_CENTRE:%.2f m | POS: X=%.2f Y=%.2f Z=%.2f\n", 
                          tag.id, 
                          tag.distanceAuCentre, 
                          tag.position.getX(), 
                          tag.position.getY(), 
                          tag.position.getZ());
        }
        Serial.println("--- FIN TRAME UART HUB ---");
        */

        // 5. ENVOI DES DONNÉES SUR LE PORT UART (Serial)

        JsonDocument doc;

        for (const auto& tag : listeA_Trier) {
            JsonObject obj = doc.add<JsonObject>();
            obj["id"] = tag.id;

            // On arrondit à 2 décimales pour éviter les chiffres trop longs 
            obj["dist"] = round(tag.distanceAuCentre * 100.0) / 100.0; 
            obj["x"] = round(tag.position.getX() * 100.0) / 100.0; 
            obj["y"] = round(tag.position.getY() * 100.0) / 100.0; 
            obj["z"] = round(tag.position.getZ() * 100.0) / 100.0; 
        } 
    
        // On transforme l'objet en vrai texte compréhensible par le Wi-Fi 
        String json; 
        serializeJson(doc, json); 
        
        // --- ENVOI DE LA REQUÊTE --- 
        HTTPClient http; 
        http.begin("http://192.168.4.1/api/update"); 
        http.addHeader("Content-Type", "application/json"); 
        int httpResponseCode = http.POST(json); 
        
        // Retour visuel 
        if (httpResponseCode > 0) { 
            Serial.printf("Envoi WiFi OK (Code %d) -> %s\n", httpResponseCode, json.c_str()); 
        } else { 
            Serial.printf("Erreur Envoi WiFi (Code %d)\n", httpResponseCode); 
        } 
        http.end();
    }

}

