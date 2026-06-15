#include "WifiMessageManager.hpp"

// Le carnet d'adresses qui stocke jusqu'à 4 ancres (0, 1, 2, 3)
RegistreAncre carnetAdresses[ANCHORS_NUMBER] = {0};
int nombreAncresConnues = 0;

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
        Serial.print("Nouvelle Ancre detectee ! MAC : ");
        for(int i=0; i<6; i++) Serial.printf("%02X:", mac[i]);
        Serial.println();

        int id_attribue = -1;

        // 1. Connaît-on déjà cette carte ?
        for (int i = 0; i < ANCHORS_NUMBER; i++) {
            if (carnetAdresses[i].estEnregistree && memcmp(carnetAdresses[i].mac, mac, 6) == 0) {
                id_attribue = carnetAdresses[i].uwb_id;
                Serial.printf("-> On la connait ! C'est l'Ancre %d\n", id_attribue);
                break;
            }
        }

        // 2. Si c'est une nouvelle, on l'ajoute
        if (id_attribue == -1 && nombreAncresConnues < ANCHORS_NUMBER) {
            id_attribue = nombreAncresConnues; 
            
            memcpy(carnetAdresses[nombreAncresConnues].mac, mac, 6);
            carnetAdresses[nombreAncresConnues].uwb_id = id_attribue;
            carnetAdresses[nombreAncresConnues].estEnregistree = true;
            
            // On autorise le Hub à lui parler
            esp_now_peer_info_t peerInfo = {};
            memcpy(peerInfo.peer_addr, mac, 6);
            peerInfo.channel = 0;
            peerInfo.encrypt = false;
            esp_now_add_peer(&peerInfo);
            
            nombreAncresConnues++;
        }

        // 3. On lui renvoie sa configuration
        if (id_attribue != -1) {
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
        
        Serial.printf("Distances reçues (Tag %d) : A0:%.2f | A1:%.2f | A2:%.2f | A3:%.2f\n", 
            donnees.tag_id, donnees.distances[0], donnees.distances[1], 
            donnees.distances[2], donnees.distances[3]);
            
        // Plus tard, tu appelleras ta trilatération ici !
    }
}

void envoyerOrdreChangementRole(int pUwbId, uint8_t pCommand) {
    bool ancreTrouvee = false;
    uint8_t macCible[6];

    for (int i = 0; i < ANCHORS_NUMBER; i++) {
        if (carnetAdresses[i].estEnregistree && carnetAdresses[i].uwb_id == pUwbId) {
            memcpy(macCible, carnetAdresses[i].mac, 6);
            ancreTrouvee = true;
            break;
        }
    }

    if (!ancreTrouvee) return;

    MessageHubAncre ordre;
    ordre.command = pCommand;
    ordre.uwb_id = pUwbId;

    esp_now_send(macCible, (uint8_t *)&ordre, sizeof(ordre));
}