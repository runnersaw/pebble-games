#include <pebble.h>
#include "tennis.h"

#define WIDTH 140
#define HEIGHT 100
#define HORIZ_VEL 3
#define PADDLE_HEIGHT 20
#define PADDLE_DISTANCE 10
#define BALL_RADIUS 2
#define WAIT 40
#define CPU_VEL 3
#define PERSON_VEL 3
#define MAX_Y_VEL 5
  
static Window *s_tennis_window;
static Layer *s_tennis_layer;
static Layer *s_tennis_container_layer;
static TextLayer *s_tennis_player_score_layer;
static TextLayer *s_tennis_cpu_score_layer;

static AppTimer *timer;

static short ball_x_vel = HORIZ_VEL;
static short ball_y_vel = 0;
static short ball_x_pos = WIDTH/2;
static short ball_y_pos = HEIGHT/2;
static short person_vel = 0;
static short cpu_vel = 0;
static short person_y_pos = HEIGHT/2-PADDLE_HEIGHT/2;
static short cpu_y_pos = HEIGHT/2-PADDLE_HEIGHT/2;
static short paused = 0;

static short person_score = 0;
static short cpu_score = 0;

static char person_score_char;
static char cpu_score_char;

static void draw_tennis_container(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, GPoint(0,0), GPoint(WIDTH, 0));
  graphics_draw_line(ctx, GPoint(0, HEIGHT), GPoint(WIDTH, HEIGHT));
}

static void draw_tennis(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(ball_x_pos, ball_y_pos), BALL_RADIUS);
  graphics_draw_line(ctx, GPoint(PADDLE_DISTANCE, person_y_pos), GPoint(PADDLE_DISTANCE, person_y_pos+PADDLE_HEIGHT));
  graphics_draw_line(ctx, GPoint(WIDTH-PADDLE_DISTANCE, cpu_y_pos), GPoint(WIDTH-PADDLE_DISTANCE, cpu_y_pos+PADDLE_HEIGHT));
}

static void draw_score() {
  text_layer_set_background_color(s_tennis_player_score_layer, GColorClear);
  text_layer_set_text_color(s_tennis_player_score_layer, GColorWhite);
  person_score_char = (char)(((int)'0')+person_score);
  text_layer_set_text(s_tennis_player_score_layer, &person_score_char);
  text_layer_set_text_alignment(s_tennis_player_score_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_tennis_window), text_layer_get_layer(s_tennis_player_score_layer));
  
  text_layer_set_background_color(s_tennis_cpu_score_layer, GColorClear);
  text_layer_set_text_color(s_tennis_cpu_score_layer, GColorWhite);
  cpu_score_char = (char)(((int)'0')+cpu_score);
  text_layer_set_text(s_tennis_cpu_score_layer, &cpu_score_char);
  text_layer_set_text_alignment(s_tennis_cpu_score_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_tennis_window), text_layer_get_layer(s_tennis_cpu_score_layer));
}

static void update_score() {  
  person_score_char = (char)(((int)'0')+person_score);
  text_layer_set_text(s_tennis_player_score_layer, &person_score_char);  
  cpu_score_char = (char)(((int)'0')+cpu_score);
  text_layer_set_text(s_tennis_cpu_score_layer, &cpu_score_char);
}

static void move() {
  if (ball_y_pos>(cpu_y_pos+PADDLE_HEIGHT/2)+5) {
    cpu_vel = CPU_VEL;
  } else if (ball_y_pos<(cpu_y_pos+PADDLE_HEIGHT/2)-5) {
    cpu_vel = -CPU_VEL;
  } else {
    cpu_vel = 0;
  }
  if (person_y_pos>HEIGHT-PADDLE_HEIGHT && person_vel>0) {
    person_vel = 0;
  }
  if (person_y_pos<0 && person_vel<0) {
    person_vel = 0;
  }
  if (cpu_y_pos>HEIGHT-PADDLE_HEIGHT && cpu_vel>0) {
    cpu_vel = 0;
  }
  if (cpu_y_pos<0 && cpu_vel<0) {
    cpu_vel = 0;
  }
  person_y_pos += person_vel;
  cpu_y_pos += cpu_vel;
  ball_x_pos += ball_x_vel;
  ball_y_pos += ball_y_vel;
  if (ball_x_pos>WIDTH) {
    ball_x_vel = -HORIZ_VEL;
    ball_y_vel = rand() % MAX_Y_VEL;
    ball_x_pos = WIDTH/2;
    ball_y_pos = HEIGHT/2;
    person_score += 1;
    update_score();
  } else if (ball_x_pos<0) {
    ball_x_vel = HORIZ_VEL;
    ball_y_vel = rand() % MAX_Y_VEL;
    ball_x_pos = WIDTH/2;
    ball_y_pos = HEIGHT/2;
    cpu_score += 1;
    update_score();
  }
  if (ball_y_pos<=0 || ball_y_pos>=HEIGHT) {
    ball_y_vel = -ball_y_vel;
  }
  if (ball_x_pos>PADDLE_DISTANCE-BALL_RADIUS && ball_x_pos<=PADDLE_DISTANCE+BALL_RADIUS) {
    if (ball_y_pos>=person_y_pos && ball_y_pos<=person_y_pos+PADDLE_HEIGHT) {
      if (ball_x_vel < 0) {
        ball_x_vel = -ball_x_vel;
        ball_y_vel = (ball_y_pos - person_y_pos - PADDLE_HEIGHT/2)*MAX_Y_VEL*2;
        ball_y_vel = ball_y_vel/PADDLE_HEIGHT;
      }
    }
  }
  if (ball_x_pos>=WIDTH-PADDLE_DISTANCE-BALL_RADIUS && ball_x_pos<WIDTH-PADDLE_DISTANCE+BALL_RADIUS) {
    if (ball_y_pos>=cpu_y_pos && ball_y_pos<=cpu_y_pos+PADDLE_HEIGHT) {
      if (ball_x_vel > 0) {
        ball_x_vel = -ball_x_vel;
        ball_y_vel = (ball_y_pos - cpu_y_pos - PADDLE_HEIGHT/2)*MAX_Y_VEL*2;
        ball_y_vel = ball_y_vel/PADDLE_HEIGHT;
      }
    }
  }
}

static void move_with_timer() {
  move();
  layer_mark_dirty(s_tennis_layer);
  timer = app_timer_register(WAIT, move_with_timer, NULL);
}

static void pause(ClickRecognizerRef recognizer, void *context) {
  if (paused==0) {
    paused = 1;
    app_timer_cancel(timer);
  } else {
    paused = 0;
    timer = app_timer_register(WAIT, move_with_timer, NULL);
  }
}

static void begin_up(ClickRecognizerRef recognizer, void *context) {
  person_vel = -PERSON_VEL;
}

static void end_move(ClickRecognizerRef recognizer, void *context) {
  person_vel = 0;
}

static void begin_down(ClickRecognizerRef recognizer, void *context) {
  person_vel = PERSON_VEL;
}

void reset_game() {
  paused = 0;
  app_timer_cancel(timer);
  ball_x_vel = HORIZ_VEL;
  ball_y_vel = 0;
  ball_x_pos = WIDTH/2;
  ball_y_pos = HEIGHT/2;
  person_vel = 0;
  cpu_vel = 0;
  person_y_pos = HEIGHT/2-PADDLE_HEIGHT/2;
  cpu_y_pos = HEIGHT/2-PADDLE_HEIGHT/2;

  person_score = 0;
  cpu_score = 0;
  timer = app_timer_register(WAIT, move_with_timer, NULL);
}

static void reset(ClickRecognizerRef recognizer, void *context) {
  reset_game();
}

static void back(ClickRecognizerRef recognizer, void *context) {
  reset_game();
  app_timer_cancel(timer);
  window_stack_pop(true);
}

void tennis_config_provider(Window *window) {
  // set click listeners
  window_raw_click_subscribe(BUTTON_ID_UP, begin_up, end_move, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, pause);
  window_single_click_subscribe(BUTTON_ID_BACK, back);
  window_raw_click_subscribe(BUTTON_ID_DOWN, begin_down, end_move, NULL);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, reset, NULL);
}

static void tennis_window_load(Window *window) {                                              
  s_tennis_container_layer = layer_create(GRect(2, 5, WIDTH, HEIGHT+1));
  s_tennis_layer = layer_create(GRect(0, 0, WIDTH, HEIGHT+1));
  s_tennis_player_score_layer = text_layer_create(GRect(0, HEIGHT+10, 72, 148-HEIGHT-20));
  s_tennis_cpu_score_layer = text_layer_create(GRect(72, HEIGHT+10, 72, 148-HEIGHT-20));
  
  layer_set_update_proc(s_tennis_layer, draw_tennis);
  layer_set_update_proc(s_tennis_container_layer, draw_tennis_container);
  layer_add_child(window_get_root_layer(s_tennis_window), s_tennis_container_layer);
  layer_add_child(s_tennis_container_layer, s_tennis_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) tennis_config_provider);
  
  ball_y_vel = rand() % MAX_Y_VEL;
  draw_score();
  timer = app_timer_register(WAIT, move_with_timer, NULL);
}

static void tennis_window_unload(Window *window) {
  layer_destroy(s_tennis_layer);
  layer_destroy(s_tennis_container_layer);
  text_layer_destroy(s_tennis_player_score_layer);
  text_layer_destroy(s_tennis_cpu_score_layer);
  window_destroy(s_tennis_window);
}

void tennis_init() {  
  // Create main Window element and assign to pointer
  s_tennis_window = window_create();
  
  window_set_background_color(s_tennis_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_tennis_window, (WindowHandlers) {
    .load = tennis_window_load,
    .unload = tennis_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_tennis_window, true);
}