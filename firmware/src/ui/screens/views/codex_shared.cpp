/* USER CODE BEGIN implementation */
#include "ui/screen.h"
#include "ui/state_view.h"
#include "ui/styles.h"
#include "ui/theme.h"
#include "ui/screens/screen_common.h"
#include "core/datastore.h"
#include "config/layout.h"
#include <Arduino.h>
#include <string.h>

namespace {

struct CodexWindow {
  lv_obj_t* name = nullptr;
  lv_obj_t* used = nullptr;
  lv_obj_t* left = nullptr;
  lv_obj_t* bar = nullptr;
  lv_obj_t* reset = nullptr;
};

static lv_obj_t* s_status = nullptr;
static lv_obj_t* s_title = nullptr;
static CodexWindow s_window[2];

static const usage_provider_t* find_codex(const usage_rec_t& usage)
{
  for (uint8_t i = 0; i < usage.count; ++i) {
    if (strcmp(usage.p[i].id, "codex") == 0) {
      return &usage.p[i];
    }
  }
  return nullptr;
}

static void make_window(lv_obj_t* page, int index, int offset_x, const char* label)
{
  const beacon_theme_t* theme = theme_active();
  lv_obj_t* card = lv_obj_create(page);
  lv_obj_remove_style_all(card);
  lv_obj_set_size(card, 174, 238);
  lv_obj_set_style_border_width(card, theme->stroke_hair, 0);
  lv_obj_set_style_border_color(card, theme->line, 0);
  lv_obj_set_style_radius(card, theme->radius, 0);
  lv_obj_align(card, LV_ALIGN_CENTER, offset_x, 12);
  lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

  CodexWindow& window = s_window[index];
  window.name = lv_label_create(card);
  lv_obj_add_style(window.name, &S.slot, 0);
  lv_label_set_text(window.name, label);
  lv_obj_align(window.name, LV_ALIGN_TOP_LEFT, 16, 16);

  window.used = lv_label_create(card);
  lv_obj_add_style(window.used, &S.display, 0);
  lv_label_set_text(window.used, "--");
  lv_obj_align(window.used, LV_ALIGN_TOP_LEFT, 16, 54);

  window.bar = lv_bar_create(card);
  lv_obj_set_size(window.bar, 142, 8);
  lv_obj_align(window.bar, LV_ALIGN_TOP_LEFT, 16, 118);
  lv_bar_set_range(window.bar, 0, 100);
  lv_bar_set_value(window.bar, 0, LV_ANIM_OFF);
  bg_opa_if(window.bar, LV_OPA_COVER, LV_PART_MAIN);
  bg_opa_if(window.bar, LV_OPA_COVER, LV_PART_INDICATOR);
  bg_color_if(window.bar, theme->line, LV_PART_MAIN);
  bg_color_if(window.bar, theme->accent, LV_PART_INDICATOR);

  window.left = lv_label_create(card);
  lv_obj_add_style(window.left, &S.slot, 0);
  lv_label_set_text(window.left, "-- left");
  lv_obj_align(window.left, LV_ALIGN_TOP_LEFT, 16, 148);

  window.reset = lv_label_create(card);
  lv_obj_add_style(window.reset, &S.slot, 0);
  lv_label_set_text(window.reset, "reset --");
  lv_obj_align(window.reset, LV_ALIGN_TOP_LEFT, 16, 182);
}

static void build(lv_obj_t* page)
{
  const beacon_theme_t* theme = theme_active();
  s_status = build_header(page, "CODEX");

  s_title = lv_label_create(page);
  lv_obj_add_style(s_title, &S.display, 0);
  lv_label_set_text(s_title, "CODEX QUOTA");
  lv_obj_align(s_title, LV_ALIGN_TOP_LEFT, SAFE_INSET, SAFE_INSET + 34);
  txt_color(s_title, theme->accent);

  make_window(page, 0, -96, "5H WINDOW");
  make_window(page, 1, 96, "7D WINDOW");
}

static void set_window(CodexWindow& window, const usage_window_t& value, bool placeholder,
                       bool dim, const beacon_theme_t* theme, uint32_t now)
{
  if (placeholder || value.pct < 0) {
    txt_set(window.used, "--");
    txt_set(window.left, "-- left");
    txt_set(window.reset, "reset --");
    hidden_set(window.bar, true);
    txt_color(window.used, theme->ink_dim);
    txt_color(window.left, theme->ink_dim);
    return;
  }

  int used = value.pct > 100 ? 100 : value.pct;
  int left = 100 - used;
  txt_fmt(window.used, "%d%% USED", used);
  txt_fmt(window.left, "%d%% LEFT", left);
  char reset[12];
  reset_str(reset, sizeof(reset), value.reset, now);
  txt_fmt(window.reset, "reset %s", reset);
  hidden_set(window.bar, false);
  bar_set_if(window.bar, used);
  bg_color_if(window.bar, dim ? theme->line : theme->accent, LV_PART_INDICATOR);
  txt_color(window.used, dim ? theme->ink_dim : theme->ink);
  txt_color(window.left, dim ? theme->ink_dim : theme->ink);
  txt_color(window.reset, theme->ink_dim);
}

static void update(void)
{
  usage_rec_t usage = ds_get_usage();
  const beacon_theme_t* theme = theme_active();
  uint32_t now = now_s();
  bool placeholder = sv_placeholder(usage.hdr.state);
  const usage_provider_t* codex = find_codex(usage);
  bool dim = sv_dim(usage.hdr.state) || (codex != nullptr && codex->stale);
  usage_window_t none = usage_none();

  slot_set(s_status, "HUB 30S", &usage.hdr, now);
  set_window(s_window[0], codex ? codex->h5 : none, placeholder, dim, theme, now);
  set_window(s_window[1], codex ? codex->d7 : none, placeholder, dim, theme, now);
}

}

extern const screen_view_t codex_editorial_view = { build, update };
extern const screen_view_t codex_hud_view = { build, update };
extern const screen_view_t codex_calm_view = { build, update };
extern const screen_view_t codex_blueprint_view = { build, update };
extern const screen_view_t codex_led_view = { build, update };
extern const screen_view_t codex_oscilloscope_view = { build, update };
extern const screen_view_t codex_analog_view = { build, update };
/* USER CODE END implementation */
