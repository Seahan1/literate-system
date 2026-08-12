/* USER CODE BEGIN status_screen */
#include "ui/screens/screen_status.h"
#include "ui/screens/screen_module.h"
#include "ui/screens/screen_common.h"
#include "ui/theme.h"
#include "config/version.h"
#include "core/about_format.h"
#include "core/net.h"
#include "hal/power.h"
#include <Arduino.h>
#include <esp_system.h>

namespace {

static lv_obj_t* s_state = nullptr;
static lv_obj_t* s_uptime = nullptr;
static lv_obj_t* s_wifi = nullptr;
static lv_obj_t* s_battery = nullptr;
static lv_obj_t* s_heap = nullptr;
static lv_obj_t* s_version = nullptr;

static lv_obj_t* status_row(lv_obj_t* page, const char* name, int y) {
  lv_obj_t* row = lv_obj_create(page);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, SCREEN_W - 2 * SAFE_INSET, 42);
  lv_obj_align(row, LV_ALIGN_TOP_LEFT, SAFE_INSET, y);
  lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t* key = lv_label_create(row);
  lv_obj_add_style(key, &S.display, 0);
  lv_label_set_text(key, name);
  lv_obj_align(key, LV_ALIGN_LEFT_MID, 0, 0);

  lv_obj_t* value = lv_label_create(row);
  lv_obj_add_style(value, &S.slot, 0);
  lv_obj_set_width(value, 250);
  lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_align(value, LV_ALIGN_RIGHT_MID, 0, 0);

  lv_obj_t* rule = lv_obj_create(page);
  lv_obj_remove_style_all(rule);
  lv_obj_add_style(rule, &S.hairline, 0);
  lv_obj_set_size(rule, SCREEN_W - 2 * SAFE_INSET, 1);
  lv_obj_align(rule, LV_ALIGN_TOP_LEFT, SAFE_INSET, y + 42);
  return value;
}

static void status_view_build(lv_obj_t* page) {
  build_header(page, "STATUS");

  s_state = lv_label_create(page);
  lv_obj_add_style(s_state, &S.hero, 0);
  lv_obj_align(s_state, LV_ALIGN_TOP_LEFT, SAFE_INSET, SAFE_INSET + 38);

  int y = SAFE_INSET + 124;
  s_uptime = status_row(page, "UPTIME", y); y += 48;
  s_wifi = status_row(page, "WI-FI", y); y += 48;
  s_battery = status_row(page, "BATTERY", y); y += 48;
  s_heap = status_row(page, "HEAP", y); y += 48;
  s_version = status_row(page, "VERSION", y);
}

static void status_view_update(void) {
  const beacon_theme_t* t = theme_active();
  char uptime[16];
  char wifi[48];
  char heap[16];
  char battery[16];

  about_fmt_uptime(uptime_s(), uptime, sizeof(uptime));
  about_fmt_heap_kb(esp_get_free_heap_size(), heap, sizeof(heap));
  net_status_str(wifi, sizeof(wifi));

  int pct = power_battery_pct();
  bool charging = power_charging();
  if (pct >= 0) snprintf(battery, sizeof(battery), "%d%%%s", pct, charging ? " +" : "");
  else snprintf(battery, sizeof(battery), "%s", charging ? "USB" : "--");

  txt_set(s_state, net_is_up() ? "ONLINE" : "LOCAL");
  txt_color(s_state, net_is_up() ? t->up : t->accent);
  txt_set(s_uptime, uptime);
  txt_set(s_wifi, wifi);
  txt_set(s_battery, battery);
  txt_set(s_heap, heap);
  txt_set(s_version, FIRMWARE_VERSION);
  txt_color(s_wifi, net_is_up() ? t->up : t->ink_dim);
  txt_color(s_battery, charging ? t->accent : (pct >= 0 && pct <= 20 ? t->down : t->ink));
}

}

extern const screen_view_t status_editorial_view = { status_view_build, status_view_update };
extern const screen_view_t status_hud_view = { status_view_build, status_view_update };
extern const screen_view_t status_calm_view = { status_view_build, status_view_update };
extern const screen_view_t status_blueprint_view = { status_view_build, status_view_update };
extern const screen_view_t status_led_view = { status_view_build, status_view_update };
extern const screen_view_t status_oscilloscope_view = { status_view_build, status_view_update };
extern const screen_view_t status_analog_view = { status_view_build, status_view_update };

SCREEN_MODULE_SIMPLE(status, "STATUS", status_module);
/* USER CODE END status_screen */
