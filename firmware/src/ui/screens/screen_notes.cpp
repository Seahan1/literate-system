#include "ui/screens/screen_notes.h"
#include "ui/screens/screen_module.h"
#include "ui/screens/screen_common.h"
#include "ui/styles.h"
#include "ui/state_view.h"
#include "ui/theme.h"
#include "config/layout.h"
#include "core/datastore.h"
#include <Arduino.h>

namespace {

static lv_obj_t* s_status;
static lv_obj_t* s_rows[NOTES_MAX];

static void notes_view_build(lv_obj_t* page) {
  s_status = build_header(page, "NOTES");
  for (uint8_t i = 0; i < NOTES_MAX; i++) {
    s_rows[i] = lv_label_create(page);
    lv_obj_add_style(s_rows[i], &S.body, 0);
    lv_obj_set_width(s_rows[i], SCREEN_W - 2 * SAFE_INSET);
    lv_label_set_long_mode(s_rows[i], LV_LABEL_LONG_DOT);
    lv_label_set_text(s_rows[i], "--");
    lv_obj_align(s_rows[i], LV_ALIGN_TOP_LEFT, SAFE_INSET, SAFE_INSET + 46 + i * 48);
  }
}

static void notes_view_update(void) {
  notes_rec_t notes = ds_get_notes();
  const beacon_theme_t* t = theme_active();
  char status[24];
  bool nonlive = sv_status(status, sizeof(status), &notes.hdr, now_s());
  txt_set(s_status, nonlive ? status : (notes.count ? "SYNCED" : "EMPTY"));
  txt_color(s_status, nonlive && sv_severe(notes.hdr.state) ? t->down : t->ink_dim);

  bool placeholder = sv_placeholder(notes.hdr.state);
  bool dim = sv_dim(notes.hdr.state);
  for (uint8_t i = 0; i < NOTES_MAX; i++) {
    bool visible = placeholder || i < notes.count;
    hidden_set(s_rows[i], !visible);
    if (placeholder) {
      txt_set(s_rows[i], "--");
    } else if (i < notes.count) {
      char row[NOTES_TITLE_LEN + 6];
      snprintf(row, sizeof(row), "%u  %s", (unsigned)(i + 1), notes.titles[i]);
      txt_set(s_rows[i], row);
    }
    txt_color(s_rows[i], dim ? t->ink_dim : t->ink);
  }
}

}

extern const screen_view_t notes_editorial_view = { notes_view_build, notes_view_update };
extern const screen_view_t notes_hud_view = { notes_view_build, notes_view_update };
extern const screen_view_t notes_calm_view = { notes_view_build, notes_view_update };
extern const screen_view_t notes_blueprint_view = { notes_view_build, notes_view_update };
extern const screen_view_t notes_led_view = { notes_view_build, notes_view_update };
extern const screen_view_t notes_oscilloscope_view = { notes_view_build, notes_view_update };
extern const screen_view_t notes_analog_view = { notes_view_build, notes_view_update };

SCREEN_MODULE_SIMPLE(notes, "NOTES", notes_module);
