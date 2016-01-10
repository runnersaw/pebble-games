#include <pebble.h>
#include "pebble-games.h"
#include "instructionHandler.h"
#include "food.h"
#include "tennis.h"
#include "chess.h"
#include "blackjack.h"
#include "2048.h"
#if defined(PBL_COLOR)
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
  #define HEIGHT 500
  #define CELL_HEIGHT 60
#else
  #define HEIGHT 350
  #define CELL_HEIGHT 45
#endif

static Window *s_instruction_menu_window;
static Window *s_instruction_window;
static ScrollLayer *s_instruction_scroll_layer;
static TextLayer * s_instruction_text_layer;
static MenuLayer *s_instruction_menu;
static char *instruction_text_ptr;

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
  if (cell_index->row == GAME_INDEX) {
      draw_menu(ctx, cell_layer, "Play", game_icon);
  } else if (cell_index->row == INSTRUCTION_INDEX) {
    draw_menu(ctx, cell_layer, "Instructions", NULL);
  } else if (cell_index->row == SETTINGS_INDEX) {
      draw_menu(ctx, cell_layer, "Settings", NULL);
  }
}

static short get_instruction_size(short game) {
  return 100;
  /*
  switch (instruction_type) {
    case CHESS:
      return 100;
      break;
    case BLACKJACK:
      return 100;
      break;
    case TWO048:
      return 100;
      break;
    case FOOD:
      return 100;
      break;
    case TENNIS:
      return 100;
      break;
    #if defined(PBL_COLOR)
    case DECRYPT:
      return 100;
      break;
    case SOLITAIRE:
      return 100;
      break;
    #endif
  }*/
}

static ResHandle get_resource_handle(short game) {
  return resource_get_handle(RESOURCE_ID_PEBB_GAMES_ABOUT);
  /*switch (instruction_type) {
    case CHESS:
      return 100;
      break;
    case BLACKJACK:
      return 100;
      break;
    case TWO048:
      return 100;
      break;
    case FOOD:
      return 100;
      break;
    case TENNIS:
      return 100;
      break;
    #if defined(PBL_COLOR)
    case DECRYPT:
      return 100;
      break;
    case SOLITAIRE:
      return 100;
      break;
    #endif
  }*/
}

void instruction_window_load(Window *window) {
  short size = get_instruction_size(instruction_type);
  ResHandle rh = get_resource_handle(instruction_type);

  instruction_text_ptr = malloc(size*sizeof(char));
  resource_load(rh, (uint8_t *)instruction_text_ptr, size*sizeof(char));

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);    
  s_instruction_scroll_layer = scroll_layer_create(bounds);

  GRect text_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, HEIGHT);
  s_instruction_text_layer = text_layer_create(text_bounds);
  
  scroll_layer_add_child(s_instruction_scroll_layer, text_layer_get_layer(s_instruction_text_layer));
  scroll_layer_set_content_size(s_instruction_scroll_layer, GSize(100,HEIGHT));
  scroll_layer_set_click_config_onto_window(s_instruction_scroll_layer, s_instruction_window);
  text_layer_set_text(s_instruction_text_layer, instruction_text_ptr);
  text_layer_set_text_alignment(s_instruction_text_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_instruction_window), scroll_layer_get_layer(s_instruction_scroll_layer));

  #if defined(PBL_ROUND)
    text_layer_set_overflow_mode(s_instruction_text_layer, GTextOverflowModeWordWrap);
    text_layer_enable_screen_text_flow_and_paging(s_instruction_text_layer, 15);
    scroll_layer_set_paging(s_instruction_scroll_layer, true);
  #endif
}

void instruction_window_unload(Window *window) {
  free(instruction_text_ptr);
  text_layer_destroy(s_instruction_text_layer);
  scroll_layer_destroy(s_instruction_scroll_layer);
  window_destroy(s_instruction_window);
}

void instruction_chosen() {
  // Create main Window element and assign to pointer
  s_instruction_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_instruction_window, (WindowHandlers) {
    .load = instruction_window_load,
    .unload = instruction_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_instruction_window, true);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->row == GAME_INDEX) {
    switch (instruction_type) {
      case CHESS:
        chess_init();
        break;
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
      case DECRYPT:
        decrypt_init();
        break;
      case SOLITAIRE:
        solitaire_init();
        break;
      #endif
    }
  } else if (cell_index->row == INSTRUCTION_INDEX) {
    APP_LOG(APP_LOG_LEVEL_INFO, "instructions selected %d", instruction_type);
  } else if (cell_index->row == SETTINGS_INDEX) {
    APP_LOG(APP_LOG_LEVEL_INFO, "instructions selected %d", instruction_type);
  }
}

static void load_bitmaps() {
  switch (instruction_type) {
    case CHESS:
      game_icon = gbitmap_create_with_resource(RESOURCE_ID_CHESS_ICON);
      break;
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
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  menu_layer_set_click_config_onto_window(s_instruction_menu, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_instruction_menu));
}

static void instruction_menu_window_unload(Window *window) {
  destroy_bitmaps();
  menu_layer_destroy(s_instruction_menu);
}

static void setHasSettings(short game) {
  if (game==CHESS) {
    has_settings = 1;
  } else {
    has_settings = 0;
  }
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

void instruction_deinit() {
    // Destroy Window
    window_destroy(s_instruction_menu_window);
}