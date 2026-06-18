#include "HubDataStorage.hpp"
#include "WifiMessageManager.hpp"
#include <ArduinoEigenDense.h>
#include <ArduinoJson.h>

//Hugues : Test communications via WiFi.
#include <WiFi.h>
#include <HTTPClient.h>
//\Hugues

#include "Trilateration.h"
#include "GridLibrary.hpp"
#include "Polygone.hpp"
#include "RecuperationDonneesAncres.hpp"
#include "ScreenCommunicationManager.hpp"

RecuperationDonneesAncres recupDonnees;

void ecouterReseauFilaire() {
  if (MODE_ACTUEL == MODE_WIRED) {
    // MAXIME
  }
}

void OnWifidataReceived(int tagId, float d0, float d1, float d2, float d3) {
  if (MODE_ACTUEL == MODE_WIFI) {
    recupDonnees.injecterDonnee(tagId, d0, d1, d2, d3);
  }
}


void setup() {
  // Initialisation du port série à la vitesse configurée dans platformio.ini
  Serial.begin(115200);
  delay(2000);

  // On vérifie que la PSRAM est bien active
  initHub();
 
  setupScreenCommunication();

  // On crée les variables qui vont stocker toutes les informations concernant les ancres, les tags et la zone de sécurité
  // On commence par les ancres
  UWBModuleList vAnchors = initAnchors("Anchors");
  Serial.println(("Contenu de la liste des ancres :\n" + vAnchors.toString()).c_str());

  // Puis on crée la liste des tags (vide pour le moment)
  UWBModuleList vTags;

  // On déclare la variable stockant la zone de sécurité
  Polygone vSafeZone = Polygone(0, initSafeZone("SafeZone"));
  Serial.println(("Contenu de la zone de sécurité :\n" + vSafeZone.toString()).c_str());
 
  // PARTIE DE HUGUES en dessous de cette ligne de code
  Serial.println("Adresse MAC :");
  Serial.println(WiFi.macAddress());
  initWifi();
  //\Hugues

  // AUTO-START : Si une configuration valide existe au démarrage sur la mémoire flash du Hub,
  // on peut forcer l'état à HUB_STATE_RUNNING directement ici.
  if (vSafeZone.getPoints().size() > 0) {
      etatActuelHub = HUB_STATE_RUNNING;
      Serial.println("[Boot] Configuration existante trouvée. Passage automatique en RUNNING.");
  } else {
      etatActuelHub = HUB_STATE_IDLE;
  }
}

// Machine Etat Normal :
void executer_HUB_STATE_RUNNING() {

  ecouterReseauFilaire(); // Ne fait rien si le mode wifi est choisi

  std::vector<DistanceMoyennes> tagsPretsPourMaths;

  if (recupDonnees.getDonneesLissees(tagsPretsPourMaths)) {
    for (const DistanceMoyennes& tag : tagsPretsPourMaths) {
      Serial.printf("[30Hz] Tag %d |D0:%.2f | D1:%.2f | D2:%.2f | D3:%.2f\n", tag.tag_id, tag.distances[0], tag.distances[1], tag.distances[2], tag.distances[3]);
    }
  }

  // CALCULS TRILATERATION
  // PEUT RECEVOIR A TOUT MOMENT UNE NOUVELLE CONFIGURATION (POSITIONS ANCRES + ZONE EXCLUSION)

  //SIMULATION POUR L'INSTANT (BOUCLE PRINCIPALE ROUTINE HAUTE FRÉQUENCE 33ms)
  static unsigned long chronoRuntime = 0;
  if (millis() - chronoRuntime >= 33) { // Fréquence stricte 30Hz / 33ms
      
      // Ici, les fonctions réelles écriront dans ces variables après calculs :
      int ids[3] = {101, 12, 110};
      float xs[3] = {3.5, -1.5, 2.0};
      float ys[3] = {1.2, 2.4, -4.0};
      float dists[3] = {3.7, 2.8, 4.5};
      bool alarmes[3] = {false, true, false}; // Exemple d'alerte (il est entré dans le polygone vSafeZone)

      // Envoi direct de la ligne de données calculée (ou simulée) au manager UART
      envoyerMiseAJourTagsRuntime(ids, xs, ys, dists, alarmes, 3);
      
      chronoRuntime = millis();
  }
}

void  executer_HUB_STATE_DETECTING_TAGS_FOR_INIT () {
  //CALCULS POUR RECUPERATION DE TOUTES LES DISTANCES

  //SIMULATION POUR L'INSTANT
  Serial.println("[Machine Etats] Étape : Scan initial des tags demandé...");
  delay(1000); // On simule 1 seconde de calcul matériel d'acquisition

  // Ce que tes fonctions de scan retourneraient :
  int listeIds[3] = {101, 105, 110};
  float listeDistances[3] = {4.2, 2.1, 7.8};
  int nombreDeTagsTrouves = 3;

  // On transmet le résultat brut au manager d'écran
  envoyerListeTagsDecouverts(listeIds, listeDistances, nombreDeTagsTrouves);

  // Le travail est fait, on repasse en attente de la sélection utilisateur
  etatActuelHub = HUB_STATE_IDLE;
  Serial.println("[Machine Etats] Scan terminé. Liste envoyée. Retour en IDLE.");
}

void executer_HUB_STATE_GENERATING_GEOMETRY() {
  //CALCULS POUR CONNAITRE LE PLAN / LES POSITIONS DES ANCRES / DEFINITION DE LA ZONE D'EXCLUSION

  //SIMULATION POUR L'INSTANT
  Serial.printf("[Machine Etats] Étape : Génération géométrie pour le Tag cible #%d...\n", idTagSelectionne);
  delay(1500); // On simule 1.5 seconde de calculs matriciels complexes d'auto-positionnement

  // Ce que tes fonctions mathématiques sortiraient (Positions relatives en mètres) :
  float ancresCalculees[4][2] = {
      {-1.0, 2.0},  // Ancre 0 (X, Y)
      {1.0, 2.0},   // Ancre 1 (X, Y)
      {-1.0, -2.0}, // Ancre 2 (X, Y)
      {1.0, -2.0}   // Ancre 3 (X, Y)
  };

  float zone64PointsCalculee[64][2];
  for (int i = 0; i < 64; i++) {
      float angle = (i * 2.0 * PI) / 64.0;
      zone64PointsCalculee[i][0] = cos(angle) * 7.0; // Cercle de sécurité de 7 mètres par défaut
      zone64PointsCalculee[i][1] = sin(angle) * 7.0;
  }

  // Envoi de la géométrie calculée au manager d'écran
  envoyerGeometrieCalibration(ancresCalculees, zone64PointsCalculee);

  // Calculs finis, on se remet en attente que l'utilisateur ajuste sur le tel et valide
  etatActuelHub = HUB_STATE_IDLE;
  Serial.println("[Machine Etats] Géométrie initiale envoyée. Retour en IDLE.");
}

void executer_HUB_STATE_IDLE() {
  // ATTENDS DE RECEVOIR DE L'ECRAN LA ZONE D'EXCLUSION ET LA POSITION DES ANCRES = POUR L'INSTANT AUCUNE CONFIGURATION
  
  // Cette fonction reste vide ou fait clignoter une LED d'état. 
  // C'est loopScreenCommunication() qui va la faire sortir de cet état d'attente
  // lorsqu'un message arrivera sur l'UART.
}

void loop() {
  // L'écouteur UART tourne en tâche de fond permanente pour intercepter les changements d'état
  loopScreenCommunication();

  // Aiguillage dynamique de la loop selon l'état de l'automate principal
  switch (etatActuelHub) {
      case HUB_STATE_IDLE:
          executer_HUB_STATE_IDLE();
          break;
      case HUB_STATE_DETECTING_TAGS_FOR_INIT:
          executer_HUB_STATE_DETECTING_TAGS_FOR_INIT();
          break;
      case HUB_STATE_GENERATING_GEOMETRY:
          executer_HUB_STATE_GENERATING_GEOMETRY();
          break;
      case HUB_STATE_RUNNING:
          executer_HUB_STATE_RUNNING();
          break;
  }
}