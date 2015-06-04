#include <pebble.h>
#include "instructions.h"
#include "menuHandler.h"
#include "food.h"
#include "tennis.h"
#include "chess.h"
#include "blackjack.h"
#include "2048.h"
#include "decrypt.h"
  
#define CHESS 0
#define BLACKJACK 1
#define TWO048 3
#define FOOD 4
#define TENNIS 5
#define DECRYPT 6
  
static Window *s_play_menu_window;
static Window *s_instructions_window;
static TextLayer * s_instructions_text_layer;
static SimpleMenuLayer *s_play_menu;
static char *instructions_text_ptr;

static int game_chosen;

// A simple menu layer can have multiple sections
static SimpleMenuSection menu_sections[1];

// Each section is composed of a number of menu items
static SimpleMenuItem menu_items[2];

void instructions_window_load() {
  if (game_chosen == CHESS) {
    instructions_text_ptr = malloc(100*sizeof(char));
    ResHandle rh = resource_get_handle(RESOURCE_ID_PEBB_GAMES_ABOUT);
    resource_load(rh, (uint8_t *)instructions_text_ptr, 100*sizeof(char));
  }
  s_instructions_text_layer = text_layer_create(GRect(0,0,144,148));
  text_layer_set_text(s_instructions_text_layer, instructions_text_ptr);
  text_layer_set_text_alignment(s_instructions_text_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_instructions_window), text_layer_get_layer(s_instructions_text_layer));
}

void instructions_window_unload() {
  free(instructions_text_ptr);
  text_layer_destroy(s_instructions_text_layer);
  window_destroy(s_instructions_window);
}

void instructions() {
  // Create main Window element and assign to pointer
  s_instructions_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_instructions_window, (WindowHandlers) {
    .load = instructions_window_load,
    .unload = instructions_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_instructions_window, true);
}

static void menu_window_load(Window *window) {
  short index = 0;
  
  SimpleMenuLayerSelectCallback callback = NULL;
  if (game_chosen == CHESS) {
    callback = chess_init;
  }
  
  menu_items[index] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "Play",
    .callback = callback
  };
  index++;
  
  menu_items[index] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "Instructions",
    .callback = instructions
  };
  index++;
  
  // Header
  menu_sections[0] = (SimpleMenuSection) {
    .items = menu_items,
    .num_items = ARRAY_LENGTH(menu_items)
  };
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  
  s_play_menu = simple_menu_layer_create(bounds, window, menu_sections, 1, NULL);

  // Add it to the window for display
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_play_menu));
}

static void menu_window_unload(Window *window) {
}

static void play_menu_init() {
  // Create main Window element and assign to pointer
  s_play_menu_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_play_menu_window, (WindowHandlers) {
    .load = menu_window_load,
    .unload = menu_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_play_menu_window, true);
}

static void play_menu_deinit() {
    // Destroy Window
    simple_menu_layer_destroy(s_play_menu);
    window_destroy(s_play_menu_window);
}

void chess_chosen() {
  game_chosen = CHESS;
  play_menu_init();
}