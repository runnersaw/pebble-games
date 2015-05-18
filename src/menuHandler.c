#include <pebble.h>
#include "menuHandler.h"
#include "food.h"
#include "tennis.h"
#include "chess.h"
#include "blackjack.h"
#include "2048.h"
#include "decrypt.h"

#define NUM_MENU_SECTIONS 1

#define CHESS_INDEX 0
#define BLACKJACK_INDEX 1
#define TWO048_INDEX 2
#ifdef PBL_PLATFORM_BASALT
  #define DECRYPT_INDEX 3
  #define FOOD_INDEX 4
  #define TENNIS_INDEX 5
  #define ABOUT_INDEX 6
#else
  #define FOOD_INDEX 3
  #define TENNIS_INDEX 4
  #define ABOUT_INDEX 5
#endif

#ifdef PBL_PLATFORM_BASALT
  #define NUM_MENU_ICONS 7
  #define NUM_MENU_ITEMS 7
#else
  #define NUM_MENU_ICONS 6
  #define NUM_MENU_ITEMS 6
#endif
#define CHAR_NUM 350
#define HEIGHT 350

#define ICON_SIZE 28

static Window *s_menu_window;
static Window *s_about_window;
static ScrollLayer *s_about_scroll_layer;
static TextLayer * s_about_text_layer;
static MenuLayer *s_games_menu;
static char *about_text_ptr;

static GBitmap *info_icon;
static GBitmap *chess_icon;
static GBitmap *tennis_icon;
static GBitmap *food_icon;
static GBitmap *blackjack_icon;
static GBitmap *two048_icon;
#ifdef PBL_PLATFORM_BASALT
  static GBitmap *decrypt_icon;
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
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  menu_cell_basic_header_draw(ctx, cell_layer, "Games");
}

static void draw_menu(GContext *ctx, const Layer *layer, char *title, GBitmap *bmp) {
  GRect frame = layer_get_frame(layer);

  int font_size = 40;
  int margin = (frame.size.h - ICON_SIZE) / 2;
  int topMargin = (frame.size.h - font_size) / 2;
  int textx = 2*margin+ICON_SIZE;
  int textw = frame.size.w - 2*margin - ICON_SIZE;
  #ifdef PBL_PLATFORM_BASALT
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
  #else
    graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  graphics_draw_bitmap_in_rect(ctx, bmp, GRect(margin, margin, ICON_SIZE, ICON_SIZE));
  graphics_draw_text(ctx, title, fonts_get_system_font(FONT_KEY_GOTHIC_28), GRect(textx, topMargin, textw, font_size), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case CHESS_INDEX:
      draw_menu(ctx, cell_layer, "Chess", chess_icon);
      break;
    case BLACKJACK_INDEX:
      draw_menu(ctx, cell_layer, "Blackjack", blackjack_icon);
      break;
    case TWO048_INDEX:
      draw_menu(ctx, cell_layer, "2048", two048_icon);
      break;
    case FOOD_INDEX:
      draw_menu(ctx, cell_layer, "FOOD!", food_icon);
      break;
    case TENNIS_INDEX:
      draw_menu(ctx, cell_layer, "Tennis", tennis_icon);
      break;
    case ABOUT_INDEX:
      draw_menu(ctx, cell_layer, "About", info_icon);
      break;
    #ifdef PBL_PLATFORM_BASALT
    case DECRYPT_INDEX:
      draw_menu(ctx, cell_layer, "Decrypt", decrypt_icon);
      break;
    #endif
  }
}

void about_window_load() {
  about_text_ptr = malloc(CHAR_NUM*sizeof(char));
  ResHandle rh = resource_get_handle(RESOURCE_ID_PEBB_GAMES_ABOUT);
  resource_load(rh, (uint8_t *)about_text_ptr, CHAR_NUM*sizeof(char));
  s_about_scroll_layer = scroll_layer_create(GRect(0,0,144,168));
  s_about_text_layer = text_layer_create(GRect(0,0,144,HEIGHT));
  scroll_layer_add_child(s_about_scroll_layer, text_layer_get_layer(s_about_text_layer));
  scroll_layer_set_content_size(s_about_scroll_layer, GSize(100,HEIGHT));
  scroll_layer_set_click_config_onto_window(s_about_scroll_layer, s_about_window);
  text_layer_set_text(s_about_text_layer, about_text_ptr);
  text_layer_set_text_alignment(s_about_text_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_about_window), scroll_layer_get_layer(s_about_scroll_layer));
}

void about_window_unload() {
  free(about_text_ptr);
  text_layer_destroy(s_about_text_layer);
  scroll_layer_destroy(s_about_scroll_layer);
  window_destroy(s_about_window);
}

void about_chosen() {
  // Create main Window element and assign to pointer
  s_about_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_about_window, (WindowHandlers) {
    .load = about_window_load,
    .unload = about_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_about_window, true);
}

void load_bitmaps() {
  tennis_icon = gbitmap_create_with_resource(RESOURCE_ID_TENNIS_ICON);
  food_icon = gbitmap_create_with_resource(RESOURCE_ID_FOOD_ICON);
  chess_icon = gbitmap_create_with_resource(RESOURCE_ID_CHESS_ICON);
  info_icon = gbitmap_create_with_resource(RESOURCE_ID_INFO_ICON);
  blackjack_icon = gbitmap_create_with_resource(RESOURCE_ID_BLACKJACK_ICON);
  two048_icon = gbitmap_create_with_resource(RESOURCE_ID_TWO048_ICON);
  #ifdef PBL_PLATFORM_BASALT
    decrypt_icon = gbitmap_create_with_resource(RESOURCE_ID_DECRYPT_ICON);
  #endif
}

static void destroy_bitmaps() {
  gbitmap_destroy(info_icon);
  gbitmap_destroy(chess_icon);
  gbitmap_destroy(food_icon);
  gbitmap_destroy(tennis_icon);
  gbitmap_destroy(blackjack_icon);
  gbitmap_destroy(two048_icon);
  #ifdef PBL_PLATFORM_BASALT
    gbitmap_destroy(decrypt_icon);
  #endif
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case CHESS_INDEX:
      chess_init();
      break;
    case BLACKJACK_INDEX:
      blackjack_init();
      break;
    case TWO048_INDEX:
      two048_init();
      break;
    case FOOD_INDEX:
      food_init();
      break;
    case TENNIS_INDEX:
      tennis_init();
      break;
    case ABOUT_INDEX:
      about_chosen();
      break;
    #ifdef PBL_PLATFORM_BASALT
    case DECRYPT_INDEX:
      decrypt_init();
      break;
    #endif
  }
}

static void menu_window_load(Window *window) {
  load_bitmaps();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_games_menu = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_games_menu, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
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
