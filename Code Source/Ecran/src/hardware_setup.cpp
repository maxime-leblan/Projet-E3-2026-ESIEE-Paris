#include "hardware_setup.h"
#include "config.h"
#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>
#include <Wire.h>
#include <SD_MMC.h>

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED, GFX_NOT_DEFINED, GFX_NOT_DEFINED,
    40, 41, 39, 42, 45, 48, 47, 21, 14,
    5, 6, 7, 15, 16, 4, 8, 3, 46, 9, 1
);

Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(
    bus, SCREEN_WIDTH, 1, 40, 48, 128, SCREEN_HEIGHT, 1, 13, 3, 45
);

TAMC_GT911 ts = TAMC_GT911(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_INT, TOUCH_RST, 1024, 600);

lv_color_t *draw_buf;
lv_color_t *draw_buf2;

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

void setup_hardware_basic() {
    gfx->begin();
    pinMode(TFT_BL, OUTPUT); pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TFT_BL, LOW); delay(100);
    digitalWrite(TOUCH_RST, LOW); delay(1000); digitalWrite(TOUCH_RST, HIGH); delay(1000);
    digitalWrite(TOUCH_RST, LOW); delay(1000); digitalWrite(TOUCH_RST, HIGH); delay(1000);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    ts.begin(); ts.setRotation(ROTATION_NORMAL);

    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
    if (!SD_MMC.begin("/sdcard", true)) Serial.println("Erreur SD");
}

void setup_hardware_lvgl() {
    draw_buf = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    draw_buf2 = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!draw_buf) draw_buf = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!draw_buf2) draw_buf2 = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, draw_buf, draw_buf2, DRAW_BUF_SIZE);

    static lv_disp_drv_t disp_drv; lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH; disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush; disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv; lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER; indev_drv.read_cb = touchscreen_read; lv_indev_drv_register(&indev_drv);
}

