#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Liste des états de la machine à états principale du Hub.
 */
enum HubState {
    HUB_STATE_IDLE = 0,                     ///< Attente d'une configuration ou d'une commande
    HUB_STATE_DETECTING_TAGS_FOR_INIT = 1,  ///< Phase de scan initial de tous les tags environnants
    HUB_STATE_COLLECTING_POINTS = 2,        ///< Phase de récupération de la zone d'exclusion
    HUB_STATE_GENERATING_GEOMETRY = 3,      ///< Calcul du plan initial (ancres + 64 points)
    HUB_STATE_RUNNING = 4                   ///< Mode surveillance actif à haute fréquence (33ms)
};

// Variables globales partagées, modifiables par l'UART (interceptions de commandes)
extern volatile HubState etatActuelHub;
extern volatile int idTagSelectionne;

/**
 * @brief Initialise l'UART de communication avec l'écran.
 */
void setupScreenCommunication();

/**
 * @brief Écoute l'UART et intercepte les paquets JSON pour changer l'état global du Hub.
 */
void loopScreenCommunication();

/**
 * @brief Envoie à l'écran la liste des tags détectés avec leurs distances respectives.
 * @param ids Tableau des IDs des tags trouvés
 * @param distances Tableau des distances associées en mètres
 * @param count Nombre de tags dans le tableau
 */
void envoyerListeTagsDecouverts(int ids[], float distances[], int count);

/**
 * @brief Envoie le plan géométrique initial calculé (4 ancres et 64 points de périmètre).
 * @param ancres Tableau 2D des coordonnées (X, Y) des 4 capteurs
 * @param points Tableau 2D des coordonnées (X, Y) des 64 points de la zone d'exclusion
 */
void envoyerGeometrieCalibration(float ancres[4][2], float points[64][2]);

/**
 * @brief Envoie le flux temps réel de position d'un tag (utilisé dans la boucle RUNNING).
 * @param ids IDs des tags concernés
 * @param xs Positions X calculées en mètres
 * @param ys Position sY calculées en mètres
 * @param distances Distances directes calculées par rapport au centre/véhicule
 * @param alarmes Statuts d'alertes (true si dans la zone d'exclusion)
 */
void envoyerMiseAJourTagsRuntime(int ids[], float xs[], float ys[], float distances[], bool alarmes[], int count);

