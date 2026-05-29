#include <Arduino.h>
#include <lvgl.h>
#include <ArduinoJson.h> 
#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>
#include <Wire.h>
#include <FS.h>
#include <SD_MMC.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <sys/time.h>
#include <time.h>

// =======================================================
// CONFIGURATION ÉCRAN & MÉTRIQUE
// =======================================================
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480
#define CENTRE_X (SCREEN_WIDTH / 2)
#define CENTRE_Y (SCREEN_HEIGHT / 2)
#define PIXELS_PER_METER 19.2f

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18
#define TOUCH_INT -1
#define TOUCH_RST 38
#define TFT_BL 10
#define PIN_SD_CMD 11
#define PIN_SD_CLK 12
#define PIN_SD_D0 13

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED, GFX_NOT_DEFINED, GFX_NOT_DEFINED,
    40, 41, 39, 42, 45, 48, 47, 21, 14,
    5, 6, 7, 15, 16, 4, 8, 3, 46, 9, 1
);

Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(
    bus, SCREEN_WIDTH, 1, 40, 48, 128, SCREEN_HEIGHT, 1, 13, 3, 45
);

TAMC_GT911 ts = TAMC_GT911(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_INT, TOUCH_RST, 1024, 600);

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10)
lv_color_t *draw_buf;

AsyncWebServer server(80);

// =======================================================
// STRUCTURES DE DONNÉES
// =======================================================
struct TagGraphique {
  lv_obj_t * point;
  lv_obj_t * label_id;
  bool en_alarme;
};

struct VehiculeConfig {
  String nom;
  String operateur;
  String date_creation;
  String fichier_image;
  float zone_metres[64][2]; 
  lv_point_t zone_pixels[64];
  float capteurs_metres[4][2];
  lv_point_t capteurs_pixels[4];
  int nb_points;
};

struct LogEvent {
  unsigned long timestamp;
  String type;
  String message;
};

TagGraphique tag1;
VehiculeConfig liste_vehicules[20]; 
int total_vehicules = 0;
int id_vehicule_actif = -1;

lv_obj_t * scr_radar;
lv_obj_t * scr_vehicules;
lv_obj_t * polygone_exclusion;
lv_obj_t * mosaique; 
lv_obj_t * pelleteuse;
lv_obj_t * visuel_capteurs[4];
lv_obj_t * cadre_alerte_global;
lv_obj_t * label_vide; 
bool alarme_danger = false; 
bool flag_recharger_ui = false; 

// Simulation Hub
unsigned long dernier_temps = 0;
int etape_simulation = 0;
int calib_state = 0; 
unsigned long calib_timer = 0;
float sim_calib_points[64][2];
float sim_calib_capteurs[4][2] = {
  {-1.0, 2.0},
  {1.0, 2.0},
  {-1.0, -2.0},
  {1.0, -2.0}
};

// =======================================================
// HORODATAGE & LOGS SD
// =======================================================
String obtenirHeure() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  if (tv.tv_sec < 1000000000) return "Heure_Inconnue"; 
  time_t now = tv.tv_sec;
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

void logWarningSD(int tag_id, float x, float y) {
  File f = SD_MMC.open("/warnings.csv", FILE_APPEND);
  if (f) {
    if (f.size() == 0) f.println("Date,TagID,X,Y"); 
    f.printf("%s,%d,%.1f,%.1f\n", obtenirHeure().c_str(), tag_id, x, y);
    f.close();
  }
}

// =======================================================
// GESTION FICHIERS JSON
// =======================================================
void charger_vehicules_sd() {
  total_vehicules = 0;
  id_vehicule_actif = -1;
  File file = SD_MMC.open("/calibrations.json", "r");
  if (!file) return;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) return;

  JsonArray array = doc.as<JsonArray>();
  for (JsonVariant v : array) {
    if (total_vehicules >= 20) break;
    liste_vehicules[total_vehicules].nom = v["nom"].as<String>();
    liste_vehicules[total_vehicules].operateur = v["operateur"].as<String>();
    liste_vehicules[total_vehicules].date_creation = v["date"].as<String>();
    liste_vehicules[total_vehicules].fichier_image = v["image"].as<String>();
    
    JsonArray zone = v["zone"].as<JsonArray>();
    int pts = 0;
    for (JsonVariant p : zone) {
      if (pts >= 64) break;
      float mx = p["x"].as<float>();
      float my = p["y"].as<float>();
      liste_vehicules[total_vehicules].zone_metres[pts][0] = mx;
      liste_vehicules[total_vehicules].zone_metres[pts][1] = my;
      liste_vehicules[total_vehicules].zone_pixels[pts].x = CENTRE_X + (int)(mx * PIXELS_PER_METER);
      liste_vehicules[total_vehicules].zone_pixels[pts].y = CENTRE_Y - (int)(my * PIXELS_PER_METER);
      pts++;
    }

    // ----------------------------------------------------------------
    // LECTURE DES CAPTEURS DEPUIS LE FICHIER JSON
    // ----------------------------------------------------------------
    // 1. On cherche la "boîte" nommée "sensors" dans notre objet JSON
    JsonArray sensors = v["sensors"].as<JsonArray>();
    
    int s_idx = 0; // Un compteur pour s'assurer qu'on ne dépasse pas 4 capteurs
    
    for (JsonVariant s : sensors) {
      if (s_idx >= 4) break; // Sécurité : on s'arrête si le JSON a trop de capteurs
      
      // 2. On extrait les coordonnées X et Y en mètres (nombres à virgule : float)
      float sx = s["x"].as<float>();
      float sy = s["y"].as<float>();
      
      // 3. On sauvegarde la valeur en mètres dans notre structure pour la garder en mémoire
      liste_vehicules[total_vehicules].capteurs_metres[s_idx][0] = sx;
      liste_vehicules[total_vehicules].capteurs_metres[s_idx][1] = sy;
      
      // 4. CONVERSION MATHÉMATIQUE (Mètres vers Pixels)
      // L'écran fait 800 pixels de large. Notre centre est à 400 (CENTRE_X).
      // On multiplie nos mètres par PIXELS_PER_METER (19.2) pour savoir combien de pixels ça représente.
      liste_vehicules[total_vehicules].capteurs_pixels[s_idx].x = CENTRE_X + (int)(sx * PIXELS_PER_METER);
      
      // Sur un écran physique, le point (0,0) est en HAUT à GAUCHE. Donc plus Y augmente, plus on DESCEND.
      // C'est pour cela qu'on met un signe "-" (CENTRE_Y - ...) pour inverser le sens mathématique vers le sens de l'écran.
      liste_vehicules[total_vehicules].capteurs_pixels[s_idx].y = CENTRE_Y - (int)(sy * PIXELS_PER_METER);
      
      s_idx++;
    }


    // FERMETURE PARFAITE DE LA BOUCLE (Le dernier point rejoint le premier)
    if(pts > 1) {
        liste_vehicules[total_vehicules].zone_pixels[pts-1].x = liste_vehicules[total_vehicules].zone_pixels[0].x;
        liste_vehicules[total_vehicules].zone_pixels[pts-1].y = liste_vehicules[total_vehicules].zone_pixels[0].y;
    }
    
    liste_vehicules[total_vehicules].nb_points = pts;
    total_vehicules++;
  }
}

static void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode) {
  LV_UNUSED(drv);
  const char * flags = (mode == LV_FS_MODE_WR) ? "w" : ((mode == LV_FS_MODE_RD) ? "r" : "r+");
  File * f = new File(SD_MMC.open(String("/") + path, flags));
  if(!*f) { delete f; return NULL; }
  return (void *)f;
}
static lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p) {
  LV_UNUSED(drv); File * f = (File *)file_p; f->close(); delete f; return LV_FS_RES_OK;
}
static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br) {
  LV_UNUSED(drv); File * f = (File *)file_p; *br = f->read((uint8_t *)buf, btr); return LV_FS_RES_OK;
}
static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence) {
  LV_UNUSED(drv); File * f = (File *)file_p; 
  SeekMode m = (whence == LV_FS_SEEK_CUR) ? SeekCur : ((whence == LV_FS_SEEK_END) ? SeekEnd : SeekSet);
  f->seek(pos, m); return LV_FS_RES_OK;
}
static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p) {
  LV_UNUSED(drv); File * f = (File *)file_p; *pos_p = f->position(); return LV_FS_RES_OK;
}
void lv_port_fs_init(void) {
  static lv_fs_drv_t fs_drv; lv_fs_drv_init(&fs_drv);
  fs_drv.letter = 'A'; fs_drv.open_cb = fs_open; fs_drv.close_cb = fs_close;
  fs_drv.read_cb = fs_read; fs_drv.seek_cb = fs_seek; fs_drv.tell_cb = fs_tell;
  lv_fs_drv_register(&fs_drv);
}

// =======================================================
// PONT ÉCRAN & TACTILE
// =======================================================
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)color_p, w, h);
  lv_disp_flush_ready(disp);
}

void touchscreen_read(lv_indev_drv_t * indev, lv_indev_data_t * data) {
  ts.read();
  if (ts.isTouched) {
    data->state = LV_INDEV_STATE_PR;
    int mapped_x = map(ts.points[0].x, 1024, 200, 0, SCREEN_WIDTH);
    int mapped_y = map(ts.points[0].y, 600, 120, 0, SCREEN_HEIGHT);
    if(mapped_x < 0) mapped_x = 0; if(mapped_x > SCREEN_WIDTH - 1) mapped_x = SCREEN_WIDTH - 1;
    if(mapped_y < 0) mapped_y = 0; if(mapped_y > SCREEN_HEIGHT - 1) mapped_y = SCREEN_HEIGHT - 1;
    data->point.x = mapped_x; data->point.y = mapped_y;
    ts.isTouched = false; 
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// =======================================================
// INTERFACE GRAPHIQUE LVGL
// =======================================================
static void btn_go_vehicules_cb(lv_event_t * e) { lv_scr_load(scr_vehicules); }
static void btn_go_radar_cb(lv_event_t * e) { lv_scr_load(scr_radar); }

static void btn_select_vehicule_cb(lv_event_t * e) {
  lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
  int idx = (int)(uintptr_t)lv_obj_get_user_data(btn);
  id_vehicule_actif = idx;

  // Affiche et place les capteurs
  for(int i=0; i<4; i++) {
    lv_obj_clear_flag(visuel_capteurs[i], LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(visuel_capteurs[i], LV_ALIGN_CENTER, 
                 liste_vehicules[idx].capteurs_pixels[i].x - CENTRE_X, 
                 liste_vehicules[idx].capteurs_pixels[i].y - CENTRE_Y);
  }

  lv_line_set_points(polygone_exclusion, liste_vehicules[idx].zone_pixels, liste_vehicules[idx].nb_points);
  String chemin = "A:/" + liste_vehicules[idx].fichier_image;
  lv_img_set_src(pelleteuse, chemin.c_str());
  lv_scr_load(scr_radar);
}

void initialiser_composant_tag(TagGraphique &t, lv_obj_t * parent) {
  t.point = lv_obj_create(parent);
  lv_obj_set_size(t.point, 40, 40);
  lv_obj_set_style_radius(t.point, LV_RADIUS_CIRCLE, 0);
  lv_obj_align(t.point, LV_ALIGN_CENTER, 0, -500); 
  lv_obj_clear_flag(t.point, LV_OBJ_FLAG_SCROLLABLE);
  t.label_id = lv_label_create(t.point);
  lv_label_set_text(t.label_id, "?");
  lv_obj_set_style_text_color(t.label_id, lv_color_hex(0xFFFFFF), 0);
  lv_obj_center(t.label_id);
  t.en_alarme = false;
}

void mettre_a_jour_visuel_tag(TagGraphique &t, float x_m, float y_m, bool alarme, int id) {
  t.en_alarme = alarme;
  lv_label_set_text_fmt(t.label_id, "%d", id);
  int px_x = (int)(x_m * PIXELS_PER_METER);
  int px_y = (int)(-y_m * PIXELS_PER_METER); 
  lv_obj_align(t.point, LV_ALIGN_CENTER, px_x, px_y);
}

void lv_create_main_gui(void) {
  scr_radar = lv_obj_create(NULL);
  lv_obj_clear_flag(scr_radar, LV_OBJ_FLAG_SCROLLABLE); 
  lv_obj_set_style_bg_color(scr_radar, lv_color_hex(0x000000), 0);

  scr_vehicules = lv_obj_create(NULL);
  lv_obj_clear_flag(scr_vehicules, LV_OBJ_FLAG_SCROLLABLE); 
  lv_obj_set_style_bg_color(scr_vehicules, lv_color_hex(0x111111), 0);

  cadre_alerte_global = lv_obj_create(lv_layer_top()); 
  lv_obj_set_size(cadre_alerte_global, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_opa(cadre_alerte_global, LV_OPA_TRANSP, 0); 
  lv_obj_set_style_border_color(cadre_alerte_global, lv_color_hex(0xFF0000), 0); 
  lv_obj_set_style_border_width(cadre_alerte_global, 10, 0); 
  lv_obj_add_flag(cadre_alerte_global, LV_OBJ_FLAG_HIDDEN); 
  
  lv_obj_t * btn_veh = lv_btn_create(scr_radar);
  lv_obj_set_size(btn_veh, 100, 60);
  lv_obj_align(btn_veh, LV_ALIGN_TOP_RIGHT, -10, 10);
  lv_obj_add_event_cb(btn_veh, btn_go_vehicules_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_t * lbl_btn = lv_label_create(btn_veh);
  lv_label_set_text(lbl_btn, "MENU");
  lv_obj_center(lbl_btn);

  polygone_exclusion = lv_line_create(scr_radar);
  lv_obj_set_style_line_width(polygone_exclusion, 5, 0);
  lv_obj_set_style_line_color(polygone_exclusion, lv_color_hex(0xFF3333), 0);

  // Création des 4 capteurs
  for(int i=0; i<4; i++) {
    visuel_capteurs[i] = lv_obj_create(scr_radar);
    lv_obj_set_size(visuel_capteurs[i], 12, 12);
    lv_obj_set_style_bg_color(visuel_capteurs[i], lv_color_hex(0xFBC02D), 0); // Jaune
    lv_obj_set_style_radius(visuel_capteurs[i], 2, 0);
    lv_obj_add_flag(visuel_capteurs[i], LV_OBJ_FLAG_HIDDEN); // Cachés par défaut
  }



  pelleteuse = lv_img_create(scr_radar);
  lv_obj_align(pelleteuse, LV_ALIGN_CENTER, 0, 0);

  label_vide = lv_label_create(scr_radar);
  lv_label_set_text(label_vide, "En attente de configuration.\nConnectez-vous au Wi-Fi.");
  lv_obj_set_style_text_color(label_vide, lv_color_hex(0x888888), 0);
  lv_obj_align(label_vide, LV_ALIGN_CENTER, 0, 0);

  initialiser_composant_tag(tag1, scr_radar);

  lv_obj_t * btn_rad = lv_btn_create(scr_vehicules);
  lv_obj_set_size(btn_rad, 100, 60);
  lv_obj_align(btn_rad, LV_ALIGN_TOP_RIGHT, -10, 10);
  lv_obj_add_event_cb(btn_rad, btn_go_radar_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_t * lbl_rad = lv_label_create(btn_rad);
  lv_label_set_text(lbl_rad, "RETOUR");
  lv_obj_center(lbl_rad);

  mosaique = lv_obj_create(scr_vehicules);
  lv_obj_set_size(mosaique, 700, 350);
  lv_obj_align(mosaique, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_flex_flow(mosaique, LV_FLEX_FLOW_ROW_WRAP); 

  lv_scr_load(scr_radar);
}

void construire_menu_vehicules() {
  lv_obj_clean(mosaique); 
  
  // SI TOUT EST SUPPRIMÉ : NETTOYAGE TOTAL DE L'ÉCRAN
  if(total_vehicules == 0) {
    lv_obj_clear_flag(label_vide, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(polygone_exclusion, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(pelleteuse, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(tag1.point, LV_OBJ_FLAG_HIDDEN); // On cache le radar
    return;
  }
  
  // SINON ON RÉAFFICHE TOUT
  lv_obj_add_flag(label_vide, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(polygone_exclusion, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(pelleteuse, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(tag1.point, LV_OBJ_FLAG_HIDDEN);

  for(int i = 0; i < total_vehicules; i++) {
    lv_obj_t * btn = lv_btn_create(mosaique);
    lv_obj_set_size(btn, 180, 150); 
    lv_obj_set_user_data(btn, (void *)(uintptr_t)i);
    lv_obj_add_event_cb(btn, btn_select_vehicule_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl = lv_label_create(btn);
    lv_label_set_text(lbl, liste_vehicules[i].nom.c_str());
    lv_obj_center(lbl);
  }

  // Affiche et place les capteurs
  for(int i=0; i<4; i++) {
    lv_obj_clear_flag(visuel_capteurs[i], LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(visuel_capteurs[i], LV_ALIGN_CENTER, 
                 liste_vehicules[id_vehicule_actif].capteurs_pixels[i].x - CENTRE_X, 
                 liste_vehicules[id_vehicule_actif].capteurs_pixels[i].y - CENTRE_Y);
  }


  if (id_vehicule_actif < 0 || id_vehicule_actif >= total_vehicules) id_vehicule_actif = 0;
  lv_line_set_points(polygone_exclusion, liste_vehicules[id_vehicule_actif].zone_pixels, liste_vehicules[id_vehicule_actif].nb_points);
  String chemin = "A:/" + liste_vehicules[id_vehicule_actif].fichier_image;
  lv_img_set_src(pelleteuse, chemin.c_str());
}

// =======================================================
// INITIALISATION
// =======================================================
void setup() {
  Serial.begin(115200);

  gfx->begin();
  pinMode(TFT_BL, OUTPUT); pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TFT_BL, LOW); delay(100);
  digitalWrite(TOUCH_RST, LOW); delay(1000); digitalWrite(TOUCH_RST, HIGH); delay(1000);
  digitalWrite(TOUCH_RST, LOW); delay(1000); digitalWrite(TOUCH_RST, HIGH); delay(1000); 

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  ts.begin(); ts.setRotation(ROTATION_NORMAL);

  SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
  if (!SD_MMC.begin("/sdcard", true)) Serial.println("Erreur SD");
  
  charger_vehicules_sd();
  WiFi.softAP("MaTouch_Radar", "12345678"); 

  // --- API SERVEUR WEB ---
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (SD_MMC.exists("/index.html")) request->send(SD_MMC, "/index.html", "text/html");
    else request->send(200, "text/plain", "Fichier index.html introuvable !");
  });

  server.on("/api/time", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(request->hasParam("ts", true)){
      long ts = request->getParam("ts", true)->value().toInt();
      struct timeval tv; tv.tv_sec = ts; tv.tv_usec = 0;
      settimeofday(&tv, NULL);
      setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); // Fuseau Français
      tzset();
    }
    request->send(200, "text/plain", "Heure synchronisée");
  });

  server.on("/api/warnings", HTTP_GET, [](AsyncWebServerRequest *request){
    if (SD_MMC.exists("/warnings.csv")) request->send(SD_MMC, "/warnings.csv", "text/csv");
    else request->send(200, "text/csv", "Date,TagID,X,Y\n"); 
  });
  server.on("/api/calibrations", HTTP_GET, [](AsyncWebServerRequest *request){
    if (SD_MMC.exists("/calibrations.json")) request->send(SD_MMC, "/calibrations.json", "application/json");
    else request->send(200, "application/json", "[]"); 
  });

  server.on("/api/calibrations", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    static File file;
    if (!index) file = SD_MMC.open("/calibrations.json", "w");
    if (file) file.write(data, len);
    if (index + len == total) {
      if (file) file.close();
      flag_recharger_ui = true; 
    }
  });

  // API Upload Image (Convertie)
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Upload OK");
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    static File imgFile;
    if (!index) imgFile = SD_MMC.open("/" + filename, "w");
    if (imgFile) imgFile.write(data, len);
    if (final && imgFile) imgFile.close();
  });

  server.on("/api/hub/tags", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", "[{\"id\":101, \"dist\":3.2}, {\"id\":105, \"dist\":7.8}]");
  });

  server.on("/api/hub/start", HTTP_POST, [](AsyncWebServerRequest *request){
    calib_state = 1; calib_timer = millis();
    request->send(200, "text/plain", "OK");
  });

  server.on("/api/hub/cancel", HTTP_POST, [](AsyncWebServerRequest *request){
    calib_state = 0; request->send(200, "text/plain", "Annulé");
  });

  server.on("/api/hub/status", HTTP_GET, [](AsyncWebServerRequest *request){
    if (calib_state == 0) { request->send(200, "application/json", "{\"status\":\"idle\"}"); }
    else if (calib_state == 1) { request->send(200, "application/json", "{\"status\":\"wait\"}"); }
    else if (calib_state == 2) {
      // 1. On crée le document JSON (Syntaxe V7)
      JsonDocument doc; 
      doc["status"] = "done";
      
      // 2. On crée un tableau "points" et on le remplit
      JsonArray pts = doc["points"].to<JsonArray>();
      for(int i=0; i<64; i++){
        // "add<JsonObject>()" remplace "createNestedObject()" en V7
        JsonObject p = pts.add<JsonObject>(); 
        p["x"] = sim_calib_points[i][0];
        p["y"] = sim_calib_points[i][1];
      }

      // 3. On crée un tableau "sensors" (les capteurs) et on le remplit
      JsonArray caps = doc["sensors"].to<JsonArray>();
      for(int i=0; i<4; i++){
        JsonObject c = caps.add<JsonObject>();
        // Ici, on utilise bien notre tableau "float" et pas l'objet graphique !
        c["x"] = sim_calib_capteurs[i][0]; 
        c["y"] = sim_calib_capteurs[i][1];
      }

      // 4. On transforme tout ça en texte et on l'envoie
      String res; 
      serializeJson(doc, res); 
      request->send(200, "application/json", res);
      
      calib_state = 0; 
    }
  });


  server.begin(); 

  lv_init(); lv_port_fs_init(); 
  draw_buf = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!draw_buf) draw_buf = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  static lv_disp_draw_buf_t disp_buf; lv_disp_draw_buf_init(&disp_buf, draw_buf, NULL, DRAW_BUF_SIZE);
  static lv_disp_drv_t disp_drv; lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = SCREEN_WIDTH; disp_drv.ver_res = SCREEN_HEIGHT; disp_drv.flush_cb = my_disp_flush; disp_drv.draw_buf = &disp_buf;
  lv_disp_drv_register(&disp_drv);
  static lv_indev_drv_t indev_drv; lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER; indev_drv.read_cb = touchscreen_read; lv_indev_drv_register(&indev_drv);

  lv_create_main_gui();
  construire_menu_vehicules();
}

void loop() {
  if (flag_recharger_ui) {
    charger_vehicules_sd();
    construire_menu_vehicules();
    flag_recharger_ui = false;
  }

  if (tag1.en_alarme) {
    if ((millis() / 200) % 2 == 0) lv_obj_set_style_bg_color(tag1.point, lv_color_hex(0xFF0000), 0);
    else lv_obj_set_style_bg_color(tag1.point, lv_color_hex(0x000000), 0);
  } else {
    lv_obj_set_style_bg_color(tag1.point, lv_color_hex(0x00FF00), 0);
  }

  if (alarme_danger) {
    if ((millis() / 250) % 2 == 0) lv_obj_clear_flag(cadre_alerte_global, LV_OBJ_FLAG_HIDDEN); 
    else lv_obj_add_flag(cadre_alerte_global, LV_OBJ_FLAG_HIDDEN); 
  } else {
    lv_obj_add_flag(cadre_alerte_global, LV_OBJ_FLAG_HIDDEN); 
  }

  // Simulation Hub (Calibration)
  if (calib_state == 1 && (millis() - calib_timer > 3000)) {
    for(int i=0; i<64; i++){
      float angle = (i * 2 * PI) / 64.0;
      sim_calib_points[i][0] = cos(angle) * 8.0; 
      sim_calib_points[i][1] = sin(angle) * 8.0;
    }
    // Fermeture de la boucle
    sim_calib_points[63][0] = sim_calib_points[0][0];
    sim_calib_points[63][1] = sim_calib_points[0][1];
    calib_state = 2;
  }

  // Simulation Hub (Déplacement des tags)
  if (millis() - dernier_temps > 3000 && id_vehicule_actif >= 0 && calib_state == 0) {
    alarme_danger = false;
    bool was_in_alarm = tag1.en_alarme;
    float mx = 0, my = 0;

    if (etape_simulation == 0) { mx = 10.0; my = 5.0; tag1.en_alarme = false; } 
    else if (etape_simulation == 1) { 
      mx = 3.5; my = 2.0; tag1.en_alarme = true; alarme_danger = true; 
      if (!was_in_alarm) logWarningSD(101, mx, my); // Écriture log
    } 
    else if (etape_simulation == 2) { mx = -12.0; my = -8.0; tag1.en_alarme = false; }
    
    mettre_a_jour_visuel_tag(tag1, mx, my, tag1.en_alarme, 101); 
    
    etape_simulation++;
    if (etape_simulation > 2) etape_simulation = 0;
    dernier_temps = millis();
  }

  lv_tick_inc(5);
  lv_timer_handler();
  delay(5);
}