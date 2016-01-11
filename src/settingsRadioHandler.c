#include <pebble.h>
#include "pebble-games.h"
#include "settingsRadioHandler.h"

#define RADIO_BUTTON_WINDOW_CELL_HEIGHT  44
#define RADIO_BUTTON_WINDOW_RADIO_RADIUS 6

static Window *s_settings_radio_window;
static MenuLayer *s_settings_radio_menu_layer;

static short settings_type;
static short s_current_selection = 0;
static short num_rows;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return num_rows + 1;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  if(cell_index->row == num_rows) {
    // This is the submit item
    menu_cell_basic_draw(ctx, cell_layer, "Submit", NULL, NULL);
  } else {
    // This is a choice item
    static char *s_buff;
    #ifdef CHESS 
    if (cell_index->row == CHESS_EASY) {
      s_buff = "Easy";
    } else if (cell_index->row == CHESS_HARD) {
      s_buff = "Hard";
    }
    #endif
    menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);

    // get bounds and set point
    GRect bounds = layer_get_bounds(cell_layer);
    GPoint p = GPoint(bounds.size.w - (3 * RADIO_BUTTON_WINDOW_RADIO_RADIUS), (bounds.size.h / 2));

    // Selected?
    if(menu_cell_layer_is_highlighted(cell_layer)) {
      graphics_context_set_stroke_color(ctx, GColorWhite);
      graphics_context_set_fill_color(ctx, GColorWhite);
    } else {
      graphics_context_set_fill_color(ctx, GColorBlack);
    }

    // Draw radio filled/empty
    graphics_draw_circle(ctx, p, RADIO_BUTTON_WINDOW_RADIO_RADIUS);
    if(cell_index->row == s_current_selection) {
      // This is the selection
      graphics_fill_circle(ctx, p, RADIO_BUTTON_WINDOW_RADIO_RADIUS - 3);
    }
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ? 
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    44);
}

static void save_setting() {
  #ifdef CHESS
  if (settings_type == CHESS_DIFFICULTY_SETTING) {
    persist_write_int(CHESS_DIFFICULTY_KEY, s_current_selection);
  }
  #endif
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  if(cell_index->row == num_rows) {
    save_setting();
    window_stack_pop(true);
  } else {
    // Change selection
    s_current_selection = cell_index->row;
    menu_layer_reload_data(menu_layer);
  }
}

static void set_num_rows() {
  #ifdef CHESS
  if (settings_type == CHESS_DIFFICULTY_SETTING) {
    num_rows = 2;
    return;
  }
  #endif
  num_rows = 0;
}

static void load_setting() {
  #ifdef CHESS
  if (settings_type == CHESS_DIFFICULTY_SETTING) {
    if (persist_exists(CHESS_BOARD_STATE_KEY)) {
      s_current_selection = persist_read_int(CHESS_DIFFICULTY_KEY);
    }
  }
  #endif
}

static void window_load(Window *window) {
  load_setting();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_settings_radio_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_settings_radio_menu_layer, window);
  menu_layer_set_callbacks(s_settings_radio_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      .select_click = select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_settings_radio_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_settings_radio_menu_layer);

  window_destroy(window);
  s_settings_radio_window = NULL;
}

void settings_radio_init(short type) {
  settings_type = type;
  set_num_rows();

  if(!s_settings_radio_window) {
    s_settings_radio_window = window_create();
    window_set_window_handlers(s_settings_radio_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
  }
  window_stack_push(s_settings_radio_window, true);
}