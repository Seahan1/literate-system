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

struct CodexGauge {
  lv_obj_t* arc = nullptr;
  lv_obj_t* pct = nullptr;
  lv_obj_t* caption = nullptr;
  lv_obj_t* reset = nullptr;
};

static lv_obj_t* s_status = nullptr;
static CodexGauge s_gauge;

static const usage_provider_t* find_codex(const usage_rec_t& usage)
{
  for (uint8_t i = 0; i < usage.count; ++i) {
    if (strcmp(usage.p[i].id, "codex") == 0) {
      return &usage.p[i];
    }
  }
  return nullptr;
}

static void make_gauge(lv_obj_t* page, const beacon_theme_t* theme)
{
  constexpr int arc_size = 276;
  s_gauge.arc = lv_arc_create(page);
  lv_obj_set_size(s_gauge.arc, arc_size, arc_size);
  lv_obj_align(s_gauge.arc, LV_ALIGN_CENTER, 0, 16);
  lv_arc_set_rotation(s_gauge.arc, 135);
  lv_arc_set_bg_angles(s_gauge.arc, 0, 270);
  lv_arc_set_range(s_gauge.arc, 0, 100);
  lv_arc_set_value(s_gauge.arc, 0);
  lv_obj_remove_style(s_gauge.arc, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(s_gauge.arc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_color(s_gauge.arc, theme->line, LV_PART_MAIN);
  lv_obj_set_style_arc_color(s_gauge.arc, theme->accent, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(s_gauge.arc, 14, LV_PART_MAIN);
  lv_obj_set_style_arc_width(s_gauge.arc, 14, LV_PART_INDICATOR);

  s_gauge.pct = lv_label_create(page);
  lv_obj_add_style(s_gauge.pct, &S.hero, 0);
  lv_obj_set_width(s_gauge.pct, arc_size);
  lv_obj_set_style_text_align(s_gauge.pct, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_text(s_gauge.pct, "--");
  lv_obj_align_to(s_gauge.pct, s_gauge.arc, LV_ALIGN_CENTER, 0, -10);

  s_gauge.caption = lv_label_create(page);
  lv_obj_add_style(s_gauge.caption, &S.display, 0);
  lv_obj_set_width(s_gauge.caption, arc_size);
  lv_obj_set_style_text_align(s_gauge.caption, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_text(s_gauge.caption, "7D USAGE");
  lv_obj_align_to(s_gauge.caption, s_gauge.arc, LV_ALIGN_CENTER, 0, 48);

  s_gauge.reset = lv_label_create(page);
  lv_obj_add_style(s_gauge.reset, &S.slot, 0);
  lv_label_set_text(s_gauge.reset, "RESET --");
  lv_obj_align_to(s_gauge.reset, s_gauge.arc, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
}

static void build(lv_obj_t* page)
{
  const beacon_theme_t* theme = theme_active();
  s_status = build_header(page, "CODEX");
  make_gauge(page, theme);
}

static void set_gauge(const usage_window_t& value, bool placeholder, bool dim,
                      const beacon_theme_t* theme, uint32_t now)
{
  char reset[12];
  reset_str(reset, sizeof(reset), placeholder ? 0 : value.reset, now);
  txt_fmt(s_gauge.reset, "RESET %s", reset);

  if (placeholder || value.pct < 0) {
    txt_set(s_gauge.pct, "--");
    if (lv_arc_get_value(s_gauge.arc) != 0) lv_arc_set_value(s_gauge.arc, 0);
    txt_color(s_gauge.pct, theme->ink_dim);
    lv_obj_set_style_arc_color(s_gauge.arc, theme->line, LV_PART_INDICATOR);
    return;
  }

  int used = value.pct > 100 ? 100 : value.pct;
  txt_fmt(s_gauge.pct, "%d%%", used);
  if (lv_arc_get_value(s_gauge.arc) != used) lv_arc_set_value(s_gauge.arc, used);
  txt_color(s_gauge.pct, dim ? theme->ink_dim : theme->ink);
  lv_obj_set_style_arc_color(s_gauge.arc, dim ? theme->line : theme->accent, LV_PART_INDICATOR);
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
  set_gauge(codex ? codex->d7 : none, placeholder, dim, theme, now);
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
