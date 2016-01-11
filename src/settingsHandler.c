#include <pebble.h>
#include "pebble-games.h"
#include "settingsRadioHandler.h"

#define NUM_SETTINGS_SECTIONS 1

#if defined(PBL_ROUND)
  #define CELL_HEIGHT 60
#else
  #define CELL_HEIGHT 45
#endif

#define NUM_SETTINGS_ITEMS 0

static short current_game;

static Window *s_settings_menu_window;
static MenuLayer *s_settings_menu;

// A simple menu layer can have multiple sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_SETTINGS_SECTIONS;
}

// Each section is composed of a number of menu items
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  #ifdef CHESS
  if (current_game == CHESS) {
    return CHESS_HAS_SETTINGS;
  }
  #endif
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

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  #ifdef CHESS
  if (current_game == CHESS) {
    if (cell_index->row == CHESS_DIFFICULTY) {
      menu_cell_basic_draw(ctx, cell_layer, "Difficulty", NULL, NULL);
    }
  }
  #endif
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  #ifdef CHESS
  if (current_game == CHESS) {
    if (cell_index->row == CHESS_DIFFICULTY) {
      settings_radio_init(CHESS_DIFFICULTY_SETTING);
    }
  }
  #endif
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
  window_destroy(window);
  s_settings_menu_window = NULL;
}

void settings_init(short game) {
  current_game = game;
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