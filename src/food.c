#include <pebble.h>
#include "food.h"
  
#define RIGHT 0
#define UP 1
#define LEFT 2
#define DOWN 3
  
#define DIRECTION_RIGHT -1
#define DIRECTION_LEFT 1

#define MOVE_LENGTH 1
#define FRAME_WIDTH 120
#define TIMEOUT 200
#define BORDER 72-FRAME_WIDTH/2
  
#define GRID_SIZE 20
#define MAX_PLAYER_LENGTH 99
  
#define BOX_WIDTH FRAME_WIDTH/GRID_SIZE
  
#define PLAYER_LOCATION_KEY 24123
  
static Window *s_food_window;
static Layer *s_food_layer;

static AppTimer *timer;

static short *x_pos;
static short *y_pos;
static short length = 1;
static short direction = RIGHT;
static short paused = 0;
static short turned = 0;
static short food_x = 12;
static short food_y = 15;

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
  paused = 2;  
}

static void check_end_game() {
  if (x_pos[0]<0 || x_pos[0]>=GRID_SIZE || y_pos[0]<0 || y_pos[0]>=GRID_SIZE) {
    end_game();
  } else if (check_if_in_player(x_pos[0], y_pos[0], 1) == 1) {
    end_game();
  }
}

static void move() {
  short captured = 0;
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
  turned = 0;
}

static void move_with_timer() {
  move();
  layer_mark_dirty(s_food_layer);
  if (paused == 0) {
    timer = app_timer_register(TIMEOUT, move_with_timer, NULL);
  }
}

static void turn(short direction_turned) {
  if (turned==0) {
    direction+=direction_turned;
    if (direction<0) {
      direction+=4;
    } else {
      direction = direction%4;
    }
  }
  turned=1;
}

void reset() {
  app_timer_cancel(timer);
  length = 1;
  init_xy();
  direction = RIGHT;
  paused = 0;
  turned = 0;
  generate_food();
  layer_mark_dirty(s_food_layer);
  timer = app_timer_register(TIMEOUT, move_with_timer, NULL);
}

static void pause() {
  if (paused == 1) {
    timer = app_timer_register(TIMEOUT, move_with_timer, NULL);
    paused = 0;
  } else if (paused == 2) {
    reset();
  } else {
    app_timer_cancel(timer);
    paused = 1;
  }
}

static void food_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  turn(DIRECTION_LEFT);
}

static void food_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  pause();
}

static void food_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  turn(DIRECTION_RIGHT);
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

static void draw_food(Layer *layer, GContext *ctx) {
  // draw container
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, GRect(BORDER, BORDER, FRAME_WIDTH, FRAME_WIDTH));
  #ifdef PBL_PLATFORM_BASALT
    graphics_context_set_fill_color(ctx, GColorBrightGreen);
    graphics_fill_rect(ctx, GRect(BORDER, BORDER, FRAME_WIDTH, FRAME_WIDTH), 0, GCornerNone);
  #endif
    
  #ifdef PBL_PLATFORM_BASALT
    graphics_context_set_fill_color(ctx, GColorCobaltBlue);
  #else
    graphics_context_set_fill_color(ctx, GColorBlack);
  #endif
  int k;
  for (k=0;x_pos[k]>-1;k++) {
    graphics_fill_rect(ctx, GRect(x_pos[k]*BOX_WIDTH+BORDER, y_pos[k]*BOX_WIDTH+BORDER, BOX_WIDTH-1, BOX_WIDTH-1), 0, GCornerNone);
  }
  #ifdef PBL_PLATFORM_BASALT
    graphics_context_set_fill_color(ctx, GColorRed);
  #endif
  graphics_fill_rect(ctx, GRect(food_x*BOX_WIDTH+BORDER, food_y*BOX_WIDTH+BORDER, BOX_WIDTH-1, BOX_WIDTH-1), 0, GCornerNone);
  
  char score[3] = "  ";
  if (length>=10) {
    score[0] = (char)(((int)'0')+length/10);
  } else {
    score[0] = ' ';
  }
  score[1] = (char)(((int)'0')+length%10);
  
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, score, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(52,FRAME_WIDTH+BORDER,40,20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  
  if (paused==2) {
    graphics_draw_text(ctx, "End", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(52,62,40,20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }
}

static void food_window_load(Window *window) {  
  x_pos=malloc(MAX_PLAYER_LENGTH*sizeof(short));
  y_pos=malloc(MAX_PLAYER_LENGTH*sizeof(short));
  init_xy();
  generate_food();
  
  s_food_layer = layer_create(GRect(0, 0, 144, 152));
  layer_set_update_proc(s_food_layer, draw_food);
  layer_add_child(window_get_root_layer(s_food_window), s_food_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) food_config_provider);
  timer = app_timer_register(TIMEOUT, move_with_timer, NULL);
}

static void food_window_unload(Window *window) {
  free(x_pos);
  free(y_pos);
  layer_destroy(s_food_layer);
  window_destroy(s_food_window);
}

void food_init() {  
  // Create main Window element and assign to pointer
  s_food_window = window_create();

  #ifdef PBL_PLATFORM_BASALT
    window_set_background_color(s_food_window, GColorIslamicGreen);
  #endif
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_food_window, (WindowHandlers) {
    .load = food_window_load,
    .unload = food_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_food_window, true);
}