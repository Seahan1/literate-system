#include "ui/chrome.h"
#include "ui/theme.h"
#include "config/layout.h"

typedef enum {
  CH_NONE,
  CH_GRID,
  CH_BLUEPRINT,
  CH_DOTS,
  CH_GRATICULE,
  CH_RAINBOW_DOTS,
  CH_PACMAN,
} chrome_kind_t;

// theme index -> chrome kind (catalog order: editorial,hud,dotmatrix,blueprint,led,oscilloscope,analog,pacman)
static chrome_kind_t kind_for(uint8_t idx) {
  switch (idx) {
    case 1: return CH_GRID;        // Aerospace HUD
    case 3: return CH_BLUEPRINT;   // Blueprint
    case 4: return CH_DOTS;        // LED matrix
    case 5: return CH_GRATICULE;   // Oscilloscope
    case 2: return CH_RAINBOW_DOTS; // Dot-Matrix
    case 7: return CH_PACMAN;      // Pac-Man
    default: return CH_NONE;       // editorial / calm / analog
  }
}

static void line(lv_draw_ctx_t* ctx, lv_draw_line_dsc_t* d, int x1, int y1, int x2, int y2) {
  lv_point_t a = {(lv_coord_t)x1, (lv_coord_t)y1}, b = {(lv_coord_t)x2, (lv_coord_t)y2};
  lv_draw_line(ctx, d, &a, &b);
}

static void grid(lv_draw_ctx_t* ctx, const lv_area_t* a, lv_color_t c, int sp, lv_opa_t opa) {
  lv_draw_line_dsc_t d; lv_draw_line_dsc_init(&d); d.color = c; d.width = 1; d.opa = opa;
  for (int x = a->x1; x <= a->x2; x += sp) line(ctx, &d, x, a->y1, x, a->y2);
  for (int y = a->y1; y <= a->y2; y += sp) line(ctx, &d, a->x1, y, a->x2, y);
}

static void pixel_block(lv_draw_ctx_t* ctx, int x, int y, int size, lv_color_t color, lv_opa_t opa) {
  lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d);
  d.bg_color = color; d.bg_opa = opa; d.radius = 0;
  lv_area_t block = {(lv_coord_t)x, (lv_coord_t)y, (lv_coord_t)(x + size - 1), (lv_coord_t)(y + size - 1)};
  lv_draw_rect(ctx, &d, &block);
}

static void pacman_sprite(lv_draw_ctx_t* ctx, int x, int y, lv_color_t body, lv_color_t eye) {
  static const char* const shape[5] = {"  XX ", " XXXX", "XXXXX", "XXXX ", " XX  "};
  const int cell = 5;
  for (int row = 0; row < 5; row++)
    for (int col = 0; col < 5; col++)
      if (shape[row][col] == 'X') pixel_block(ctx, x + col * cell, y + row * cell, cell, body, LV_OPA_COVER);
  pixel_block(ctx, x + 15, y + 5, 2, eye, LV_OPA_COVER);
}

static void ghost_sprite(lv_draw_ctx_t* ctx, int x, int y, lv_color_t body, lv_color_t eye) {
  static const char* const shape[5] = {" XXX ", "XXXXX", "XXXXX", "XXXXX", "X X X"};
  const int cell = 5;
  for (int row = 0; row < 5; row++)
    for (int col = 0; col < 5; col++)
      if (shape[row][col] == 'X') pixel_block(ctx, x + col * cell, y + row * cell, cell, body, LV_OPA_COVER);
  pixel_block(ctx, x + 7, y + 10, 3, eye, LV_OPA_COVER);
  pixel_block(ctx, x + 17, y + 10, 3, eye, LV_OPA_COVER);
  pixel_block(ctx, x + 8, y + 11, 1, lv_color_make(22, 22, 55), LV_OPA_COVER);
  pixel_block(ctx, x + 18, y + 11, 1, lv_color_make(22, 22, 55), LV_OPA_COVER);
}

static void pacman_maze(lv_draw_ctx_t* ctx, const lv_area_t* a, const beacon_theme_t* t) {
  const int x0 = a->x1 + 28, x1 = a->x2 - 28;
  const int y0 = a->y1 + 56, y1 = a->y2 - 42;
  const int cx = (x0 + x1) / 2;
  lv_draw_line_dsc_t maze; lv_draw_line_dsc_init(&maze);
  maze.color = t->accent2; maze.width = 2; maze.opa = LV_OPA_50;

  line(ctx, &maze, x0, y0, x1, y0);
  line(ctx, &maze, x0, y1, x1, y1);
  line(ctx, &maze, x0, y0, x0, y1);
  line(ctx, &maze, x1, y0, x1, y1);
  line(ctx, &maze, x0 + 20, y0 + 58, cx - 30, y0 + 58);
  line(ctx, &maze, cx + 30, y0 + 58, x1 - 20, y0 + 58);
  line(ctx, &maze, x0 + 20, y0 + 118, x0 + 120, y0 + 118);
  line(ctx, &maze, x1 - 120, y0 + 118, x1 - 20, y0 + 118);
  line(ctx, &maze, x0 + 20, y1 - 66, x0 + 120, y1 - 66);
  line(ctx, &maze, x1 - 120, y1 - 66, x1 - 20, y1 - 66);
  line(ctx, &maze, cx - 62, y0 + 118, cx - 62, y1 - 66);
  line(ctx, &maze, cx + 62, y0 + 118, cx + 62, y1 - 66);
  line(ctx, &maze, x0 + 88, y0, x0 + 88, y0 + 58);
  line(ctx, &maze, x1 - 88, y0, x1 - 88, y0 + 58);
  line(ctx, &maze, x0 + 88, y1 - 66, x0 + 88, y1);
  line(ctx, &maze, x1 - 88, y1 - 66, x1 - 88, y1);

  for (int x = x0 + 32; x < x1 - 20; x += 22)
    pixel_block(ctx, x, y0 + 84, 3, (x / 22) % 2 ? t->accent : t->alert, LV_OPA_80);
  for (int x = x0 + 32; x < x1 - 20; x += 22)
    pixel_block(ctx, x, y1 - 26, 3, (x / 22) % 2 ? t->accent2 : t->accent, LV_OPA_80);

  pacman_sprite(ctx, x0 + 12, y1 - 34, t->accent, t->ink);
  ghost_sprite(ctx, x1 - 116, y0 + 14, lv_color_make(255, 82, 112), t->ink);
  ghost_sprite(ctx, x1 - 86, y0 + 14, lv_color_make(80, 150, 255), t->ink);
  ghost_sprite(ctx, x1 - 56, y0 + 14, lv_color_make(110, 255, 82), t->ink);
}

static void draw_cb(lv_event_t* e) {
  lv_obj_t* bg = lv_event_get_target(e);
  lv_draw_ctx_t* ctx = lv_event_get_draw_ctx(e);
  const beacon_theme_t* t = theme_active(); if (!t) return;
  lv_area_t a; lv_obj_get_coords(bg, &a);
  int cx = (a.x1 + a.x2) / 2, cy = (a.y1 + a.y2) / 2;

  switch (kind_for(theme_index())) {
    case CH_GRID:
      grid(ctx, &a, t->line, 48, LV_OPA_COVER);
      break;

    case CH_BLUEPRINT: {
      grid(ctx, &a, t->line, 24, LV_OPA_50);
      lv_draw_line_dsc_t d; lv_draw_line_dsc_init(&d); d.color = t->accent; d.width = 1; d.opa = LV_OPA_70;
      int L = 16, in = SAFE_INSET - 12;
      line(ctx, &d, a.x1+in, a.y1+in, a.x1+in+L, a.y1+in); line(ctx, &d, a.x1+in, a.y1+in, a.x1+in, a.y1+in+L); // TL
      line(ctx, &d, a.x2-in, a.y1+in, a.x2-in-L, a.y1+in); line(ctx, &d, a.x2-in, a.y1+in, a.x2-in, a.y1+in+L); // TR
      line(ctx, &d, a.x1+in, a.y2-in, a.x1+in+L, a.y2-in); line(ctx, &d, a.x1+in, a.y2-in, a.x1+in, a.y2-in-L); // BL
      line(ctx, &d, a.x2-in, a.y2-in, a.x2-in-L, a.y2-in); line(ctx, &d, a.x2-in, a.y2-in, a.x2-in, a.y2-in-L); // BR
      int ch = 130;
      line(ctx, &d, cx-ch, cy, cx+ch, cy); line(ctx, &d, cx, cy-ch, cx, cy+ch);   // crosshair
      break;
    }

    case CH_DOTS: {
      lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d); d.bg_color = t->accent; d.bg_opa = LV_OPA_30; d.radius = LV_RADIUS_CIRCLE;
      const lv_area_t* clip = ctx->clip_area;
      for (int y = a.y1; y <= a.y2; y += 13)
        for (int x = a.x1; x <= a.x2; x += 13) {
          lv_area_t dot = {(lv_coord_t)x, (lv_coord_t)y, (lv_coord_t)(x+1), (lv_coord_t)(y+1)};
          if (dot.x2 < clip->x1 || dot.x1 > clip->x2 || dot.y2 < clip->y1 || dot.y1 > clip->y2) continue;
          lv_draw_rect(ctx, &d, &dot);
        }
      break;
    }

    case CH_RAINBOW_DOTS: {
      const lv_area_t* clip = ctx->clip_area;
      int width = a.x2 - a.x1 + 1;
      lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d); d.radius = LV_RADIUS_CIRCLE;
      for (int y = a.y1; y <= a.y2; y += 13) {
        for (int x = a.x1; x <= a.x2; x += 13) {
          lv_area_t dot = {(lv_coord_t)x, (lv_coord_t)y, (lv_coord_t)(x + 1), (lv_coord_t)(y + 1)};
          if (dot.x2 < clip->x1 || dot.x1 > clip->x2 || dot.y2 < clip->y1 || dot.y1 > clip->y2) continue;
          uint16_t hue = (uint16_t)(((x - a.x1) * 360 + (y - a.y1) * 24) / (width > 0 ? width : 1)) % 360;
          d.bg_color = lv_color_hsv_to_rgb(hue, 220, 255);
          d.bg_opa = (lv_opa_t)(LV_OPA_20 + ((y - a.y1) % 39));
          lv_draw_rect(ctx, &d, &dot);
        }
      }
      break;
    }

    case CH_GRATICULE:
      grid(ctx, &a, t->line, 42, LV_OPA_50);   // graticule only; no bright center axis
      break;

    case CH_PACMAN:
      pacman_maze(ctx, &a, t);
      break;

    default: break;
  }
}

lv_obj_t* chrome_attach(lv_obj_t* page) {
  lv_obj_t* bg = lv_obj_create(page);
  lv_obj_remove_style_all(bg);
  lv_obj_set_size(bg, SCREEN_W, SCREEN_H);
  lv_obj_center(bg);
  lv_obj_clear_flag(bg, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(bg, draw_cb, LV_EVENT_DRAW_MAIN, NULL);
  return bg;
}
