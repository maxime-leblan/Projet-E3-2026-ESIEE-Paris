#pragma once
#include <Arduino.h>

// Le nombre maximum d'ancres gérées par le Hub
#define ANCHORS_NUMBER 4

// --- STRUCTURE DU REGISTRE --- 
// Contient l'ensemble des adresses matérielles (MAC ou CAN) des ancres
struct RegistreAncre {
    uint8_t mac[6];      
    int uwb_id;          
    bool estEnregistree;
};

// --- PROTOTYPES DU GESTIONNAIRE ---
void initGestionnaireAncres();

// Demande une identification. Renvoie l'ID (0 à 3) ou -1 si impossible.
int attribuerOuRecupererIdAncre(const uint8_t adresseHardware[6]);

// Permet aux managers réseau de récupérer l'adresse d'une ancre pour lui parler
bool recupererAdresseParId(int uwb_id_recherche, uint8_t adresseOut[6]);
