#include "GestionnaireAncres.hpp"

RegistreAncre carnetAdresses[ANCHORS_NUMBER];
int nombreAncresConnues = 0;

void initGestionnaireAncres() {
    for (int i = 0; i < ANCHORS_NUMBER; i++) {
        carnetAdresses[i].estEnregistree = false;
        carnetAdresses[i].uwb_id = -1;
        memset(carnetAdresses[i].mac, 0, 6);
    }
    nombreAncresConnues = 0;
}

int attribuerOuRecupererIdAncre(const uint8_t adresseHardware[6]) {
    // 1. Connaît-on déjà cette carte ?
    for (int i = 0; i < ANCHORS_NUMBER; i++) {
        if (carnetAdresses[i].estEnregistree && memcmp(carnetAdresses[i].mac, adresseHardware, 6) == 0) {
            Serial.printf("-> Carte deja connue ! C'est l'Ancre %d\n", carnetAdresses[i].uwb_id);
            return carnetAdresses[i].uwb_id;
        }
    }

    // 2. Si c'est une nouvelle, on l'ajoute dans une case vide
    if (nombreAncresConnues < ANCHORS_NUMBER) {
        int nouvel_id = nombreAncresConnues;
        
        memcpy(carnetAdresses[nouvel_id].mac, adresseHardware, 6);
        carnetAdresses[nouvel_id].uwb_id = nouvel_id;
        carnetAdresses[nouvel_id].estEnregistree = true;
        
        nombreAncresConnues++;
        Serial.printf("-> Nouvelle carte enregistree sous l'ID Ancre %d\n", nouvel_id);
        
        return nouvel_id;
    }

    // 3. Le carnet est plein
    Serial.println("-> Erreur : Impossible d'attribuer un ID, carnet plein !");
    return -1;
}

bool recupererAdresseParId(int uwb_id_recherche, uint8_t adresseOut[6]) {
    for (int i = 0; i < ANCHORS_NUMBER; i++) {
        if (carnetAdresses[i].estEnregistree && carnetAdresses[i].uwb_id == uwb_id_recherche) {
            memcpy(adresseOut, carnetAdresses[i].mac, 6);
            return true; // Trouvé !
        }
    }
    return false; // Introuvable
}

