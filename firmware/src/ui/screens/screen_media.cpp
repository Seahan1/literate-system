/* USER CODE BEGIN media_screen */
#include "ui/screen.h"
#include "ui/theme.h"
#include "ui/styles.h"
#include "ui/state_view.h"
#include "ui/screens/screen_common.h"
#include "core/datastore.h"
#include "config/layout.h"
#include <Arduino.h>
#include <string.h>

namespace {

static lv_obj_t* s_status = nullptr;
static lv_obj_t* s_title = nullptr;
static lv_obj_t* s_artist = nullptr;
static lv_obj_t* s_album = nullptr;
static lv_obj_t* s_title_caption = nullptr;
static lv_obj_t* s_artist_caption = nullptr;
static lv_obj_t* s_album_caption = nullptr;
static lv_obj_t* s_progress = nullptr;
static lv_obj_t* s_elapsed = nullptr;
static lv_obj_t* s_duration = nullptr;

static lv_obj_t* make_caption(lv_obj_t* page, const char* text, int y) {
  lv_obj_t* label = lv_label_create(page);
  lv_obj_add_style(label, &S.eyebrow, 0);
  lv_label_set_text(label, text);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, SAFE_INSET, y);
  return label;
}

static lv_obj_t* make_value(lv_obj_t* page, int y, int width) {
  lv_obj_t* label = lv_label_create(page);
  lv_obj_add_style(label, &S.display, 0);
  lv_obj_set_width(label, width);
  lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
  lv_label_set_text(label, "--");
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, SAFE_INSET, y);
  return label;
}

static void clock_text(char* out, size_t cap, uint32_t seconds) {
  snprintf(out, cap, "%u:%02u", (unsigned)(seconds / 60), (unsigned)(seconds % 60));
}

static void build(lv_obj_t* page) {
  const beacon_theme_t* theme = theme_active();
  s_status = build_header(page, "NOW PLAYING");
  s_title_caption = make_caption(page, "TITLE", SAFE_INSET + 42);
  s_title = make_value(page, SAFE_INSET + 62, SCREEN_W - 2 * SAFE_INSET);
  s_artist_caption = make_caption(page, "ARTIST", SAFE_INSET + 154);
  s_artist = make_value(page, SAFE_INSET + 174, SCREEN_W - 2 * SAFE_INSET);
  s_album_caption = make_caption(page, "ALBUM", SAFE_INSET + 254);
  s_album = make_value(page, SAFE_INSET + 274, SCREEN_W - 2 * SAFE_INSET);
  s_progress = lv_bar_create(page);
  lv_obj_set_size(s_progress, SCREEN_W - 2 * SAFE_INSET, 10);
  lv_obj_align(s_progress, LV_ALIGN_TOP_LEFT, SAFE_INSET, SAFE_INSET + 354);
  lv_bar_set_range(s_progress, 0, 1000);
  lv_bar_set_value(s_progress, 0, LV_ANIM_OFF);
  bg_opa_if(s_progress, LV_OPA_COVER, LV_PART_MAIN);
  bg_opa_if(s_progress, LV_OPA_COVER, LV_PART_INDICATOR);
  bg_color_if(s_progress, theme->line, LV_PART_MAIN);
  bg_color_if(s_progress, theme->accent, LV_PART_INDICATOR);
  s_elapsed = lv_label_create(page);
  lv_obj_add_style(s_elapsed, &S.slot, 0);
  lv_label_set_text(s_elapsed, "0:00");
  lv_obj_align(s_elapsed, LV_ALIGN_TOP_LEFT, SAFE_INSET, SAFE_INSET + 370);
  s_duration = lv_label_create(page);
  lv_obj_add_style(s_duration, &S.slot, 0);
  lv_label_set_text(s_duration, "0:00");
  lv_obj_set_width(s_duration, SCREEN_W - 2 * SAFE_INSET);
  lv_obj_set_style_text_align(s_duration, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_align(s_duration, LV_ALIGN_TOP_LEFT, SAFE_INSET, SAFE_INSET + 370);
  txt_color(s_title_caption, theme->accent);
  txt_color(s_artist_caption, theme->accent2);
  txt_color(s_album_caption, theme->accent2);
}

static void update(void) {
  media_rec_t media = ds_get_media();
  const beacon_theme_t* theme = theme_active();
  uint32_t now = now_s();
  bool placeholder = sv_placeholder(media.hdr.state);
  bool dim = sv_dim(media.hdr.state);
  char status[24];
  bool nonlive = sv_status(status, sizeof(status), &media.hdr, now);

  if (nonlive) {
    txt_set(s_status, status);
    txt_color(s_status, sv_severe(media.hdr.state) ? theme->down : theme->ink_dim);
  } else {
    txt_set(s_status, media.title[0] ? (media.playing ? "PLAYING" : "PAUSED") : "NOT PLAYING");
    txt_color(s_status, media.title[0] ? (media.playing ? theme->up : theme->accent) : theme->ink_dim);
  }

  txt_set(s_title, placeholder || !media.title[0] ? "--" : media.title);
  txt_set(s_artist, placeholder || !media.artist[0] ? "--" : media.artist);
  txt_set(s_album, placeholder || !media.album[0] ? "--" : media.album);
  value_state(s_title, media.hdr.state);
  value_state(s_artist, media.hdr.state);
  value_state(s_album, media.hdr.state);

  uint32_t duration = media.duration_s;
  uint32_t position = media.position_s > duration && duration ? duration : media.position_s;
  bool has_progress = !placeholder && duration > 0;
  hidden_set(s_progress, !has_progress);
  hidden_set(s_elapsed, !has_progress);
  hidden_set(s_duration, !has_progress);
  if (has_progress) {
    uint32_t level = (position * 1000u) / duration;
    bar_set_if(s_progress, (int32_t)level);
    char elapsed[12];
    char total[12];
    clock_text(elapsed, sizeof(elapsed), position);
    clock_text(total, sizeof(total), duration);
    txt_set(s_elapsed, elapsed);
    txt_set(s_duration, total);
    txt_color(s_elapsed, theme->ink_dim);
    txt_color(s_duration, theme->ink_dim);
    bg_color_if(s_progress, dim ? theme->line : theme->accent, LV_PART_INDICATOR);
  }
}

}

extern const screen_view_t media_editorial_view = { build, update };
extern const screen_view_t media_hud_view = { build, update };
extern const screen_view_t media_calm_view = { build, update };
extern const screen_view_t media_blueprint_view = { build, update };
extern const screen_view_t media_led_view = { build, update };
extern const screen_view_t media_oscilloscope_view = { build, update };
extern const screen_view_t media_analog_view = { build, update };

static const screen_view_t* s_views[] = {
  &media_editorial_view, &media_hud_view, &media_calm_view, &media_blueprint_view,
  &media_led_view, &media_oscilloscope_view, &media_analog_view, &media_calm_view,
};

static lv_obj_t* module_build(lv_obj_t* page) {
  s_views[theme_index()]->build(page);
  return page;
}

static void module_update(void) {
  s_views[theme_index()]->update();
}

extern const screen_module_t media_module = { "NOW", module_build, module_update };
/* USER CODE END media_screen */
