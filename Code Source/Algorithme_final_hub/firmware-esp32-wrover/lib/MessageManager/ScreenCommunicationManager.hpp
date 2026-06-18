#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Initialise le module de communication spécifique avec l'écran Matouch.
 * * Configure l'UART dédié, configure les broches de liaison matérielle et prépare
 * les buffers internes pour l'échange de messages avec l'écran.
 */
void setupScreenCommunication();

/**
 * @brief Gère la boucle de traitement asynchrone des messages de l'écran.
 * * Cette fonction doit être appelée à chaque itération de la boucle principale (loop).
 * Elle vérifie la réception de commandes HTTP relayées, supervise l'état de la calibration
 * et gère l'envoi périodique des positions des tags.
 */
void loopScreenCommunication();

/**
 * @brief Analyse et exécute les commandes centralisées reçues depuis l'interface de l'écran ou du téléphone.
 * * @param doc Référence vers le document JSON parsé contenant la structure de la commande ("cmd").
 */
void processIncomingCommand(JsonDocument& doc);

/**
 * @brief Envoie l'état périodique des tags détectés par le Hub vers l'écran.
 * * Génère un flux JSON contenant l'identifiant de chaque tag, ses coordonnées cartésiennes
 * en mètres ainsi que son statut d'alerte.
 */
void sendTagsToScreen();

/**
 * @brief Transmet les résultats géométriques d'un calibrage matériel validé.
 * * Envoie un bloc structuré contenant la liste des points de la zone d'exclusion ainsi que
 * l'emplacement des capteurs/ancres mesurés sur le véhicule.
 */
void sendCalibrationDataToScreen();

