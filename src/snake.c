#include <pebble.h>
#include "snake.h"
  
#define RIGHT 0
#define UP 1
#define LEFT 2
#define DOWN 3

#define MOVE_LENGTH 1
#define FRAME_WIDTH 120
#define TIMEOUT 200
  
#define GRID_SIZE 20
#define MAX_SNAKE_LENGTH 64
  
#define BOX_WIDTH FRAME_WIDTH/GRID_SIZE
  
static Window *s_snake_window;
static Layer *s_snake_container_layer;
static Layer *s_snake_layer;
static TextLayer *s_snake_lost_layer;
static TextLayer *s_snake_score_layer;

static AppTimer *timer;

static int x_pos[MAX_SNAKE_LENGTH];
static int y_pos[MAX_SNAKE_LENGTH];
static int length = 1;
static int direction = RIGHT;
static int paused = 0;
static int turned = 0;
static int captured = 0;
static int food_x = 12;
static int food_y = 15;

static char score[12];

static int check_if_in_snake(int x, int y, int start) {
  int l;
  for (l=start;x_pos[l]>-1;l++) {
    if (x==x_pos[l] && y==y_pos[l]) {
      return 1;
      APP_LOG(APP_LOG_LEVEL_INFO, "Found at position %d", l);
    }
  }
  return 0;
}

static void generate_food() {
  food_x = rand() % 20;
  food_y = rand() % 20;
  if (check_if_in_snake(food_x, food_y, 0) == 1) {
    generate_food();
  }
}

static void draw_score() {
  s_snake_score_layer = text_layer_create(GRect(0, 130, 144, 30));
  text_layer_set_background_color(s_snake_score_layer, GColorClear);
  text_layer_set_text_color(s_snake_score_layer, GColorBlack);
  
  snprintf(score, 12, "Score: %d", length);
  
  text_layer_set_text(s_snake_score_layer, score);

  text_layer_set_text_alignment(s_snake_score_layer, GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(s_snake_window), text_layer_get_layer(s_snake_score_layer));
}

static void update_score() {  
  snprintf(score, 12, "Score: %d", length);
  
  text_layer_set_text(s_snake_score_layer, score);  
}

static void init_xy() {
  int j;
  for (j=0;j<MAX_SNAKE_LENGTH;j++) {
    x_pos[j] = -1;
    y_pos[j] = -1; 
  }
  x_pos[0] = GRID_SIZE/2;
  y_pos[0] = GRID_SIZE/2;
}

static void end_game() {
  app_timer_cancel(timer);
  paused = 1;
  
  s_snake_lost_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_snake_lost_layer, GColorClear);
  text_layer_set_text_color(s_snake_lost_layer, GColorBlack);
  text_layer_set_text(s_snake_lost_layer, "End Game");

  // Improve the layout to be more like a watchface
  text_layer_set_text_alignment(s_snake_lost_layer, GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(s_snake_window), text_layer_get_layer(s_snake_lost_layer));
  
}

static void check_end_game() {
  if (x_pos[0]<0 || x_pos[0]>=GRID_SIZE || y_pos[0]<0 || y_pos[0]>=GRID_SIZE) {
    end_game();
  } else if (check_if_in_snake(x_pos[0], y_pos[0], 1) == 1) {
    end_game();
  }
}

static void move() {
  int temp_x = x_pos[0];
  int temp_y = y_pos[0];
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
  int i;
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
  layer_mark_dirty(s_snake_layer);
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
  layer_remove_from_parent(text_layer_get_layer(s_snake_lost_layer));
}

void reset() {
  APP_LOG(APP_LOG_LEVEL_INFO, "RESETTING");
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

static void snake_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  turn_left();
}

static void snake_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  pause();
}

static void snake_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  turn_right();
}

static void snake_select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  reset();
}

void snake_config_provider(Window *window) {
  // set click listeners
  window_single_click_subscribe(BUTTON_ID_UP, snake_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, snake_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, snake_down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, snake_select_long_click_handler, NULL);
}

static void draw_snake_container(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, layer_get_bounds(layer));
}

static void draw_snake(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  int k;
  for (k=0;x_pos[k]>-1;k++) {
    graphics_fill_rect(ctx, GRect(x_pos[k]*BOX_WIDTH, y_pos[k]*BOX_WIDTH, BOX_WIDTH-1, BOX_WIDTH-1), 0, GCornerNone);
  }
  graphics_fill_rect(ctx, GRect(food_x*BOX_WIDTH, food_y*BOX_WIDTH, BOX_WIDTH-1, BOX_WIDTH-1), 0, GCornerNone);
}

static void snake_window_load(Window *window) {
  s_snake_container_layer = layer_create(GRect(12, 12, FRAME_WIDTH, FRAME_WIDTH));
  s_snake_layer = layer_create(GRect(0, 0, FRAME_WIDTH, FRAME_WIDTH));
  
  layer_set_update_proc(s_snake_container_layer, draw_snake_container);
  layer_set_update_proc(s_snake_layer, draw_snake);
  layer_add_child(window_get_root_layer(s_snake_window), s_snake_container_layer);
  layer_add_child(s_snake_container_layer, s_snake_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) snake_config_provider);
  
  draw_score();  
  timer = app_timer_register(TIMEOUT, move_with_timer, NULL);
}

static void snake_window_unload(Window *window) {
  layer_destroy(s_snake_layer);
  layer_destroy(s_snake_container_layer);
  layer_destroy(text_layer_get_layer(s_snake_lost_layer));
  layer_destroy(text_layer_get_layer(s_snake_score_layer));
  window_destroy(s_snake_window);
}

void snake_init() {
  init_xy();
  generate_food();
  
  // Create main Window element and assign to pointer
  s_snake_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_snake_window, (WindowHandlers) {
    .load = snake_window_load,
    .unload = snake_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_snake_window, true);
}

void snake_deinit() {
    // Destroy Window
    window_destroy(s_snake_window);
}

/*int main(void) {
  address_init();
  app_event_loop();
  address_deinit();
}*/