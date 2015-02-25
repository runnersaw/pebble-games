#include <pebble.h>
#include "food.h"
  
#define RIGHT 0
#define UP 1
#define LEFT 2
#define DOWN 3

#define MOVE_LENGTH 1
#define FRAME_WIDTH 120
#define TIMEOUT 200
  
#define GRID_SIZE 20
#define MAX_PLAYER_LENGTH 64
  
#define BOX_WIDTH FRAME_WIDTH/GRID_SIZE
  
#define PLAYER_LOCATION_KEY 24123
  
static Window *s_food_window;
static Layer *s_food_container_layer;
static Layer *s_food_layer;
static TextLayer *s_food_lost_layer;
static TextLayer *s_food_score_layer;

static AppTimer *timer;

static short *x_pos;
static short *y_pos;
static short length = 1;
static short direction = RIGHT;
static short paused = 0;
static short turned = 0;
static short captured = 0;
static short food_x = 12;
static short food_y = 15;

static char score[3] = "  ";

static short check_if_in_player(int x, int y, int start) {
  short l;
  for (l=start;x_pos[l]>-1;l++) {
    if (x==x_pos[l] && y==y_pos[l]) {
      return 1;
    }
  }
  return 0;
}

static void generate_food() {
  food_x = rand() % 20;
  food_y = rand() % 20;
  if (check_if_in_player(food_x, food_y, 0) == 1) {
    generate_food();
  }
}

static void draw_score() {
  text_layer_set_background_color(s_food_score_layer, GColorClear);
  text_layer_set_text_color(s_food_score_layer, GColorBlack);
  
  if (length>=10) {
    score[0] = (char)(((int)'0')+length/10);
  } else {
    score[0] = ' ';
  }
  score[1] = (char)(((int)'0')+length%10);
  
  text_layer_set_text(s_food_score_layer, score);

  text_layer_set_text_alignment(s_food_score_layer, GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(s_food_window), text_layer_get_layer(s_food_score_layer));
}

static void update_score() {  
  if (length>=10) {
    score[0] = (char)(((int)'0')+length/10);
  } else {
    score[0] = ' ';
  }
  score[1] = (char)(((int)'0')+length%10);
  text_layer_set_text(s_food_score_layer, score);  
}

static void init_xy() {
  short j;
  for (j=0;j<MAX_PLAYER_LENGTH;j++) {
    *(x_pos+j) = -1;
    *(y_pos+j) = -1; 
  }
  x_pos[0] = GRID_SIZE/2;
  y_pos[0] = GRID_SIZE/2;
}

static void end_game() {
  app_timer_cancel(timer);
  paused = 1;
  
  text_layer_set_background_color(s_food_lost_layer, GColorClear);
  text_layer_set_text_color(s_food_lost_layer, GColorBlack);
  text_layer_set_text(s_food_lost_layer, "End");

  // Improve the layout to be more like a watchface
  text_layer_set_text_alignment(s_food_lost_layer, GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(s_food_window), text_layer_get_layer(s_food_lost_layer));
  
}

static void check_end_game() {
  if (x_pos[0]<0 || x_pos[0]>=GRID_SIZE || y_pos[0]<0 || y_pos[0]>=GRID_SIZE) {
    end_game();
  } else if (check_if_in_player(x_pos[0], y_pos[0], 1) == 1) {
    end_game();
  }
}

static void move() {
  short temp_x = x_pos[0];
  short temp_y = y_pos[0];
  if (direction == RIGHT) {
    temp_x += MOVE_LENGTH;
  } else if (direction == DOWN) {
    temp_y += MOVE_LENGTH;
  } else if (direction == UP) {
    temp_y -= MOVE_LENGTH;
  } else if (direction == LEFT) {
    temp_x -= MOVE_LENGTH;
  }
  if (temp_x == food_x && temp_y == food_y) {
    captured = 1;
    generate_food();
  } else {
    captured = 0;
  }
  short i;
  if (captured == 1) {
    x_pos[length] = x_pos[length-1];
    y_pos[length] = y_pos[length-1];
  }
  for (i=length-1;i>0;i--) {
    x_pos[i] = x_pos[i-1];
    y_pos[i] = y_pos[i-1];
  }
  x_pos[0] = temp_x;
  y_pos[0] = temp_y;
  if (captured == 1) {
    length += 1;
  }
  check_end_game();
  update_score();
  turned = 0;
}

static void move_with_timer() {
  move();
  layer_mark_dirty(s_food_layer);
  if (paused == 0) {
    timer = app_timer_register(TIMEOUT, move_with_timer, NULL);
  }
}

static void turn_right() {
  if (turned == 0) {
    if (direction!=0) {
      direction -= 1;
    } else {
      direction = 3;
    }
  }
  turned = 1;
}

static void turn_left() {
  if (turned == 0) {
    if (direction!=3) {
      direction += 1;
    } else {
      direction = 0;
    }
  }
  turned = 1;
}

static void pause() {
  if (paused == 1) {
    timer = app_timer_register(TIMEOUT, move_with_timer, NULL);
    paused = 0;
  } else {
    app_timer_cancel(timer);
    paused = 1;
  }
}

static void remove_end_game() {
  layer_remove_from_parent(text_layer_get_layer(s_food_lost_layer));
}

void reset() {
  app_timer_cancel(timer);
  length = 1;
  init_xy();
  direction = RIGHT;
  paused = 0;
  turned = 0;
  captured = 0;
  generate_food();
  update_score();
  remove_end_game();
  timer = app_timer_register(TIMEOUT, move_with_timer, NULL);
}

static void food_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  turn_left();
}

static void food_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  pause();
}

static void food_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  turn_right();
}

static void food_select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  reset();
}

static void food_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  reset();
  app_timer_cancel(timer);
  window_stack_pop(true);
}

void food_config_provider(Window *window) {
  // set click listeners
  window_single_click_subscribe(BUTTON_ID_UP, food_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, food_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, food_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, food_back_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, food_select_long_click_handler, NULL);
}

static void draw_food_container(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, layer_get_bounds(layer));
}

static void draw_food(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  int k;
  for (k=0;x_pos[k]>-1;k++) {
    graphics_fill_rect(ctx, GRect(x_pos[k]*BOX_WIDTH, y_pos[k]*BOX_WIDTH, BOX_WIDTH-1, BOX_WIDTH-1), 0, GCornerNone);
  }
  graphics_fill_rect(ctx, GRect(food_x*BOX_WIDTH, food_y*BOX_WIDTH, BOX_WIDTH-1, BOX_WIDTH-1), 0, GCornerNone);
}

static void food_window_load(Window *window) {  
  s_food_container_layer = layer_create(GRect(12, 12, FRAME_WIDTH, FRAME_WIDTH));
  s_food_layer = layer_create(GRect(0, 0, FRAME_WIDTH, FRAME_WIDTH));
  s_food_lost_layer = text_layer_create(GRect(0, 55, 144, 50));
  s_food_score_layer = text_layer_create(GRect(0, 130, 144, 30));
  
  layer_set_update_proc(s_food_container_layer, draw_food_container);
  layer_set_update_proc(s_food_layer, draw_food);
  layer_add_child(window_get_root_layer(s_food_window), s_food_container_layer);
  layer_add_child(s_food_container_layer, s_food_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) food_config_provider);
  
  draw_score();  
  timer = app_timer_register(TIMEOUT, move_with_timer, NULL);
}

static void food_window_unload(Window *window) {
  free(x_pos);
  free(y_pos);
  layer_destroy(s_food_layer);
  layer_destroy(s_food_container_layer);
  layer_destroy(text_layer_get_layer(s_food_lost_layer));
  layer_destroy(text_layer_get_layer(s_food_score_layer));
  window_destroy(s_food_window);
}

void food_init() {
  x_pos=malloc(MAX_PLAYER_LENGTH*sizeof(short));
  y_pos=malloc(MAX_PLAYER_LENGTH*sizeof(short));
  init_xy();
  generate_food();
  
  // Create main Window element and assign to pointer
  s_food_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_food_window, (WindowHandlers) {
    .load = food_window_load,
    .unload = food_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_food_window, true);
}