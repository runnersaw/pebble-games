#include <pebble.h>
#include "menuHandler.h"
#include "windowHandler.h"
  
#define NUM_MENU_ITEMS 3

static Window *s_menu_window;
static SimpleMenuLayer *s_games_menu;

// A simple menu layer can have multiple sections
static SimpleMenuSection menu_sections[1];

// Each section is composed of a number of menu items
static SimpleMenuItem menu_items[NUM_MENU_ITEMS];

static void menu_window_load(Window *window) {
  menu_items[0] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "Snake",
    .callback = snake_chosen,
  };
  
  menu_items[1] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "Pong",
    .callback = pong_chosen,
  };
  
  menu_items[2] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "Chess",
    .callback = chess_chosen,
  };
  
  // Header
  menu_sections[0] = (SimpleMenuSection) {
    .items = menu_items,
    .num_items = ARRAY_LENGTH(menu_items)
  };
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  
  s_games_menu = simple_menu_layer_create(bounds, window, menu_sections, 1, NULL);

  // Add it to the window for display
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_games_menu));
}

static void menu_window_unload(Window *window) {
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

/*int main(void) {
  address_init();
  app_event_loop();
  address_deinit();
}*/
