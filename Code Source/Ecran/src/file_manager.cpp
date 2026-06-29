#include "file_manager.h"
#include "config.h"
#include <FS.h>
#include <SD_MMC.h>
#include <ArduinoJson.h>
#include <sys/time.h>
#include <time.h>

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
        if(pts > 1) {
            liste_vehicules[total_vehicules].zone_pixels[pts-1].x = liste_vehicules[total_vehicules].zone_pixels[0].x;
            liste_vehicules[total_vehicules].zone_pixels[pts-1].y = liste_vehicules[total_vehicules].zone_pixels[0].y;
        }
        liste_vehicules[total_vehicules].nb_points = pts;

        JsonArray sensors = v["sensors"].as<JsonArray>();
        int s_idx = 0;
        for (JsonVariant s : sensors) {
            if (s_idx >= MAX_CAPTEURS) break;
            float sx = s["x"].as<float>();
            float sy = s["y"].as<float>();
            liste_vehicules[total_vehicules].capteurs_metres[s_idx][0] = sx;
            liste_vehicules[total_vehicules].capteurs_metres[s_idx][1] = sy;
            liste_vehicules[total_vehicules].capteurs_pixels[s_idx].x = CENTRE_X + (int)(sx * PIXELS_PER_METER);
            liste_vehicules[total_vehicules].capteurs_pixels[s_idx].y = CENTRE_Y - (int)(sy * PIXELS_PER_METER);
            s_idx++;
        }
        liste_vehicules[total_vehicules].nb_capteurs = s_idx;
        total_vehicules++;
    }
}

// === FILE SYSTEM LVGL ===
static void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode) {
    const char * flags = (mode == LV_FS_MODE_WR) ? "w" : ((mode == LV_FS_MODE_RD) ? "r" : "r+");
    File * f = new File(SD_MMC.open(String("/") + path, flags));
    if(!*f) { delete f; return NULL; }
    return (void *)f;
}
static lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p) {
    File * f = (File *)file_p; f->close(); delete f; return LV_FS_RES_OK;
}
static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br) {
    File * f = (File *)file_p; *br = f->read((uint8_t *)buf, btr); return LV_FS_RES_OK;
}
static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence) {
    File * f = (File *)file_p;
    SeekMode m = (whence == LV_FS_SEEK_CUR) ? SeekCur : ((whence == LV_FS_SEEK_END) ? SeekEnd : SeekSet);
    f->seek(pos, m); return LV_FS_RES_OK;
}
static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p) {
    File * f = (File *)file_p; *pos_p = f->position(); return LV_FS_RES_OK;
}

void lv_port_fs_init(void) {
    static lv_fs_drv_t fs_drv; lv_fs_drv_init(&fs_drv);
    fs_drv.letter = 'A'; fs_drv.open_cb = fs_open; fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read; fs_drv.seek_cb = fs_seek; fs_drv.tell_cb = fs_tell;
    lv_fs_drv_register(&fs_drv);
}

