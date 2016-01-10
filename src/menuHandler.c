#include <pebble.h>
#include "pebble-games.h"
#include "menuHandler.h"
#include "instructionHandler.h"
#include "textHandler.h"

#define NUM_MENU_SECTIONS 1

#if defined(PBL_COLOR)
  #define NUM_MENU_ICONS 8
  #define NUM_MENU_ITEMS 8
#else
  #define NUM_MENU_ICONS 5
  #define NUM_MENU_ITEMS 5
#endif
#define CHAR_NUM 350

#define ICON_SIZE 28
#if defined(PBL_ROUND)
  #define HEIGHT 500
  #define CELL_HEIGHT 60
#else
  #define HEIGHT 350
  #define CELL_HEIGHT 45
#endif

static Window *s_menu_window;
static Window *s_about_window;
static ScrollLayer *s_about_scroll_layer;
static TextLayer * s_about_text_layer;
static MenuLayer *s_games_menu;
static char *about_text_ptr;

static GBitmap *info_icon;
static GBitmap *tennis_icon;
static GBitmap *food_icon;
static GBitmap *blackjack_icon;
static GBitmap *two048_icon;
#if defined(PBL_COLOR)
  static GBitmap *chess_icon;
  static GBitmap *solitaire_icon;
  static GBitmap *decrypt_icon;
  static GBitmap *instruction_icon;
#endif

// A simple menu layer can have multiple sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

// Each section is composed of a number of menu items
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return NUM_MENU_ITEMS;
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return 0;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  return CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  //menu_cell_basic_header_draw(ctx, cell_layer, "Games");
}

static void draw_menu(GContext *ctx, const Layer *layer, char *title, GBitmap *bmp) {
  /*
  GRect frame = layer_get_frame(layer);

  int font_size = 40;
  int margin = (frame.size.h - ICON_SIZE) / 2;
  int topMargin = (frame.size.h - font_size) / 2;
  int textx = 2*margin+ICON_SIZE;
  int textw = frame.size.w - 2*margin - ICON_SIZE;
  #if defined(PBL_COLOR)
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
  #else
    graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  if (bmp != NULL) {
    graphics_draw_bitmap_in_rect(ctx, bmp, GRect(margin, margin, ICON_SIZE, ICON_SIZE));
  }
  graphics_draw_text(ctx, title, fonts_get_system_font(FONT_KEY_GOTHIC_28), GRect(textx, topMargin, textw, font_size), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  */
  menu_cell_basic_draw(ctx, layer, title, NULL, bmp);
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case BLACKJACK:
      draw_menu(ctx, cell_layer, "Blackjack", blackjack_icon);
      break;
    case TWO048:
      draw_menu(ctx, cell_layer, "2048", two048_icon);
      break;
    case FOOD:
      draw_menu(ctx, cell_layer, "FOOD!", food_icon);
      break;
    case TENNIS:
      draw_menu(ctx, cell_layer, "Tennis", tennis_icon);
      break;
    case ABOUT:
      draw_menu(ctx, cell_layer, "About", info_icon);
      break;
    #if defined(PBL_COLOR)
    case CHESS:
      draw_menu(ctx, cell_layer, "Chess", chess_icon);
      break;
    case DECRYPT:
      draw_menu(ctx, cell_layer, "Decrypt", decrypt_icon);
      break;
    case SOLITAIRE:
      draw_menu(ctx, cell_layer, "Solitaire", solitaire_icon);
      break;
    #endif
  }
}

static void load_bitmaps() {
  tennis_icon = gbitmap_create_with_resource(RESOURCE_ID_TENNIS_ICON);
  food_icon = gbitmap_create_with_resource(RESOURCE_ID_FOOD_ICON);
  info_icon = gbitmap_create_with_resource(RESOURCE_ID_INFO_ICON);
  blackjack_icon = gbitmap_create_with_resource(RESOURCE_ID_BLACKJACK_ICON);
  two048_icon = gbitmap_create_with_resource(RESOURCE_ID_TWO048_ICON);
  #if defined(PBL_COLOR)
    chess_icon = gbitmap_create_with_resource(RESOURCE_ID_CHESS_ICON);
    decrypt_icon = gbitmap_create_with_resource(RESOURCE_ID_DECRYPT_ICON);
    solitaire_icon = gbitmap_create_with_resource(RESOURCE_ID_SOLITAIRE_ICON);
  #endif
}

static void destroy_bitmaps() {
  gbitmap_destroy(info_icon);
  gbitmap_destroy(food_icon);
  gbitmap_destroy(tennis_icon);
  gbitmap_destroy(blackjack_icon);
  gbitmap_destroy(two048_icon);
  #if defined(PBL_COLOR)
    gbitmap_destroy(chess_icon);
    gbitmap_destroy(decrypt_icon);
    gbitmap_destroy(solitaire_icon);
  #endif
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case ABOUT:
      text_init(ABOUT);
      break;
    default:
      instruction_init(cell_index->row);
      break;
  }
}

static void menu_window_load(Window *window) {
  load_bitmaps();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_games_menu = menu_layer_create(bounds);
  #if defined(PBL_ROUND)
    menu_layer_set_center_focused((MenuLayer *)s_games_menu, true);
  #endif
  menu_layer_set_callbacks(s_games_menu, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  menu_layer_set_click_config_onto_window(s_games_menu, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_games_menu));
}

static void menu_window_unload(Window *window) {
  destroy_bitmaps();
  menu_layer_destroy(s_games_menu);
}

void menu_init() {
  // Create main Window element and assign to pointer
  s_menu_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_menu_window, (WindowHandlers) {
    .load = menu_window_load,
    .unload = menu_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_menu_window, true);
}

void menu_deinit() {
    // Destroy Window
    window_destroy(s_menu_window);
}

int main() {
  menu_init();
  app_event_loop();
  menu_deinit();
}
