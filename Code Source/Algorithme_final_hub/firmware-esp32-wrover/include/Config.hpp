#pragma once

// Mode de communication :
enum ModeCommunication {
    MODE_WIFI,
    MODE_WIRED
};
constexpr ModeCommunication MODE_ACTUEL = MODE_WIFI;

//Nombre maximum de Tags que le Hub peut gérer simultanément
constexpr int MAX_TAGS = 10;

//Temps d'une fenêtre pour la Recuperation des Donnees Ancres. (Par défaut on récupère tout et calcule toutes les 1/30s)
constexpr const unsigned long FENETRE_MS = 33;

//Début de la fenêtre des ids pour les tags :
constexpr int ID_DEPART_TAGS = 4;