#include <pebble.h>
#include "pebble-games.h"

#define NUM_SETTINGS_SECTIONS 1

#define CHESS_INDEX 0
#define BLACKJACK_INDEX 1
#define TWO048_INDEX 2
#if defined(PBL_COLOR)
  #define DECRYPT_INDEX 3
  #define SOLITAIRE_INDEX 4
  #define FOOD_INDEX 5
  #define TENNIS_INDEX 6
#else
  #define FOOD_INDEX 3
  #define TENNIS_INDEX 4
#endif

#if defined(PBL_COLOR)
  #define NUM_SETTINGS_ITEMS 7
#else
  #define NUM_SETTINGS_ITEMS 5
#endif

#define ICON_SIZE 28
#if defined(PBL_ROUND)
  #define HEIGHT 500
  #define CELL_HEIGHT 60
#else
  #define HEIGHT 350
  #define CELL_HEIGHT 45
#endif

static Window *s_settings_menu_window;
static Window *s_settings_window;
static ScrollLayer *s_settings_scroll_layer;
static TextLayer * s_settings_text_layer;
static MenuLayer *s_settings_menu;
static char *settings_text_ptr;

// A simple menu layer can have multiple sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_SETTINGS_SECTIONS;
}

// Each section is composed of a number of menu items
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return NUM_SETTINGS_ITEMS;
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
    case CHESS_INDEX:
      draw_menu(ctx, cell_layer, "Chess", NULL);
      break;
    case BLACKJACK_INDEX:
      draw_menu(ctx, cell_layer, "Blackjack", NULL);
      break;
    case TWO048_INDEX:
      draw_menu(ctx, cell_layer, "2048", NULL);
      break;
    case FOOD_INDEX:
      draw_menu(ctx, cell_layer, "FOOD!", NULL);
      break;
    case TENNIS_INDEX:
      draw_menu(ctx, cell_layer, "Tennis", NULL);
      break;
    #if defined(PBL_COLOR)
    case DECRYPT_INDEX:
      draw_menu(ctx, cell_layer, "Decrypt", NULL);
      break;
    case SOLITAIRE_INDEX:
      draw_menu(ctx, cell_layer, "Solitaire", NULL);
      break;
    #endif
  }
}

void settings_window_load(Window *window) {
  settings_text_ptr = malloc(300*sizeof(char));
  ResHandle rh = resource_get_handle(RESOURCE_ID_PEBB_GAMES_ABOUT);
  resource_load(rh, (uint8_t *)settings_text_ptr, 300*sizeof(char));

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);    
  s_settings_scroll_layer = scroll_layer_create(bounds);

  GRect text_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, HEIGHT);
  s_settings_text_layer = text_layer_create(text_bounds);
  
  scroll_layer_add_child(s_settings_scroll_layer, text_layer_get_layer(s_settings_text_layer));
  scroll_layer_set_content_size(s_settings_scroll_layer, GSize(100,HEIGHT));
  scroll_layer_set_click_config_onto_window(s_settings_scroll_layer, s_settings_window);
  text_layer_set_text(s_settings_text_layer, settings_text_ptr);
  text_layer_set_text_alignment(s_settings_text_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_settings_window), scroll_layer_get_layer(s_settings_scroll_layer));

  #if defined(PBL_ROUND)
    text_layer_set_overflow_mode(s_settings_text_layer, GTextOverflowModeWordWrap);
    text_layer_enable_screen_text_flow_and_paging(s_settings_text_layer, 15);
    scroll_layer_set_paging(s_settings_scroll_layer, true);
  #endif
}

void settings_window_unload(Window *window) {
  free(settings_text_ptr);
  text_layer_destroy(s_settings_text_layer);
  scroll_layer_destroy(s_settings_scroll_layer);
  window_destroy(s_settings_window);
}

void settings_chosen() {
  // Create main Window element and assign to pointer
  s_settings_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_settings_window, (WindowHandlers) {
    .load = settings_window_load,
    .unload = settings_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_settings_window, true);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case CHESS_INDEX:
      break;
    case BLACKJACK_INDEX:
      break;
    case TWO048_INDEX:
      break;
    case FOOD_INDEX:
      break;
    case TENNIS_INDEX:
      break;
    #if defined(PBL_COLOR)
    case DECRYPT_INDEX:
      break;
    case SOLITAIRE_INDEX:
      break;
    #endif
  }
}

static void settings_menu_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_settings_menu = menu_layer_create(bounds);
  #if defined(PBL_ROUND)
    menu_layer_set_center_focused((MenuLayer *)s_settings_menu, true);
  #endif
  menu_layer_set_callbacks(s_settings_menu, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  menu_layer_set_click_config_onto_window(s_settings_menu, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_settings_menu));
}

static void settings_menu_window_unload(Window *window) {
  menu_layer_destroy(s_settings_menu);
}

void settings_init() {
  // Create main Window element and assign to pointer
  s_settings_menu_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_settings_menu_window, (WindowHandlers) {
    .load = settings_menu_window_load,
    .unload = settings_menu_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_settings_menu_window, true);
}

void settings_deinit() {
    // Destroy Window
    window_destroy(s_settings_menu_window);
}