#include <pebble.h>
#include "pong.h"

#define WIDTH 140
#define HEIGHT 100
#define HORIZ_VEL 3
#define PADDLE_HEIGHT 20
#define PADDLE_DISTANCE 10
#define BALL_RADIUS 2
#define WAIT 40
#define PERSON_VEL 2
#define MAX_Y_VEL 4
  
static Window *s_pong_window;
static Layer *s_pong_layer;
static Layer *s_pong_container_layer;
static TextLayer *s_pong_player_score_layer;
static TextLayer *s_pong_cpu_score_layer;

static AppTimer *timer;

static int ball_x_vel = HORIZ_VEL;
static int ball_y_vel = 0;
static int ball_x_pos = WIDTH/2;
static int ball_y_pos = HEIGHT/2;
static int person_vel = 0;
static int cpu_vel = 0;
static int person_y_pos = HEIGHT/2-PADDLE_HEIGHT/2;
static int cpu_y_pos = HEIGHT/2-PADDLE_HEIGHT/2;

static int person_score = 0;
static int cpu_score = 0;

static char person_score_char[12];
static char cpu_score_char[12];

static void draw_pong_container(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, GPoint(0,0), GPoint(WIDTH, 0));
  graphics_draw_line(ctx, GPoint(0, HEIGHT), GPoint(WIDTH, HEIGHT));
}

static void draw_pong(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(ball_x_pos, ball_y_pos), BALL_RADIUS);
  graphics_draw_line(ctx, GPoint(PADDLE_DISTANCE, person_y_pos), GPoint(PADDLE_DISTANCE, person_y_pos+PADDLE_HEIGHT));
  graphics_draw_line(ctx, GPoint(WIDTH-PADDLE_DISTANCE, cpu_y_pos), GPoint(WIDTH-PADDLE_DISTANCE, cpu_y_pos+PADDLE_HEIGHT));
}

static void draw_score() {
  s_pong_player_score_layer = text_layer_create(GRect(0, HEIGHT+10, 72, 148-HEIGHT-20));
  text_layer_set_background_color(s_pong_player_score_layer, GColorClear);
  text_layer_set_text_color(s_pong_player_score_layer, GColorWhite);
  snprintf(person_score_char, 12, "Player: %d", person_score);
  text_layer_set_text(s_pong_player_score_layer, person_score_char);
  text_layer_set_text_alignment(s_pong_player_score_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_pong_window), text_layer_get_layer(s_pong_player_score_layer));
  
  s_pong_cpu_score_layer = text_layer_create(GRect(72, HEIGHT+10, 72, 148-HEIGHT-20));
  text_layer_set_background_color(s_pong_cpu_score_layer, GColorClear);
  text_layer_set_text_color(s_pong_cpu_score_layer, GColorWhite);
  snprintf(cpu_score_char, 12, "CPU: %d", cpu_score);
  text_layer_set_text(s_pong_cpu_score_layer, cpu_score_char);
  text_layer_set_text_alignment(s_pong_cpu_score_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_pong_window), text_layer_get_layer(s_pong_cpu_score_layer));
}

static void update_score() {  
  snprintf(person_score_char, 12, "Player: %d", person_score);
  text_layer_set_text(s_pong_player_score_layer, person_score_char);
  snprintf(cpu_score_char, 12, "CPU: %d", cpu_score);
  text_layer_set_text(s_pong_cpu_score_layer, cpu_score_char);
}

static void move() {
  if (ball_y_pos>(cpu_y_pos+PADDLE_HEIGHT/2)) {
    cpu_vel = PERSON_VEL;
  } else {
    cpu_vel = -PERSON_VEL;
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
  layer_mark_dirty(s_pong_layer);
  timer = app_timer_register(WAIT, move_with_timer, NULL);
}

static void pause(ClickRecognizerRef recognizer, void *context) {
  
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

static void reset(ClickRecognizerRef recognizer, void *context) {
  
}

void pong_config_provider(Window *window) {
  // set click listeners
  window_raw_click_subscribe(BUTTON_ID_UP, begin_up, end_move, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, pause);
  window_raw_click_subscribe(BUTTON_ID_DOWN, begin_down, end_move, NULL);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, reset, NULL);
}

static void pong_window_load(Window *window) {
  s_pong_container_layer = layer_create(GRect(2, 5, WIDTH, HEIGHT+1));
  s_pong_layer = layer_create(GRect(0, 0, WIDTH, HEIGHT+1));
  
  layer_set_update_proc(s_pong_layer, draw_pong);
  layer_set_update_proc(s_pong_container_layer, draw_pong_container);
  layer_add_child(window_get_root_layer(s_pong_window), s_pong_container_layer);
  layer_add_child(s_pong_container_layer, s_pong_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) pong_config_provider);
  
  ball_y_vel = rand() % MAX_Y_VEL;
  draw_score();
  timer = app_timer_register(WAIT, move_with_timer, NULL);
}

static void pong_window_unload(Window *window) {
  layer_destroy(s_pong_layer);
  layer_destroy(s_pong_container_layer);
  layer_destroy(text_layer_get_layer(s_pong_player_score_layer));
  layer_destroy(text_layer_get_layer(s_pong_cpu_score_layer));
  window_destroy(s_pong_window);
}

void pong_init() {  
  // Create main Window element and assign to pointer
  s_pong_window = window_create();
  
  window_set_background_color(s_pong_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_pong_window, (WindowHandlers) {
    .load = pong_window_load,
    .unload = pong_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_pong_window, true);
}

void pong_deinit() {
    // Destroy Window
    window_destroy(s_pong_window);
}