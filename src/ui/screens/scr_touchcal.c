/* Touch calibration: tap three targets (top-left, top-right, bottom-left).
 * The platform captures the raw GT911 coordinate at each tap and derives the
 * swap/invert transform, so any T-Deck self-calibrates. The targets here must
 * match the order the platform expects: step 0 = TL, 1 = TR, 2 = BL. */
#include "../ui.h"
#include <stdio.h>

void lz_scr_touchcal(lv_obj_t *root)
{
    lv_obj_set_style_bg_color(root, lv_color_hex(0x0A0C10), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    lv_obj_t *title = lz_text(root, "Touch calibration", LZ_F_BODY, LZ_TEXT);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -14);

    char sub[48];
    snprintf(sub, sizeof sub, "Tap the green dot   (%d of 3)",
             (S.cal_step >= 0 && S.cal_step < 3) ? S.cal_step + 1 : 1);
    lv_obj_t *hint = lz_text(root, sub, LZ_F_SMALL, LZ_TEXT_3);
    lv_obj_align(hint, LV_ALIGN_CENTER, 0, 10);

    const int M = LZ_CAL_MARGIN;
    const int tx[3] = { M, LZ_W - M, M };
    const int ty[3] = { M, M, LZ_H - M };
    int step = (S.cal_step >= 0 && S.cal_step < 3) ? S.cal_step : 0;
    int cx = tx[step], cy = ty[step];

    lv_obj_t *ring = lz_dot(root, 24, lv_color_hex(0x143026));
    lv_obj_align(ring, LV_ALIGN_TOP_LEFT, cx - 12, cy - 12);
    lv_obj_t *dot = lz_dot(root, 10, LZ_MINT);
    lv_obj_align(dot, LV_ALIGN_TOP_LEFT, cx - 5, cy - 5);

    lz_nav_set(1, 0, NULL);   /* touch-only screen */
}
