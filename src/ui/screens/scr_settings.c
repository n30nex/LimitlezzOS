/* Settings (airtime scheduler, first-class network toggles, grouped rows)
 * + System (battery ring & stats, opened via Settings -> Device) */
#include "../ui.h"
#include <stdio.h>

static const char *REGIONS[]  = { "US", "EU868", "ANZ", "CN", "JP" };
static const char *PRESETS[]  = { "Long/Fast", "Long/Slow", "Med/Fast", "Short/Fast" };
static const char *TXPOW[]    = { "Low", "Medium", "High", "Max" };
static const int   TXPOW_DBM[] = { 2, 8, 17, 22 };
static const char *TIMEOUTS[] = { "15s", "30s", "1m", "5m", "Never" };
static const char *KBLIGHT[]  = { "Auto", "On", "Off" };

enum { ROW_VALUE, ROW_TOGGLE, ROW_SLIDER, ROW_NAV };

typedef struct {
    const char *label, *icon;
    int kind;
} srow_t;

/* focus indices 2..10 map onto this table (this is a dark-only OS — no dark
 * mode toggle; Wi-Fi opens its own setup screen) */
static const srow_t SROWS[10] = {
    { "Region",           LZ_I_PUBLIC,     ROW_VALUE  },  /* f=2  */
    { "Modem preset",     LZ_I_GRAPHIC_EQ, ROW_VALUE  },  /* f=3  */
    { "TX power",         LZ_I_CELL_TOWER, ROW_VALUE  },  /* f=4  */
    { "Wi-Fi",            LZ_I_WIFI,       ROW_NAV    },  /* f=5  */
    { "GPS",              LZ_I_LOCATION,   ROW_TOGGLE },  /* f=6  */
    { "Brightness",       LZ_I_BRIGHTNESS, ROW_SLIDER },  /* f=7  */
    { "Keyboard light",   LZ_I_BRIGHTNESS, ROW_VALUE  },  /* f=8  */
    { "Sleep after",      LZ_I_SCHEDULE,   ROW_VALUE  },  /* f=9  */
    { "Power saving",     LZ_I_BOLT,       ROW_TOGGLE },  /* f=10 */
    { "System & battery", LZ_I_MONITORING, ROW_NAV    },  /* f=11 */
};
#define SETTINGS_FOCUS_COUNT 12   /* 2 network rows + 10 SROWS */

static void cycle(int *idx, int n) { *idx = (*idx + 1) % n; }

/* focus skips the MeshCore network row while it's locked (Stage 1) */
static bool settings_disabled(int idx) { return idx == 1 && !LZ_MESHCORE_ENABLED; }

static void settings_activate(int f)
{
    switch(f) {
        case 0: S.net_mt = !S.net_mt; break;
        case 1: if(!LZ_MESHCORE_ENABLED) return; S.net_mc = !S.net_mc; break;
        case 2: cycle(&S.settings.region, 5); break;
        case 3: cycle(&S.settings.preset, 4); break;
        case 4: cycle(&S.settings.tx, 4); lz_backend_set_tx_power(TXPOW_DBM[S.settings.tx]); break;
        case 5: lz_go(LZ_V_WIFI); return;          /* Wi-Fi setup */
        case 6: S.settings.gps = !S.settings.gps; break;
        case 7: return;                            /* slider: left/right adjusts */
        case 8: cycle(&S.settings.kb_light, 3); break;
        case 9: cycle(&S.settings.timeout, 5); break;
        case 10: S.settings.save = !S.settings.save; break;
        case 11: lz_go(LZ_V_SYSTEM); return;
        default: return;
    }
    lz_rebuild();
}

static lv_obj_t *group_card(lv_obj_t *body, const char *title)
{
    lv_obj_t *hd = lz_text(body, title, LZ_F_SMALL, LZ_TEXT_3);
    lv_obj_set_style_pad_left(hd, 4, 0);
    lv_obj_set_style_pad_top(hd, 4, 0);
    lv_obj_t *card = lz_box(body);
    lv_obj_set_width(card, lv_pct(100));
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(card, LZ_RADIUS_CARD, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, LZ_CARD_BORDER, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    return card;
}

static lv_obj_t *setting_row_base(lv_obj_t *card, bool focused, bool last)
{
    lv_obj_t *row = lz_box(card);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(row, focused ? LZ_ROW_FOCUS_BG : LZ_ROW_BG, 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    if(focused) {
        lv_obj_set_style_border_width(row, 2, 0);
        lv_obj_set_style_border_color(row, LZ_FOCUS, 0);
    } else if(!last) {
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(0x1E232A), 0);
    }
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(row, 10, 0);
    lv_obj_set_style_pad_ver(row, 7, 0);
    lv_obj_set_style_pad_column(row, 9, 0);
    return row;
}

static void value_chevron(lv_obj_t *row, const char *value)
{
    lz_text(row, value, LZ_F_SMALL, LZ_TEXT_VALUE);
    lz_icon(row, LZ_I_CHEV_R, &lz_icons_14, LZ_TEXT_3);
}

void lz_scr_settings(lv_obj_t *root)
{
    bool mt = S.net_mt, mc = S.net_mc, both = mt && mc;
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lz_navbar(root, "Settings", NULL);

    lv_obj_t *body = lz_vflex(root);
    lv_obj_set_style_pad_top(body, 8, 0);
    lv_obj_set_style_pad_hor(body, 9, 0);
    lv_obj_set_style_pad_bottom(body, 10, 0);
    lv_obj_set_style_pad_row(body, 5, 0);
    lz_nav_set_scroll(body);

    /* --- airtime scheduler card --- */
    lv_obj_t *air = lz_card(body);
    lv_obj_set_height(air, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(air, 9, 0);
    lv_obj_set_flex_flow(air, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(air, 6, 0);

    lv_obj_t *ahead = lz_box(air);
    lv_obj_set_width(ahead, lv_pct(100));
    lv_obj_set_height(ahead, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(ahead, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ahead, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lz_text(ahead, "AIRTIME SCHEDULER", LZ_F_SMALL, lv_color_hex(0x7F868F));
    const char *alabel = both ? "Split 50 / 50" : mt ? "Meshtastic 100%"
                       : mc ? "MeshCore 100%" : "Idle";
    lz_text(ahead, alabel, LZ_F_SMALL, LZ_TEXT_VALUE);

    lv_obj_t *abar = lz_box(air);
    lv_obj_set_width(abar, lv_pct(100));
    lv_obj_set_height(abar, 7);
    lv_obj_set_style_radius(abar, 4, 0);
    lv_obj_set_style_bg_color(abar, LZ_INSET_BG, 0);
    lv_obj_set_style_bg_opa(abar, LV_OPA_COVER, 0);
    lv_obj_set_style_clip_corner(abar, true, 0);
    lv_obj_set_flex_flow(abar, LV_FLEX_FLOW_ROW);
    int mt_pct = mt ? (both ? 50 : 100) : 0;
    int mc_pct = mc ? (both ? 50 : 100) : 0;
    if(mt_pct) {
        lv_obj_t *seg = lz_box(abar);
        lv_obj_set_size(seg, lv_pct(mt_pct), lv_pct(100));
        lv_obj_set_style_bg_color(seg, LZ_AIRTIME_MT, 0);
        lv_obj_set_style_bg_opa(seg, LV_OPA_COVER, 0);
    }
    if(mc_pct) {
        lv_obj_t *seg = lz_box(abar);
        lv_obj_set_size(seg, lv_pct(mc_pct), lv_pct(100));
        lv_obj_set_style_bg_color(seg, LZ_AIRTIME_MC, 0);
        lv_obj_set_style_bg_opa(seg, LV_OPA_COVER, 0);
    }

    const char *anote = both
        ? "Both radios share airtime. Disable one to give the other full throughput and lower latency."
        : (mt || mc)
        ? "One network disabled - the active radio now has full airtime: faster delivery, lower latency."
        : "Both networks disabled. Enable one to start receiving.";
    lv_obj_t *noteL = lz_text(air, anote, LZ_F_SMALL, lv_color_hex(0x7F868F));
    lv_label_set_long_mode(noteL, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(noteL, lv_pct(100));

    /* --- NETWORKS: first-class toggles (MeshCore locked until Stage 2) --- */
    lv_obj_t *nets = group_card(body, "NETWORKS");
    for(int i = 0; i < 2; i++) {
        bool is_mt = i == 0;
        bool on = is_mt ? mt : mc;
        bool locked = !is_mt && !LZ_MESHCORE_ENABLED;
        lv_obj_t *row = setting_row_base(nets, S.focus == i, i == 1);
        if(locked) lv_obj_set_style_opa(row, LV_OPA_50, 0);
        lv_obj_t *tile = lz_box(row);
        lv_obj_set_size(tile, 28, 28);
        lv_obj_set_style_radius(tile, 8, 0);
        lv_obj_set_style_bg_color(tile, is_mt ? LZ_IDTILE_MT : LZ_IDTILE_MC, 0);
        lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
        lv_obj_t *ic = lz_icon(tile, is_mt ? LZ_I_HUB : LZ_I_LAN, &lz_icons_16f,
                               is_mt ? LZ_ON_CYAN : LZ_ON_AMBER);
        lv_obj_center(ic);
        lv_obj_t *cl = lz_box(row);
        lv_obj_set_flex_grow(cl, 1);
        lv_obj_set_height(cl, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(cl, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(cl, 1, 0);
        lz_text(cl, is_mt ? "Meshtastic" : "MeshCore", LZ_F_BODY, LZ_TEXT);
        char mtsub[40];
        snprintf(mtsub, sizeof mtsub, "Node %s - US - LongFast", lz_svc_identity()->short_name);
        const char *sub = is_mt
            ? (mt ? mtsub : "Disabled - history kept")
            : (locked ? "Coming soon" : mc ? "Companion - 5 contacts" : "Disabled - history kept");
        lz_text(cl, sub, LZ_F_SMALL, is_mt ? LZ_TEXT_2 : lv_color_hex(0x988E7C));
        if(locked) {
            lv_obj_t *chip = lz_box(row);
            lv_obj_set_size(chip, LV_SIZE_CONTENT, 16);
            lv_obj_set_style_radius(chip, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(chip, lv_color_hex(0x252A31), 0);
            lv_obj_set_style_bg_opa(chip, LV_OPA_COVER, 0);
            lv_obj_set_style_pad_hor(chip, 7, 0);
            lv_obj_t *cl2 = lz_text(chip, "Soon", LZ_F_SMALL, lv_color_hex(0x868F99));
            lv_obj_center(cl2);
        } else {
            lz_toggle(row, on, is_mt ? LZ_TRACK_MT : LZ_TRACK_MC);
        }
    }

    /* --- grouped rows --- */
    static const struct { const char *title; int first, count; } GROUPS[5] = {
        { "RADIO",        2, 3 }, { "CONNECTIVITY", 5, 2 },
        { "DISPLAY",      7, 3 }, { "POWER",       10, 1 }, { "DEVICE", 11, 1 },
    };
    char bval[8];
    for(int g = 0; g < 5; g++) {
        lv_obj_t *card = group_card(body, GROUPS[g].title);
        for(int k = 0; k < GROUPS[g].count; k++) {
            int f = GROUPS[g].first + k;
            const srow_t *r = &SROWS[f - 2];
            lv_obj_t *row = setting_row_base(card, S.focus == f, k == GROUPS[g].count - 1);
            lz_icon(row, r->icon, &lz_icons_18, lv_color_hex(0xAAB0B9));
            lv_obj_t *lbl = lz_text(row, r->label, LZ_F_BODY, LZ_TEXT_SETTING);
            lv_obj_set_flex_grow(lbl, 1);
            switch(f) {
                case 2: value_chevron(row, REGIONS[S.settings.region]); break;
                case 3: value_chevron(row, PRESETS[S.settings.preset]); break;
                case 4: value_chevron(row, TXPOW[S.settings.tx]); break;
                case 5: value_chevron(row, lz_wifi_connected() ? lz_wifi_connected()
                                          : (lz_wifi_enabled() ? "On" : "Off")); break;
                case 6: lz_toggle(row, S.settings.gps, LZ_TOGGLE_ON); break;
                case 7: {
                    /* brightness slider (left/right adjusts while focused) */
                    lv_obj_t *track = lz_box(row);
                    lv_obj_set_size(track, 96, 5);
                    lv_obj_set_style_radius(track, 3, 0);
                    lv_obj_set_style_bg_color(track, lv_color_hex(0x22272F), 0);
                    lv_obj_set_style_bg_opa(track, LV_OPA_COVER, 0);
                    lv_obj_t *fillb = lz_box(track);
                    lv_obj_set_size(fillb, lv_pct(S.settings.bright), 5);
                    lv_obj_set_style_radius(fillb, 3, 0);
                    lv_obj_set_style_bg_color(fillb, LZ_SLIDER_HI, 0);
                    lv_obj_set_style_bg_opa(fillb, LV_OPA_COVER, 0);
                    lv_obj_t *knob = lz_dot(track, 11, LZ_KNOB);
                    lv_obj_align(knob, LV_ALIGN_LEFT_MID, (96 * S.settings.bright) / 100 - 5, 0);
                    break;
                }
                case 8: value_chevron(row, KBLIGHT[S.settings.kb_light]); break;
                case 9: value_chevron(row, TIMEOUTS[S.settings.timeout]); break;
                case 10: lz_toggle(row, S.settings.save, LZ_TOGGLE_ON); break;
                case 11: value_chevron(row, "87% - 24C"); break;
            }
            lz_nav_track(row, f);
        }
    }
    (void)bval;

    /* --- ABOUT (static) --- */
    lv_obj_t *about = group_card(body, "ABOUT");
    const char *ks[2] = { "Device", "Firmware" };
    const char *vs[2] = { "LilyGo T-Deck", "LimitlezzOS Alpha 0.1" };
    for(int i = 0; i < 2; i++) {
        lv_obj_t *r = lz_box(about);
        lv_obj_set_width(r, lv_pct(100));
        lv_obj_set_height(r, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(r, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(r, 10, 0);
        lv_obj_set_style_pad_ver(r, 7, 0);
        if(i == 0) {
            lv_obj_set_style_border_side(r, LV_BORDER_SIDE_BOTTOM, 0);
            lv_obj_set_style_border_width(r, 1, 0);
            lv_obj_set_style_border_color(r, lv_color_hex(0x1E232A), 0);
        }
        lz_text(r, ks[i], LZ_F_SMALL, lv_color_hex(0x8B939C));
        lz_text(r, vs[i], LZ_F_SMALL, LZ_TEXT_STRONG);
    }

    lz_nav_set(1, SETTINGS_FOCUS_COUNT, settings_activate);
    lz_nav_set_skip(settings_disabled);
}

/* ===== System ===== */

void lz_scr_system(lv_obj_t *root)
{
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lz_navbar(root, NULL, "Settings");

    lv_obj_t *body = lz_vflex(root);
    lv_obj_set_style_pad_top(body, 10, 0);
    lv_obj_set_style_pad_hor(body, 11, 0);
    lv_obj_set_style_pad_bottom(body, 10, 0);
    lv_obj_set_style_pad_row(body, 8, 0);
    lz_nav_set_scroll(body);

    /* battery ring + summary */
    lv_obj_t *hero = lz_box(body);
    lv_obj_set_width(hero, lv_pct(100));
    lv_obj_set_height(hero, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(hero, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hero, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(hero, 13, 0);

    lv_obj_t *ring = lv_arc_create(hero);
    lv_obj_remove_style_all(ring);
    lv_obj_set_size(ring, 58, 58);
    lv_arc_set_rotation(ring, 270);
    lv_arc_set_bg_angles(ring, 0, 360);
    lv_arc_set_range(ring, 0, 100);
    lv_obj_remove_style(ring, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(ring, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ring, lv_color_hex(0x20252D), LV_PART_MAIN);
    lv_obj_set_style_arc_width(ring, 6, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(ring, LZ_GREEN_BRIGHT, LV_PART_INDICATOR);

    /* real values from the platform (sim shows demo numbers) */
    lz_sysinfo_t si; lz_svc_sysinfo(&si);
    bool have_batt = si.battery_pct >= 0;
    lv_arc_set_value(ring, have_batt ? si.battery_pct : 0);
    char rpb[8]; snprintf(rpb, sizeof rpb, have_batt ? "%d%%" : "USB", si.battery_pct);
    lv_obj_t *rp = lz_text(ring, rpb, LZ_F_HEAD, LZ_TEXT);
    lv_obj_align(rp, LV_ALIGN_CENTER, 0, -4);
    char rvb[10]; snprintf(rvb, sizeof rvb, "%.2fV", (double)si.battery_v);
    lv_obj_t *rv = lz_text(ring, rvb, LZ_F_SMALL, lv_color_hex(0x7F868F));
    lv_obj_align(rv, LV_ALIGN_CENTER, 0, 12);

    lv_obj_t *hc = lz_box(hero);
    lv_obj_set_flex_grow(hc, 1);
    lv_obj_set_height(hc, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(hc, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(hc, 2, 0);
    lz_text(hc, si.usb ? "On USB power" : (have_batt ? "On battery" : "Powered"), LZ_F_HEAD, LZ_TEXT);
    char vline[24]; snprintf(vline, sizeof vline, "%.2f V", (double)si.battery_v);
    lz_text(hc, vline, LZ_F_SMALL, LZ_TEXT_2);
    lv_obj_t *chip = lz_box(hc);
    lv_obj_set_size(chip, LV_SIZE_CONTENT, 16);
    lv_obj_set_style_radius(chip, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(chip, LZ_GREEN_BG, 0);
    lv_obj_set_style_bg_opa(chip, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_hor(chip, 7, 0);
    lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(chip, 4, 0);
    /* health baseline from resting voltage: a healthy LiPo sits ~3.7-4.2V under
     * light load; sustained low voltage means it needs service/charging */
    const char *health; lv_color_t hcol, hdot;
    if(si.usb)                  { health = "Powered over USB"; hcol = LZ_GREEN_TXT; hdot = LZ_GREEN_BRIGHT; }
    else if(si.battery_v >= 3.7f){ health = "Battery healthy";  hcol = LZ_GREEN_TXT; hdot = LZ_GREEN_BRIGHT; }
    else if(si.battery_v >= 3.4f){ health = "Battery OK";       hcol = LZ_GREEN_TXT; hdot = LZ_GREEN_BRIGHT; }
    else if(si.battery_v >= 3.2f){ health = "Battery low";      hcol = LZ_SNR_MID;   hdot = LZ_SNR_MID; }
    else                         { health = "Service battery";  hcol = LZ_SNR_BAD;   hdot = LZ_SNR_BAD; }
    lv_obj_set_style_bg_color(chip, lv_color_hex(0x1A2520), 0);
    lz_dot(chip, 5, hdot);
    lz_text(chip, health, LZ_F_SMALL, hcol);

    /* live stat rows */
    char cpuv[16], ramv[24], flashv[24], tempv[12], upv[20];
    snprintf(cpuv, sizeof cpuv, "%d MHz", si.cpu_mhz);
    snprintf(ramv, sizeof ramv, "%d / %d KB", si.ram_used_kb, si.ram_total_kb);
    snprintf(flashv, sizeof flashv, "%.1f / %.1f MB", si.flash_used_kb / 1024.0, si.flash_total_kb / 1024.0);
    snprintf(tempv, sizeof tempv, "%d C", si.temp_c);
    uint32_t up = si.uptime_s;
    if(up >= 86400) snprintf(upv, sizeof upv, "%ud %02u:%02u", up/86400, (up%86400)/3600, (up%3600)/60);
    else            snprintf(upv, sizeof upv, "%02u:%02u:%02u", up/3600, (up%3600)/60, up%60);
    struct { const char *label, *value; int pct; bool bar; } rows[5] = {
        { "CPU",           cpuv,   0,  false },
        { "RAM",           ramv,   si.ram_total_kb ? si.ram_used_kb * 100 / si.ram_total_kb : 0, true },
        { "Flash storage", flashv, si.flash_total_kb ? si.flash_used_kb * 100 / si.flash_total_kb : 0, true },
        { "Temperature",   tempv,  si.temp_c > 0 ? (si.temp_c * 100 / 90) : 0, true },
        { "Uptime",        upv,    0,  false },
    };
    const lv_color_t bar_colors[5] = { LZ_BAR_CYAN, LZ_BAR_CYAN, LZ_BAR_MINT,
                                       LZ_BAR_GREEN, LZ_BAR_PURPLE };
    for(int i = 0; i < 5; i++) {
        lv_obj_t *st = lz_box(body);
        lv_obj_set_width(st, lv_pct(100));
        lv_obj_set_height(st, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(st, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(st, 3, 0);
        lv_obj_t *hd = lz_box(st);
        lv_obj_set_width(hd, lv_pct(100));
        lv_obj_set_height(hd, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(hd, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(hd, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lz_text(hd, rows[i].label, LZ_F_SMALL, LZ_TEXT_VALUE);
        lz_text(hd, rows[i].value, LZ_F_SMALL, LZ_TEXT_STRONG);
        if(!rows[i].bar) continue;
        int p = rows[i].pct; if(p < 0) p = 0; if(p > 100) p = 100;
        lv_obj_t *track = lz_box(st);
        lv_obj_set_size(track, lv_pct(100), 5);
        lv_obj_set_style_radius(track, 3, 0);
        lv_obj_set_style_bg_color(track, LZ_INSET_BG, 0);
        lv_obj_set_style_bg_opa(track, LV_OPA_COVER, 0);
        lv_obj_t *fillb = lz_box(track);
        lv_obj_set_size(fillb, lv_pct(p), 5);
        lv_obj_set_style_radius(fillb, 3, 0);
        lv_obj_set_style_bg_color(fillb, bar_colors[i], 0);
        lv_obj_set_style_bg_opa(fillb, LV_OPA_COVER, 0);
    }

    /* radio cards — live TX/RX + airtime */
    lz_radio_stats_t rs; lz_svc_radio_stats(&rs);
    char txrx[20], util[12];
    snprintf(txrx, sizeof txrx, "%u / %u", (unsigned)rs.tx_count, (unsigned)rs.rx_count);
    snprintf(util, sizeof util, "%.1f%%", (double)rs.util_pct);
    lv_obj_t *cards = lz_box(body);
    lv_obj_set_width(cards, lv_pct(100));
    lv_obj_set_height(cards, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cards, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(cards, 7, 0);
    const char *cl[2] = { "LoRa TX / RX", "Air utilization" };
    const char *cv[2] = { txrx, util };
    for(int i = 0; i < 2; i++) {
        lv_obj_t *c = lz_box(cards);
        lv_obj_set_flex_grow(c, 1);
        lv_obj_set_height(c, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(c, 9, 0);
        lv_obj_set_style_bg_color(c, LZ_CARD_BG, 0);
        lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(c, 1, 0);
        lv_obj_set_style_border_color(c, LZ_CARD_BORDER, 0);
        lv_obj_set_style_pad_all(c, 8, 0);
        lv_obj_set_flex_flow(c, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(c, 2, 0);
        lz_text(c, cl[i], LZ_F_SMALL, lv_color_hex(0x7F868F));
        lz_text(c, cv[i], LZ_F_BODY, LZ_TEXT_STRONG);
    }

    lz_nav_set(1, 0, NULL);
}
