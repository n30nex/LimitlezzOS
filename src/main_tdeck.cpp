/**
 * LimitlezzOS — LilyGO T-Deck hardware target (ESP32-S3).
 *
 * Bring-up (spec phase 1.0/1.4 — UI portion):
 *   - board power rail (GPIO 10 must go HIGH before peripherals respond)
 *   - ST7789 320x240 over SPI via TFT_eSPI (pins in platformio.ini)
 *   - I2C keyboard (ESP32-C3 slave @ 0x55) — returns one ASCII char per poll
 *   - trackball: 4 direction pins pulse on roll; click on GPIO 0
 *
 * Radio stacks, Lua VM, and the rest of the backend land behind the
 * mock data layer in src/ui/data.c (spec phases 1.3+).
 */
#ifdef LZ_TARGET_TDECK

#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include "lvgl.h"
#include "ui/ui.h"

#define BOARD_POWERON      10
#define BOARD_I2C_SDA      18
#define BOARD_I2C_SCL      8
#define KEYBOARD_I2C_ADDR  0x55
#define TRACKBALL_UP       3
#define TRACKBALL_DOWN     15
#define TRACKBALL_LEFT     1
#define TRACKBALL_RIGHT    2
#define TRACKBALL_CLICK    0

static TFT_eSPI tft;
static lv_color_t draw_buf_mem[LZ_W * 40];

extern "C" uint32_t lz_tick_ms(void) { return millis(); }

static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *px)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)px, w * h, true);
    tft.endWrite();
    lv_disp_flush_ready(drv);
}

/* trackball edges counted in ISRs, consumed in loop() */
static volatile int tb_up, tb_down, tb_left, tb_right;
static void IRAM_ATTR isr_up(void)    { tb_up++; }
static void IRAM_ATTR isr_down(void)  { tb_down++; }
static void IRAM_ATTR isr_left(void)  { tb_left++; }
static void IRAM_ATTR isr_right(void) { tb_right++; }

void setup()
{
    pinMode(BOARD_POWERON, OUTPUT);
    digitalWrite(BOARD_POWERON, HIGH);   /* power the peripherals first */
    delay(100);

    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);

    pinMode(TRACKBALL_UP, INPUT_PULLUP);
    pinMode(TRACKBALL_DOWN, INPUT_PULLUP);
    pinMode(TRACKBALL_LEFT, INPUT_PULLUP);
    pinMode(TRACKBALL_RIGHT, INPUT_PULLUP);
    pinMode(TRACKBALL_CLICK, INPUT_PULLUP);
    attachInterrupt(TRACKBALL_UP, isr_up, FALLING);
    attachInterrupt(TRACKBALL_DOWN, isr_down, FALLING);
    attachInterrupt(TRACKBALL_LEFT, isr_left, FALLING);
    attachInterrupt(TRACKBALL_RIGHT, isr_right, FALLING);

    tft.begin();
    tft.setRotation(1);                  /* landscape 320x240 */
    tft.setSwapBytes(true);
    tft.fillScreen(TFT_BLACK);

    lv_init();
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, draw_buf_mem, NULL, LZ_W * 40);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LZ_W;
    disp_drv.ver_res = LZ_H;
    disp_drv.flush_cb = flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    lz_ui_init(lv_scr_act());
}

static char read_kb(void)
{
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1);
    if(Wire.available()) {
        char c = Wire.read();
        if(c != 0) return c;
    }
    return 0;
}

void loop()
{
    /* trackball roll: 2 pulses per focus step debounces jitter */
    static const int STEP = 2;
    if(tb_up >= STEP)    { tb_up = 0;    lz_ui_key(LZ_K_UP, 0); }
    if(tb_down >= STEP)  { tb_down = 0;  lz_ui_key(LZ_K_DOWN, 0); }
    if(tb_left >= STEP)  { tb_left = 0;  lz_ui_key(LZ_K_LEFT, 0); }
    if(tb_right >= STEP) { tb_right = 0; lz_ui_key(LZ_K_RIGHT, 0); }

    static bool click_was = false;
    bool click = digitalRead(TRACKBALL_CLICK) == LOW;
    if(click && !click_was) lz_ui_key(LZ_K_ENTER, 0);
    click_was = click;

    static uint32_t last_kb = 0;
    if(millis() - last_kb > 40) {
        last_kb = millis();
        char c = read_kb();
        if(c == '\r' || c == '\n') lz_ui_key(LZ_K_ENTER, 0);
        else if(c == 8 || c == 127) lz_ui_key(LZ_K_BACK, 0);
        else if(c) lz_ui_key(LZ_K_CHAR, c);
    }

    lv_timer_handler();
    delay(5);
}

#endif /* LZ_TARGET_TDECK */
