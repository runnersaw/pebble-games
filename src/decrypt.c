#include <pebble.h>
#include "pebble-games.h"
#include "decrypt.h"

#if defined(PBL_COLOR)
#define RED 0
#define ORANGE 1
#define YELLOW 2
#define GREEN 3
#define BLUE 4
#define PURPLE 5
#define NUM_COLORS 6
#define NUM_GUESSES 6

#define MAX_GUESSES 10
#if defined(PBL_ROUND)
  #define WIDTH 75
  #define HEIGHT 120
  #define COLORS_MARGIN 30
  #define LEFT_MARGIN 25
  #define TOP_MARGIN 20
  #define INDICATOR_WIDTH 60
#else
  #define WIDTH 80
  #define HEIGHT 120
  #define COLORS_MARGIN 15
  #define LEFT_MARGIN 10
  #define TOP_MARGIN 5
  #define INDICATOR_WIDTH 50
#endif
#define BORDER 4
#define COVER_HEIGHT 10
#define RADIUS 3
#define LARGE_RADIUS 4
#define INDICATOR_RADIUS 2
#define WIN_TEXT_HEIGHT 30
#define COLORS_WIDTH 144

static Window *s_decrypt_window;
static Layer *s_decrypt_layer;

static short *target;
static short *guesses;
static short selected_color = 0;
static short *current_guess;
static short no_guesses = 0;
static short current_guess_no_chosen = 0;
static short lose = 0;
static short win = 0;

static void init_target() {
  short m;
  srand(time(NULL));
  for (m=0;m<NUM_GUESSES;m++) {
    *(target+m) = rand()%NUM_COLORS;
  }
}

static short right_color_right_place(short *target, short *guesses) {
  short m, num=0;
  for (m=0;m<NUM_GUESSES;m++) {
    if (*(target+m) == *(guesses+m)) {
      num++;
    }
  }
  return num;
}

static short right_color_wrong_place(short *target, short *guesses) {
  short num_color_target=0, num_color_guess=0, m, n, value=0;
  for (m=0;m<NUM_COLORS;m++) {
    num_color_target=0;
    num_color_guess=0;
    for (n=0;n<NUM_GUESSES;n++) {
      if (*(target+n) == m) {
        num_color_target++;
      }
      if (*(guesses+n) == m) {
        num_color_guess++;
      }
    }
    if (num_color_target<=num_color_guess) {
      value += num_color_target;
    } else {
      value += num_color_guess;
    }
  }
  value -= right_color_right_place(target, guesses);
  return value;
}

static void remove_guess() {
  current_guess_no_chosen -= 1;
  *(current_guess+current_guess_no_chosen) = -1;
}

static void add_guess(short color) {
  *(current_guess+current_guess_no_chosen) = color;
  current_guess_no_chosen += 1;
  if (current_guess_no_chosen == NUM_GUESSES) {
    if (right_color_right_place(target, current_guess)==NUM_GUESSES) {
      win = 1;
    }
    short m;
    for (m=0;m<NUM_GUESSES;m++) {
      *(guesses+NUM_GUESSES*no_guesses+m) = *(current_guess+m);
      *(current_guess+m) = -1;
    }
    current_guess_no_chosen = 0;
    no_guesses += 1;
    if (no_guesses == MAX_GUESSES) {
      if (win==0) {
        lose = 1;
      }
    }
  }
  layer_mark_dirty(s_decrypt_layer);
}

static void reset() {
  init_target();
  short m;
  for (m=0;m<MAX_GUESSES*NUM_GUESSES;m++) {
    *(guesses+m) = -1;
  }
  current_guess_no_chosen = 0;
  for (m=0;m<NUM_GUESSES;m++) {
    *(current_guess+m) = -1;
  }
  no_guesses = 0;
  lose = 0;
  win = 0;
}

static void set_fill_color(GContext *ctx, short color) {
  if (color == RED) {
    graphics_context_set_fill_color(ctx, GColorRed);
  }
  if (color == ORANGE) {
    graphics_context_set_fill_color(ctx, GColorOrange);
  }
  if (color == YELLOW) {
    graphics_context_set_fill_color(ctx, GColorYellow);
  }
  if (color == GREEN) {
    graphics_context_set_fill_color(ctx, GColorDarkGreen);
  }
  if (color == BLUE) {
    graphics_context_set_fill_color(ctx, GColorBlue);
  }
  if (color == PURPLE) {
    graphics_context_set_fill_color(ctx, GColorPurple);
  }
}

static void draw_decrypt(Layer *layer, GContext *ctx) {
  GSize size = layer_get_bounds(layer).size;

  graphics_context_set_fill_color(ctx, GColorRajah);
  graphics_fill_rect(ctx, GRect(LEFT_MARGIN, TOP_MARGIN, WIDTH, HEIGHT), 0, GCornerNone);
  short m, n;
  for (m=0;m<no_guesses;m++) {
    short right_right = right_color_right_place(target, (guesses+NUM_GUESSES*m));
    short right_wrong = right_color_wrong_place(target, (guesses+NUM_GUESSES*m));
    for (n=0;n<NUM_GUESSES;n++) {
      set_fill_color(ctx, *(guesses+NUM_GUESSES*m+n));
      graphics_fill_circle(ctx, GPoint(LEFT_MARGIN+WIDTH/(NUM_GUESSES+1)*(n+1), TOP_MARGIN+COVER_HEIGHT+BORDER+(m+1)*(HEIGHT-COVER_HEIGHT-BORDER)/(MAX_GUESSES+2)), RADIUS);
    }
    for (n=0;n<right_right;n++) {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_circle(ctx, GPoint(LEFT_MARGIN+WIDTH+INDICATOR_WIDTH/(NUM_GUESSES+1)*(n+1), TOP_MARGIN+COVER_HEIGHT+BORDER+(m+1)*(HEIGHT-COVER_HEIGHT-BORDER)/(MAX_GUESSES+2)), INDICATOR_RADIUS);
    }
    for (n=0;n<right_wrong;n++) {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_circle(ctx, GPoint(LEFT_MARGIN+WIDTH+INDICATOR_WIDTH/(NUM_GUESSES+1)*(right_right+n+1), TOP_MARGIN+COVER_HEIGHT+BORDER+(m+1)*(HEIGHT-COVER_HEIGHT-BORDER)/(MAX_GUESSES+2)), INDICATOR_RADIUS);
    }
  }
  for (m=0;m<current_guess_no_chosen;m++) {
    set_fill_color(ctx, *(current_guess+m));
    graphics_fill_circle(ctx, GPoint(LEFT_MARGIN+WIDTH/(NUM_GUESSES+1)*(m+1), TOP_MARGIN+HEIGHT-(HEIGHT-COVER_HEIGHT-BORDER)/(MAX_GUESSES+2)), RADIUS);
  }
  for (m=0;m<NUM_COLORS;m++) {
    short radius = 5;
    if (m==selected_color) {
      radius = 7;
    }
    set_fill_color(ctx, m);
    graphics_fill_circle(ctx, GPoint((size.w-COLORS_WIDTH)/2 + COLORS_WIDTH/(NUM_COLORS+1)*(m+1), size.h-COLORS_MARGIN), radius);
  }
  if (win==0 && lose==0) { 
    // draw the opponent's as covered
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(LEFT_MARGIN+BORDER, TOP_MARGIN+BORDER, WIDTH-2*BORDER, COVER_HEIGHT), 0, GCornerNone);
  } else {
    // draw the opponents sequence
    for (short m=0;m<NUM_GUESSES;m++) {
      set_fill_color(ctx, *(target+m));
      graphics_fill_circle(ctx, GPoint(LEFT_MARGIN+WIDTH/(NUM_GUESSES+1)*(m+1), TOP_MARGIN+BORDER+COVER_HEIGHT/2), LARGE_RADIUS);
    }
    for (m=0;m<NUM_GUESSES;m++) {
      *(target+m) = rand()%NUM_COLORS;
    }

    if (win==1) {
      graphics_context_set_text_color(ctx, GColorBlack);
      graphics_draw_text(ctx, "WIN", fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(0,(size.h-WIN_TEXT_HEIGHT)/2,size.w,WIN_TEXT_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL); 
    }
    if (lose==1) {
      graphics_context_set_text_color(ctx, GColorBlack);
      graphics_draw_text(ctx, "LOSE", fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(0,(size.h-WIN_TEXT_HEIGHT)/2,size.w,WIN_TEXT_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL); 
    }
  }
}

static void up_handler() {
  selected_color--;
  if (selected_color<0) {
    selected_color = NUM_COLORS-1;
  }
  layer_mark_dirty(s_decrypt_layer);
}

static void down_handler() {
  selected_color++;
  if (selected_color>=NUM_COLORS) {
    selected_color = 0;
  }
  layer_mark_dirty(s_decrypt_layer);
}

static void select_handler() {
  if (lose == 1 || win == 1) {
    reset();
  } else {
    add_guess(selected_color);
  }
  layer_mark_dirty(s_decrypt_layer);
}

static void select_long_handler() {
  reset();
  layer_mark_dirty(s_decrypt_layer);
}

static void back_handler() {
  if (current_guess_no_chosen == 0) {
    window_stack_pop(s_decrypt_window);
  } else {
    remove_guess();
  }
  layer_mark_dirty(s_decrypt_layer);
}

static void decrypt_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_decrypt_layer = layer_create(bounds);
  layer_set_update_proc(s_decrypt_layer, draw_decrypt);
  layer_add_child(window_get_root_layer(window), s_decrypt_layer);

  window_set_background_color(window, GColorRajah);

  target = malloc(NUM_GUESSES*sizeof(short));
  current_guess = malloc(NUM_GUESSES*sizeof(short));
  guesses = malloc(MAX_GUESSES*NUM_GUESSES*sizeof(short));

  init_target();
}

static void decrypt_unload(Window *window) {
  layer_destroy(s_decrypt_layer);

  free(target);
  free(guesses);
  free(current_guess);
  
  window_destroy(s_decrypt_window);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, select_long_handler, NULL);
}
  
void decrypt_init() {
  s_decrypt_window = window_create();
  
  window_set_click_config_provider(s_decrypt_window, click_config_provider);
  window_set_window_handlers(s_decrypt_window, (WindowHandlers) {
    .load = decrypt_load,
    .unload = decrypt_unload,
  });
  
  const bool animated = true;
  window_stack_push(s_decrypt_window, animated);
}
#endif