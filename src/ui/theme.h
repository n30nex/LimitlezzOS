/**
 * LimitlezzOS design tokens — extracted from the design handoff
 * (LimitlezzOS.dc.html / README). Flat solid fills only, 1px hairlines,
 * 2px near-white focus ring. No gradients, shadows, or alpha layering.
 */
#ifndef LZ_THEME_H
#define LZ_THEME_H

#include "lvgl.h"
#include "theme_colors.h"

/* ---- Surfaces (dark, cool neutral) ---- */
#define LZ_SCREEN_BG     lv_color_hex(0x0C0E12)
#define LZ_STATUSBAR_BG  lv_color_hex(0x161B22)
#define LZ_NAVBAR_BG     lv_color_hex(0x1B2230)
#define LZ_ROW_BG        lv_color_hex(0x13161C)
#define LZ_ROW_FOCUS_BG  lv_color_hex(0x1C222B)
#define LZ_ROW_BORDER    lv_color_hex(0x232A33)
#define LZ_HAIRLINE      lv_color_hex(0x11161D)
#define LZ_CARD_BG       lv_color_hex(0x191D24)   /* rgba(255,255,255,0.035) on base */
#define LZ_CARD_BORDER   lv_color_hex(0x262B33)   /* rgba(255,255,255,0.06) inset   */
#define LZ_INSET_BG      lv_color_hex(0x1A1F27)
#define LZ_CHIP_BG       lv_color_hex(0x171B21)   /* rgba(255,255,255,0.05) chips   */
#define LZ_BUBBLE_IN     lv_color_hex(0x1C212A)
#define LZ_TERM_BG       lv_color_hex(0x070809)
#define LZ_TERM_BAR      lv_color_hex(0x161B22)
#define LZ_GRAPHITE      lv_color_hex(0x444B56)   /* neutral app tiles */
#define LZ_TRACK_OFF     lv_color_hex(0x2A2F37)
#define LZ_KNOB          lv_color_hex(0xFFFFFF)

/* ---- Text ---- */
#define LZ_TEXT          lv_color_hex(0xEEF1F4)
#define LZ_TEXT_2        lv_color_hex(0x869099)
#define LZ_TEXT_3        lv_color_hex(0x6F7882)
#define LZ_TEXT_META     lv_color_hex(0x7C838C)
#define LZ_TEXT_BRIGHT   lv_color_hex(0xFFFFFF)
#define LZ_TEXT_NAV      lv_color_hex(0xE2E6EA)
#define LZ_TEXT_DIMLBL   lv_color_hex(0xAAB0B9)
#define LZ_TEXT_VALUE    lv_color_hex(0x9AA1AB)
#define LZ_TEXT_SETTING  lv_color_hex(0xE9EDF2)
#define LZ_TEXT_STRONG   lv_color_hex(0xDFE3E8)
#define LZ_TERM_GREEN    lv_color_hex(0x7FE0A8)
#define LZ_TERM_DIM      lv_color_hex(0x5F6A72)
#define LZ_TERM_CMD      lv_color_hex(0xCFD4DA)
#define LZ_ON_MINT       lv_color_hex(0x06140F)   /* dark text on mint  */
#define LZ_ON_CYAN       lv_color_hex(0x06181F)   /* dark text on cyan  */
#define LZ_ON_AMBER      lv_color_hex(0x1C1407)   /* dark text on amber */

/* ---- Metrics ---- */
#define LZ_W            320
#define LZ_H            240
#define LZ_STATUSBAR_H  22
#define LZ_NAVBAR_H     29
#define LZ_RADIUS_ROW   9
#define LZ_RADIUS_CARD  11
#define LZ_RADIUS_TILE  13
#define LZ_FOCUS_RING_W 2

/* ---- Fonts (baked tables; Montserrat stands in for Helvetica Neue) ---- */
#define LZ_F_CLOCK  (&lv_font_montserrat_48)
#define LZ_F_BIG    (&lv_font_montserrat_20)
#define LZ_F_TITLE  (&lv_font_montserrat_16)
#define LZ_F_HEAD   (&lv_font_montserrat_14)
#define LZ_F_BODY   (&lv_font_montserrat_12)
#define LZ_F_SMALL  (&lv_font_montserrat_10)
#define LZ_F_MONO   (&lv_font_unscii_8)

/* ---- Icon fonts (Material Symbols Rounded subsets) ---- */
LV_FONT_DECLARE(lz_icons_24)   /* FILL=1, launcher tiles + lock     */
LV_FONT_DECLARE(lz_icons_16f)  /* FILL=1, small filled glyphs       */
LV_FONT_DECLARE(lz_icons_18)   /* FILL=0, nav / rows / store        */
LV_FONT_DECLARE(lz_icons_14)   /* FILL=0, small chevrons / markers  */

/* ---- Icon glyph UTF-8 sequences ---- */
#define LZ_I_HUB          "\xEE\xA7\xB4"
#define LZ_I_LAN          "\xEE\xAC\xAF"
#define LZ_I_FORUM        "\xEE\xA2\xAF"
#define LZ_I_GROUP        "\xEE\xA8\xA1"
#define LZ_I_STOREFRONT   "\xEE\xA8\x92"
#define LZ_I_TERMINAL     "\xEE\xAE\x8E"
#define LZ_I_FOLDER       "\xEE\x8B\x87"
#define LZ_I_SETTINGS     "\xEE\xA2\xB8"
#define LZ_I_CHEV_L       "\xEE\x97\x8B"
#define LZ_I_CHEV_R       "\xEE\x97\x8C"
#define LZ_I_EDIT         "\xEF\x82\x97"
#define LZ_I_SEND         "\xEE\x85\xA3"
#define LZ_I_SEARCH       "\xEE\xBD\xBA"
#define LZ_I_STAR         "\xEF\x82\x9A"
#define LZ_I_LOCK         "\xEE\xA2\x99"
#define LZ_I_TAG          "\xEE\xA7\xAF"
#define LZ_I_CAMPAIGN     "\xEE\xBD\x89"
#define LZ_I_PERSON       "\xEF\x83\x93"
#define LZ_I_ROUTER       "\xEE\x8C\xA8"
#define LZ_I_SENSORS      "\xEE\x94\x9E"
#define LZ_I_KEY          "\xEE\x9C\xBC"
#define LZ_I_ROUTE        "\xEE\xAB\x8D"
#define LZ_I_PUBLIC       "\xEE\xA0\x8B"
#define LZ_I_GRAPHIC_EQ   "\xEE\x86\xB8"
#define LZ_I_CELL_TOWER   "\xEE\xAE\xBA"
#define LZ_I_WIFI         "\xEE\x98\xBE"
#define LZ_I_LOCATION     "\xEF\x87\x9B"
#define LZ_I_BRIGHTNESS   "\xEE\x86\xAC"
#define LZ_I_DARK_MODE    "\xEE\x94\x9C"
#define LZ_I_SCHEDULE     "\xEE\xBF\x96"
#define LZ_I_BOLT         "\xEE\xA8\x8B"
#define LZ_I_MONITORING   "\xEF\x86\x90"
#define LZ_I_DESCRIPTION  "\xEE\xA1\xB3"
#define LZ_I_CALCULATE    "\xEE\xA9\x9F"
#define LZ_I_NOTE         "\xEF\x87\xBC"
#define LZ_I_SATELLITE    "\xEE\xAC\xBA"
#define LZ_I_THERMOSTAT   "\xEF\x81\xB6"
#define LZ_I_DNS          "\xEE\xA1\xB5"
#define LZ_I_GAMEPAD      "\xEE\x8C\xB8"
#define LZ_I_MAP          "\xEE\x95\x9B"
#define LZ_I_CHAT         "\xEE\x83\x8B"

#endif
