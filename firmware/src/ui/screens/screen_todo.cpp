#include "ui/screens/screen_todo.h"
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
static lv_obj_t* s_today_caption;
static lv_obj_t* s_overdue_caption;
static lv_obj_t* s_today[TODO_MAX];
static lv_obj_t* s_overdue[TODO_MAX];

static lv_obj_t* make_caption(lv_obj_t* page, const char* text, int y) {
  lv_obj_t* label = lv_label_create(page);
  lv_obj_add_style(label, &S.eyebrow, 0);
  lv_label_set_text(label, text);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, SAFE_INSET, y);
  return label;
}

static void make_rows(lv_obj_t* page, lv_obj_t** rows, int y) {
  for (uint8_t i = 0; i < TODO_MAX; i++) {
    rows[i] = lv_label_create(page);
    lv_obj_add_style(rows[i], &S.body, 0);
    lv_obj_set_width(rows[i], SCREEN_W - 2 * SAFE_INSET);
    lv_label_set_long_mode(rows[i], LV_LABEL_LONG_DOT);
    lv_label_set_text(rows[i], "--");
    lv_obj_align(rows[i], LV_ALIGN_TOP_LEFT, SAFE_INSET, y + i * 42);
  }
}

static void todo_view_build(lv_obj_t* page) {
  s_status = build_header(page, "TODO");
  s_today_caption = make_caption(page, "TODAY", SAFE_INSET + 46);
  make_rows(page, s_today, SAFE_INSET + 68);
  s_overdue_caption = make_caption(page, "OVERDUE", SAFE_INSET + 238);
  make_rows(page, s_overdue, SAFE_INSET + 260);
}

static void update_rows(lv_obj_t** rows, uint8_t count, const char titles[][TODO_TITLE_LEN],
                        bool placeholder, bool dim, const beacon_theme_t* t) {
  for (uint8_t i = 0; i < TODO_MAX; i++) {
    bool visible = placeholder || i < count;
    hidden_set(rows[i], !visible);
    if (placeholder) {
      txt_set(rows[i], "--");
    } else if (i < count) {
      char row[TODO_TITLE_LEN + 6];
      snprintf(row, sizeof(row), "%u  %s", (unsigned)(i + 1), titles[i]);
      txt_set(rows[i], row);
    }
    txt_color(rows[i], dim ? t->ink_dim : t->ink);
  }
}

static void todo_view_update(void) {
  todo_rec_t todo = ds_get_todo();
  const beacon_theme_t* t = theme_active();
  char status[24];
  bool nonlive = sv_status(status, sizeof(status), &todo.hdr, now_s());
  bool has_tasks = todo.today_count || todo.overdue_count;
  txt_set(s_status, nonlive ? status : (has_tasks ? "SYNCED" : "EMPTY"));
  txt_color(s_status, nonlive && sv_severe(todo.hdr.state) ? t->down : t->ink_dim);
  txt_color(s_today_caption, t->accent);
  txt_color(s_overdue_caption, t->down);

  bool placeholder = sv_placeholder(todo.hdr.state);
  bool dim = sv_dim(todo.hdr.state);
  update_rows(s_today, todo.today_count, todo.today, placeholder, dim, t);
  update_rows(s_overdue, todo.overdue_count, todo.overdue, placeholder, dim, t);
}

}

extern const screen_view_t todo_editorial_view = { todo_view_build, todo_view_update };
extern const screen_view_t todo_hud_view = { todo_view_build, todo_view_update };
extern const screen_view_t todo_calm_view = { todo_view_build, todo_view_update };
extern const screen_view_t todo_blueprint_view = { todo_view_build, todo_view_update };
extern const screen_view_t todo_led_view = { todo_view_build, todo_view_update };
extern const screen_view_t todo_oscilloscope_view = { todo_view_build, todo_view_update };
extern const screen_view_t todo_analog_view = { todo_view_build, todo_view_update };

SCREEN_MODULE_SIMPLE(todo, "TODO", todo_module);
