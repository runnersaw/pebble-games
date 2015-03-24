#include <pebble.h>
#include "menuHandler.h"
#include "food.h"
#include "tennis.h"
#include "chess.h"
#include "blackjack.h"
#include "2048.h"
#include "mastermind.h"

#ifdef PBL_PLATFORM_BASALT
  #define NUM_MENU_ITEMS 7
#else
  #define NUM_MENU_ITEMS 6
#endif
#define CHAR_NUM 350
#define HEIGHT 350

static Window *s_menu_window;
static Window *s_about_window;
static ScrollLayer *s_about_scroll_layer;
static TextLayer * s_about_text_layer;
static SimpleMenuLayer *s_games_menu;
static char *about_text_ptr;

static GBitmap *info_icon;
static GBitmap *chess_icon;
static GBitmap *tennis_icon;
static GBitmap *food_icon;
static GBitmap *blackjack_icon;
static GBitmap *two048_icon;
#ifdef PBL_PLATFORM_BASALT
  static GBitmap *mastermind_icon;
#endif

// A simple menu layer can have multiple sections
static SimpleMenuSection menu_sections[1];

// Each section is composed of a number of menu items
static SimpleMenuItem menu_items[NUM_MENU_ITEMS];

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
    mastermind_icon = gbitmap_create_with_resource(RESOURCE_ID_MASTERMIND_ICON);
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
    gbitmap_destroy(mastermind_icon);
  #endif
}

static void menu_window_load(Window *window) {
  load_bitmaps();
  short index = 0;
  
  menu_items[index] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "Chess",
    .callback = chess_init,
    .icon=chess_icon
  };
  index++;
  
  menu_items[index] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "Blackjack",
    .callback = blackjack_init,
    .icon = blackjack_icon
  };
  index++;
  
  menu_items[index] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "2048",
    .callback = two048_init,
    .icon = two048_icon
  };
  index++;
  
  #ifdef PBL_PLATFORM_BASALT
  menu_items[index] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "Mastermind",
    .callback = mastermind_init,
    .icon = mastermind_icon
  };
  index++;
  #endif
  
  menu_items[index] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "FOOD!",
    .callback = food_init,
    .icon = food_icon
  };
  index++;
  
  menu_items[index] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "Tennis",
    .callback = tennis_init,
    .icon = tennis_icon
  };
  index++;
  
  menu_items[index] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "About",
    .callback = about_chosen,
    .icon = info_icon
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
  destroy_bitmaps();
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
    layer_destroy(simple_menu_layer_get_layer(s_games_menu));
    window_destroy(s_menu_window);
}

int main() {
  menu_init();
  app_event_loop();
  menu_deinit();
}