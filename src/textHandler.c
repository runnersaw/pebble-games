#include <pebble.h>
#include "pebble-games.h"
#include "textHandler.h"

#if defined(PBL_ROUND)
  #define SCROLL_WIDTH 100
#else
  #define SCROLL_WIDTH 100
#endif

#define TWO048_HEIGHT 168
#define FOOD_HEIGHT 168
#define TENNIS_HEIGHT 100
#define BLACKJACK_HEIGHT 150
#if defined(PBL_COLOR)
  #define CHESS_HEIGHT 300
  #define DECRYPT_HEIGHT 260
  #define SOLITAIRE_HEIGHT 168
#endif

#if defined(PBL_ROUND)
  #define ABOUT_HEIGHT 500
#else
  #define ABOUT_HEIGHT 350
#endif

#define MAX_CHARS 350

static ScrollLayer *s_instruction_scroll_layer;
static TextLayer * s_instruction_text_layer;
static Window *s_instruction_window;

static char *instruction_text_ptr;
static short instruction_type;

static short get_instruction_size(short game) {
  return MAX_CHARS;
}

static short get_instruction_height(short game) {
  switch (instruction_type) {
    case BLACKJACK:
      return BLACKJACK_HEIGHT;
      break;
    case TWO048:
      return TWO048_HEIGHT;
      break;
    case FOOD:
      return FOOD_HEIGHT;
      break;
    case TENNIS:
      return TENNIS_HEIGHT;
      break;
    case ABOUT:
      return ABOUT_HEIGHT;
      break;
    #if defined(PBL_COLOR)
    case CHESS:
      return CHESS_HEIGHT;
      break;
    case DECRYPT:
      return DECRYPT_HEIGHT;
      break;
    case SOLITAIRE:
      return SOLITAIRE_HEIGHT;
      break;
    #endif
  }
  return ABOUT_HEIGHT;
}

static ResHandle get_resource_handle(short game) {
  switch (instruction_type) {
    case BLACKJACK:
      return resource_get_handle(RESOURCE_ID_BLACKJACK_INSTRUCTIONS);
      break;
    case TWO048:
      return resource_get_handle(RESOURCE_ID_TWO048_INSTRUCTIONS);
      break;
    case FOOD:
      return resource_get_handle(RESOURCE_ID_FOOD_INSTRUCTIONS);
      break;
    case TENNIS:
      return resource_get_handle(RESOURCE_ID_TENNIS_INSTRUCTIONS);
      break;
    case ABOUT:
      return resource_get_handle(RESOURCE_ID_PEBB_GAMES_ABOUT);
      break;
    #if defined(PBL_COLOR)
    case CHESS:
      return resource_get_handle(RESOURCE_ID_CHESS_INSTRUCTIONS);
      break;
    case DECRYPT:
      return resource_get_handle(RESOURCE_ID_DECRYPT_INSTRUCTIONS);
      break;
    case SOLITAIRE:
      return resource_get_handle(RESOURCE_ID_SOLITAIRE_INSTRUCTIONS);
      break;
    #endif
  }
  return resource_get_handle(RESOURCE_ID_PEBB_GAMES_ABOUT);
}

void instruction_window_load(Window *window) {
  // set size variables for instructions
  short size = get_instruction_size(instruction_type);
  short height = get_instruction_height(instruction_type);
  ResHandle rh = get_resource_handle(instruction_type);

  // allocate text holder and load resource
  instruction_text_ptr = malloc(size*sizeof(char));
  resource_load(rh, (uint8_t *)instruction_text_ptr, size*sizeof(char));

  // get bounds of the window and set bounds of text
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect text_bounds = GRect(0, 0, bounds.size.w, height);

  // create layers
  s_instruction_scroll_layer = scroll_layer_create(bounds);
  s_instruction_text_layer = text_layer_create(text_bounds);

  // setup scroll view
  scroll_layer_set_content_size(s_instruction_scroll_layer, GSize(SCROLL_WIDTH,height));
  scroll_layer_set_click_config_onto_window(s_instruction_scroll_layer, s_instruction_window);

  // setup text view
  text_layer_set_text(s_instruction_text_layer, instruction_text_ptr);
  text_layer_set_text_alignment(s_instruction_text_layer, GTextAlignmentCenter);
  
  // add children
  layer_add_child(window_get_root_layer(s_instruction_window), scroll_layer_get_layer(s_instruction_scroll_layer));
  scroll_layer_add_child(s_instruction_scroll_layer, text_layer_get_layer(s_instruction_text_layer));

  #if defined(PBL_ROUND)
    text_layer_set_overflow_mode(s_instruction_text_layer, GTextOverflowModeWordWrap);
    text_layer_enable_screen_text_flow_and_paging(s_instruction_text_layer, 15);
    scroll_layer_set_paging(s_instruction_scroll_layer, true);
  #endif

  scroll_layer_set_content_offset(s_instruction_scroll_layer, GPoint(0,0), false);

  // free memory
  free(instruction_text_ptr);
}

void instruction_window_unload(Window *window) {
  scroll_layer_destroy(s_instruction_scroll_layer);
  text_layer_destroy(s_instruction_text_layer);
  window_destroy(s_instruction_window);
  s_instruction_window = NULL;
}

void text_init(short game) {
  // Create main Window element and assign to pointer
  instruction_type = game;
  s_instruction_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_instruction_window, (WindowHandlers) {
    .load = instruction_window_load,
    .unload = instruction_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_instruction_window, true);
}