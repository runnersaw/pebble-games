#include <pebble.h>
#include "pebble-games.h"
#include "instructionHandler.h"
#include "settingsHandler.h"
#include "textHandler.h"
#include "food.h"
#include "tennis.h"
#include "blackjack.h"
#include "2048.h"
#if defined(PBL_COLOR)
  #include "chess.h"
  #include "decrypt.h"
  #include "cards.h"
  #include "solitaire.h"
#endif

#define NUM_INSTRUCTION_SECTIONS 1

#if defined(PBL_COLOR)
  #define NUM_INSTRUCTION_ITEMS_SETTINGS 3
  #define NUM_INSTRUCTION_ITEMS_NO_SETTINGS 2
#else
  #define NUM_INSTRUCTION_ITEMS_SETTINGS 3
  #define NUM_INSTRUCTION_ITEMS_NO_SETTINGS 2
#endif

#define GAME_INDEX 0
#define INSTRUCTION_INDEX 1
#define SETTINGS_INDEX 2

#define ICON_SIZE 28
#if defined(PBL_ROUND)
  #define CELL_HEIGHT 60
#else
  #define CELL_HEIGHT 45
#endif

static Window *s_instruction_menu_window;
static MenuLayer *s_instruction_menu;

static short instruction_type;
static short has_settings;
static GBitmap *game_icon;
static GBitmap *instruction_icon;
static GBitmap *settings_icon;

// A simple menu layer can have multiple sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_INSTRUCTION_SECTIONS;
}

// Each section is composed of a number of menu items
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  if (has_settings) {
    return NUM_INSTRUCTION_ITEMS_SETTINGS;
  } else { 
    return NUM_INSTRUCTION_ITEMS_NO_SETTINGS;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return 0;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  return CELL_HEIGHT;
}

static void draw_menu(GContext *ctx, const Layer *layer, char *title, GBitmap *bmp) {
  menu_cell_basic_draw(ctx, layer, title, NULL, bmp);
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->row == GAME_INDEX) {
    draw_menu(ctx, cell_layer, "Play", game_icon);
  } else if (cell_index->row == INSTRUCTION_INDEX) {
    draw_menu(ctx, cell_layer, "Instructions", NULL);
  } else if (cell_index->row == SETTINGS_INDEX) {
    draw_menu(ctx, cell_layer, "Settings", NULL);
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->row == GAME_INDEX) {
    switch (instruction_type) {
      case BLACKJACK:
        blackjack_init();
        break;
      case TWO048:
        two048_init();
        break;
      case FOOD:
        food_init();
        break;
      case TENNIS:
        tennis_init();
        break;
      #if defined(PBL_COLOR)
      case CHESS:
        chess_init();
        break;
      case DECRYPT:
        decrypt_init();
        break;
      case SOLITAIRE:
        solitaire_init();
        break;
      #endif
    }
  } else if (cell_index->row == INSTRUCTION_INDEX) {
    text_init(instruction_type);
  } else if (cell_index->row == SETTINGS_INDEX) {
    settings_init(instruction_type);
  }
}

static void load_bitmaps() {
  switch (instruction_type) {
    case BLACKJACK:
      game_icon = gbitmap_create_with_resource(RESOURCE_ID_BLACKJACK_ICON);
      break;
    case TWO048:
      game_icon = gbitmap_create_with_resource(RESOURCE_ID_TWO048_ICON);
      break;
    case FOOD:
      game_icon = gbitmap_create_with_resource(RESOURCE_ID_FOOD_ICON);
      break;
    case TENNIS:
      game_icon = gbitmap_create_with_resource(RESOURCE_ID_TENNIS_ICON);
      break;
    #if defined(PBL_COLOR)
    case CHESS:
      game_icon = gbitmap_create_with_resource(RESOURCE_ID_CHESS_ICON);
      break;
    case DECRYPT:
      game_icon = gbitmap_create_with_resource(RESOURCE_ID_DECRYPT_ICON);
      break;
    case SOLITAIRE:
      game_icon = gbitmap_create_with_resource(RESOURCE_ID_SOLITAIRE_ICON);
      break;
    #endif
  }

}

static void destroy_bitmaps() {
  gbitmap_destroy(game_icon);
}

static void instruction_menu_window_load(Window *window) {
  load_bitmaps();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_instruction_menu = menu_layer_create(bounds);
  #if defined(PBL_ROUND)
    menu_layer_set_center_focused((MenuLayer *)s_instruction_menu, true);
  #endif

  menu_layer_set_callbacks(s_instruction_menu, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_header = NULL,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  menu_layer_set_click_config_onto_window(s_instruction_menu, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_instruction_menu));
}

static void instruction_menu_window_unload(Window *window) {
  destroy_bitmaps();
  menu_layer_destroy(s_instruction_menu);
  window_destroy(window);
  s_instruction_menu_window = NULL;
}

static void setHasSettings(short game) {
  #ifdef CHESS_HAS_SETTINGS
  if (game == CHESS) {
    has_settings = 1;
    return;
  }
  #endif
  #ifdef BLACKJACK_HAS_SETTINGS
  if (game == BLACKJACK) {
    has_settings = 1;
    return;
  }
  #endif
  #ifdef DECRYPT_HAS_SETTINGS
  if (game == DECRYPT) {
    has_settings = 1;
    return;
  }
  #endif
  #ifdef TWO048_HAS_SETTINGS
  if (game == TWO048) {
    has_settings = 1;
    return;
  }
  #endif
  #ifdef FOOD_HAS_SETTINGS
  if (game == FOOD) {
    has_settings = 1;
    return;
  }
  #endif
  #ifdef TENNIS_HAS_SETTINGS
  if (game == TENNIS) {
    has_settings = 1;
    return;
  }
  #endif
  #ifdef SOLITAIRE_HAS_SETTINGS
  if (game == SOLITAIRE) {
    has_settings = 1;
    return;
  }
  #endif


  has_settings = 0;
}

void instruction_init(short game) {
  // Create main Window element and assign to pointer
  instruction_type = game;
  setHasSettings(game);
  s_instruction_menu_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_instruction_menu_window, (WindowHandlers) {
    .load = instruction_menu_window_load,
    .unload = instruction_menu_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_instruction_menu_window, true);
}