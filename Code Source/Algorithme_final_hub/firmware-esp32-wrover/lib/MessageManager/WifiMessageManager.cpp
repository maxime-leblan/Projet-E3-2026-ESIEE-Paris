#include "WifiMessageManager.hpp"
#include "GestionnaireAncres.hpp"

// Le PONT vers le main.cpp pour transmettre les distances lues
extern void OnWifidataReceived(int tagId, float d0, float d1, float d2, float d3);

void initWifi() {
    // Le mode WIFI_STA est obligatoire pour ESP-NOW
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Erreur d'initialisation ESP-NOW");
        return;
    }
    Serial.print("Hub WiFi initialise. MAC : ");
    Serial.println(WiFi.macAddress());
 
    // Déclaration des fonctions de callback
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Affichage optionnel pour voir si l'Ancre a bien reçu l'ordre
    // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Ordre delivre" : "Echec transmission");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    uint8_t typeMessage = incomingData[0];

    // ==========================================
    // CAS 1 : UNE ANCRE S'ALLUME (MSG_INIT)
    // ==========================================
    if (typeMessage == MSG_INIT) {
        Serial.print("Nouvelle Ancre detectee via Wi-Fi ! MAC : ");
        for(int i=0; i<6; i++) Serial.printf("%02X:", mac[i]);
        Serial.println();

        // 1. On interroge le Cerveau (le Gestionnaire d'Ancres)
        int id_attribue = attribuerOuRecupererIdAncre(mac);

        // 2. Si on a un ID valide, on gère la connexion Wi-Fi et la réponse
        if (id_attribue != -1) {
            
            // On autorise le Hub à lui parler en ESP-NOW (S'il n'est pas déjà enregistré)
            if (!esp_now_is_peer_exist(mac)) {
                esp_now_peer_info_t peerInfo = {};
                memcpy(peerInfo.peer_addr, mac, 6);
                peerInfo.channel = 0;
                peerInfo.encrypt = false;
                esp_now_add_peer(&peerInfo);
            }
           
            // On lui renvoie sa configuration
            MessageHubAncre reponse;
            reponse.command = 2; // Code 2 = Configuration UWB
            reponse.uwb_id = id_attribue;
            esp_now_send(mac, (uint8_t *)&reponse, sizeof(reponse));
        }
    }
   
    // ==========================================
    // CAS 2 : RECEPTION DES DISTANCES
    // ==========================================
    else if (typeMessage == MSG_DISTANCES) {
        MessageAncreHub donnees;
        memcpy(&donnees, incomingData, sizeof(donnees));
       
        /* Commenter le print si ça spam trop le moniteur série
        Serial.printf("Distances recues (Tag %d) : A0:%.2f | A1:%.2f | A2:%.2f | A3:%.2f\n",
            donnees.tag_id, donnees.distances[0], donnees.distances[1],
            donnees.distances[2], donnees.distances[3]);
        */
           
        // -> ENVOI VERS LE MAIN.CPP POUR ACCUMULATION ET CALCUL
        OnWifidataReceived(donnees.tag_id, donnees.distances[0], donnees.distances[1], donnees.distances[2], donnees.distances[3]);
    }
}

void envoyerOrdreChangementRole(int pUwbId, uint8_t pCommand) {
    uint8_t macCible[6];

    // On interroge le Gestionnaire d'Ancres pour avoir l'adresse
    if (recupererAdresseParId(pUwbId, macCible)) {
        MessageHubAncre ordre;
        ordre.command = pCommand;
        ordre.uwb_id = pUwbId;

        esp_now_send(macCible, (uint8_t *)&ordre, sizeof(ordre));
    } else {
        Serial.printf("Impossible d'envoyer l'ordre : Ancre %d introuvable dans le registre.\n", pUwbId);
    }
}