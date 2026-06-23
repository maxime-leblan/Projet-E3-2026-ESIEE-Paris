#pragma once

// Mode de communication :
enum ModeCommunication {
    MODE_WIFI,
    MODE_WIRED
};
constexpr ModeCommunication MODE_ACTUEL = MODE_WIRED;

//Nombre maximum de Tags que le Hub peut gérer simultanément
constexpr int MAX_TAGS = 6;

//Début de la fenêtre des ids pour les tags :
constexpr int ID_DEPART_TAGS = 4;