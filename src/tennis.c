#include <pebble.h>
#include "pebble-games.h"
#include "tennis.h"

#define HEIGHT 100
#define HORIZ_VEL 3
#define PADDLE_HEIGHT 20
#define PADDLE_DISTANCE 10
#define BALL_RADIUS 2
#define WAIT 40
#define CPU_VEL 3
#define PERSON_VEL 3
#define MAX_Y_VEL 5
#define WINNING_SCORE 7
#if defined(PBL_ROUND)
  #define TOP_MARGIN (180-HEIGHT)/2
  #define WIDTH 160
  #define LEFT_MARGIN 10
  #define SCORE_MARGIN 10
  #define SCORE_HEIGHT 20
#else
  #define TOP_MARGIN 10
  #define WIDTH 144
  #define LEFT_MARGIN 0
  #define SCORE_MARGIN 10
  #define SCORE_HEIGHT 20
#endif
  
static Window *s_tennis_window;
static Layer *s_tennis_layer;

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
static short win = 0;
static short lose = 0;

static short person_score = 0;
static short cpu_score = 0;

static void draw_tennis(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_text_color(ctx, GColorWhite);
  
  // scores
  char person_score_char[2] = " ";
  person_score_char[0] = (char)(((int)'0')+person_score);
  graphics_draw_text(ctx, person_score_char, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0,HEIGHT+SCORE_MARGIN,LEFT_MARGIN+WIDTH/2,SCORE_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  
  char cpu_score_char[2] = " ";
  cpu_score_char[0] = (char)(((int)'0')+cpu_score);
  graphics_draw_text(ctx, cpu_score_char, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(LEFT_MARGIN+WIDTH/2,HEIGHT+SCORE_MARGIN,LEFT_MARGIN+WIDTH/2,SCORE_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  
  // boundaries
  graphics_draw_line(ctx, GPoint(LEFT_MARGIN,TOP_MARGIN), GPoint(LEFT_MARGIN+WIDTH,TOP_MARGIN));
  graphics_draw_line(ctx, GPoint(LEFT_MARGIN, HEIGHT+TOP_MARGIN), GPoint(LEFT_MARGIN+WIDTH, HEIGHT+TOP_MARGIN));
  
  // ball
  graphics_fill_circle(ctx, GPoint(LEFT_MARGIN+ball_x_pos, ball_y_pos+TOP_MARGIN), BALL_RADIUS);
  
  // paddles
  graphics_draw_line(ctx, GPoint(LEFT_MARGIN+PADDLE_DISTANCE, person_y_pos+TOP_MARGIN), GPoint(LEFT_MARGIN+PADDLE_DISTANCE, person_y_pos+PADDLE_HEIGHT+TOP_MARGIN));
  graphics_draw_line(ctx, GPoint(LEFT_MARGIN+WIDTH-PADDLE_DISTANCE, cpu_y_pos+TOP_MARGIN), GPoint(LEFT_MARGIN+WIDTH-PADDLE_DISTANCE, cpu_y_pos+PADDLE_HEIGHT+TOP_MARGIN));
  
  // win/lose
  if (win==1) {
    graphics_draw_text(ctx, "WIN", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(LEFT_MARGIN,TOP_MARGIN+HEIGHT/2,WIDTH,SCORE_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  } else if (lose==1) {
    graphics_draw_text(ctx, "LOSE", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(LEFT_MARGIN,TOP_MARGIN+HEIGHT/2,WIDTH,SCORE_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

static void stop() {
  paused = 1;
  app_timer_cancel(timer);
}

static void move() {
  if (ball_y_pos>(cpu_y_pos+PADDLE_HEIGHT/2)+5) {
    cpu_vel = CPU_VEL;
  } else if (ball_y_pos<(cpu_y_pos+PADDLE_HEIGHT/2)-5) {
    cpu_vel = -CPU_VEL;
  } else {
    cpu_vel = 0;
  }
  if (person_y_pos>=HEIGHT-PADDLE_HEIGHT && person_vel>0) {
    person_vel = 0;
  }
  if (person_y_pos<=0 && person_vel<0) {
    person_vel = 0;
  }
  if (cpu_y_pos>=HEIGHT-PADDLE_HEIGHT && cpu_vel>0) {
    cpu_vel = 0;
  }
  if (cpu_y_pos<=0 && cpu_vel<0) {
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
  } else if (ball_x_pos<0) {
    ball_x_vel = HORIZ_VEL;
    ball_y_vel = rand() % MAX_Y_VEL;
    ball_x_pos = WIDTH/2;
    ball_y_pos = HEIGHT/2;
    cpu_score += 1;
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
  timer = app_timer_register(WAIT, move_with_timer, NULL);
  if (person_score == WINNING_SCORE) {
    win = 1;
    stop();
  }
  if (cpu_score == WINNING_SCORE) {
    lose = 1;
    stop();
  }
  layer_mark_dirty(s_tennis_layer);
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
  win = 0;
  lose = 0;
  person_score = 0;
  cpu_score = 0;
  timer = app_timer_register(WAIT, move_with_timer, NULL);
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

static void pause(ClickRecognizerRef recognizer, void *context) {
  if (win==0 && lose==0) {
    if (paused==0) {
    } else {
      paused = 0;
      timer = app_timer_register(WAIT, move_with_timer, NULL);
    }
  } else {
    reset_game();
  }
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
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, reset_game, NULL);
}

static void tennis_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_tennis_layer = layer_create(bounds);
  
  layer_set_update_proc(s_tennis_layer, draw_tennis);
  layer_add_child(window_get_root_layer(s_tennis_window), s_tennis_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) tennis_config_provider);
  
  ball_y_vel = rand() % MAX_Y_VEL;
  timer = app_timer_register(WAIT, move_with_timer, NULL);
}

static void tennis_window_unload(Window *window) {
  layer_destroy(s_tennis_layer);
  window_destroy(s_tennis_window);
  s_tennis_window = NULL;
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