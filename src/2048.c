#include <pebble.h>
#include "2048.h"

#if defined(PBL_ROUND)
  #define WIDTH 130
#else
  #define WIDTH 140
#endif
#define RIGHT 0
#define UP 1
#define LEFT 2
#define DOWN 3
  
#define TWO048_BOARD_STATE_KEY 34

static Window *s_2048_window;
static Layer *s_2048_layer;

//static short two048_high_score;
static short *two048_game_state;
//static short two048_game_score;
static GBitmap *background;

static short get_piece(short x, short y) {
  return *(two048_game_state+4*y+x);
}

static void add_new_number() {
  short valid = 0;
  srand(time(NULL));
  while (valid==0) {
    short x = rand()%4;
    short y = rand()%4;
    short value = rand()%10;
    if (get_piece(x,y)==0) {
      valid = 1;
      if (value>0) {
        value = 1;
      } else {
        value = 2;
      }
      *(two048_game_state+4*y+x) = value;
    }
  }
  layer_mark_dirty(s_2048_layer);
}

static void init_board_state() {
  short k;
  for (k=0;k<16;k++) {
    *(two048_game_state+k) = 0;
  }
  add_new_number();
  add_new_number();
}

static void set_board_state(short state[16]) {
  for (short i=0;i<16;i++) {
    *(two048_game_state+i) = state[i];
  }
}

static short is_full() {
  short m;
  for (m=0;m<16;m++) {
    if (*(two048_game_state+m)==0) {
      return 0;
    }
  }
  return 1;
}

static short collapse(short direction) {
  short m, n, can_merge, prev_value, index, value, collapsed = 0;
  short out[4];
  for (m=0;m<4;m++) {
    for (n=0;n<4;n++) {
      out[n] = 0;
    }
    index = 0;
    prev_value = 0;
    can_merge = 1;
    for (n=0;n<4;n++) {
      if (direction==UP) {
        value = *(two048_game_state+4*n+m);
      } else if (direction==RIGHT) {
        value = *(two048_game_state+(3-n)+4*m);
      } else if (direction==LEFT) {
        value = *(two048_game_state+n+4*m);
      } else { // DOWN
        value = *(two048_game_state+4*(3-n)+m);
      }
      if (value!=0) {
        if (value==prev_value && can_merge==1) {
          // case that it should merge, if prev didn't merge and values are equal
          out[index-1] = out[index-1] + 1;
          can_merge = 0;
        } else {
          // case that it should add
          out[index] = value;
          index++;
          can_merge = 1;
          prev_value = value;
        }
      }
    }
    for (n=0;n<4;n++) {
      if (direction==UP) {
        if (*(two048_game_state+4*n+m) != out[n]) {
          collapsed = 1;
          *(two048_game_state+4*n+m) = out[n];
        }
      } else if (direction==RIGHT) {
        if (*(two048_game_state+(3-n)+4*m) != out[n]) {
          collapsed = 1;
          *(two048_game_state+(3-n)+4*m) = out[n];
        }
      } else if (direction==LEFT) {
        if (*(two048_game_state+n+4*m) != out[n]) {
          collapsed = 1;
          *(two048_game_state+n+4*m) = out[n];
        }
      } else {
        if (*(two048_game_state+4*(3-n)+m) != out[n]) {
          collapsed = 1;
          *(two048_game_state+4*(3-n)+m) = out[n];
        }
      }
    }
  }
  return collapsed;
}

#if defined(PBL_COLOR)
static GColor get_color_for_score(short score) {
  GColor color = GColorWhite;
  if (score == 1) {
    color = GColorPastelYellow;
  } else if (score==2) {
    color = GColorIcterine;
  } else if (score==3) {
    color = GColorYellow;
  } else if (score==4) {
    color = GColorRajah;
  } else if (score==5) {
    color = GColorChromeYellow;
  } else if (score==6) {
    color = GColorOrange;
  } else if (score==7) {
    color = GColorRed;
  } else if (score==8) {
    color = GColorDarkCandyAppleRed;
  } else if (score==9) {
    color = GColorJazzberryJam;
  } else if (score==10) {
    color = GColorPurple;
  } else if (score==11) {
    color = GColorIndigo;
  } else if (score==12) {
    color = GColorDukeBlue;
  } else if (score==13) {
    color = GColorCobaltBlue;
  } else if (score==14) {
    color = GColorCadetBlue;
  } else if (score==15) {
    color = GColorMayGreen;
  } else if (score==16) {
    color = GColorIslamicGreen;
  } else if (score==17) {
    color = GColorGreen;
  }
  return color;
}
#endif

static void draw_2048(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorBlack);
  short m,n, value;
  char text[5];

  GSize size = layer_get_bounds(layer).size;
  for (m=0;m<4;m++) {
    for (n=0;n<4;n++) {
      graphics_draw_bitmap_in_rect(ctx, background, GRect((size.w-WIDTH)/2+WIDTH/4*m, (size.w-WIDTH)/2+WIDTH/4*n, WIDTH/4-1, WIDTH/4-1));
      value = *(two048_game_state+m+4*n);
      graphics_context_set_text_color(ctx, GColorBlack);
      #if defined(PBL_COLOR)
        graphics_context_set_fill_color(ctx, get_color_for_score(value));
        graphics_fill_rect(ctx, GRect((size.w-WIDTH)/2+WIDTH/4*m, (size.w-WIDTH)/2+WIDTH/4*n, WIDTH/4-1, WIDTH/4-1), 0, GCornerNone);
      #else 
        graphics_context_set_fill_color(ctx, GColorWhite);
        if (value!=0) {
          graphics_fill_rect(ctx, GRect((size.w-WIDTH)/2+WIDTH/4*m, (size.w-WIDTH)/2+WIDTH/4*n, WIDTH/4-1, WIDTH/4-1), 0, GCornerNone);
        }
      #endif
      graphics_draw_rect(ctx, GRect((size.w-WIDTH)/2+WIDTH/4*m, (size.w-WIDTH)/2+WIDTH/4*n, WIDTH/4-1, WIDTH/4-1));
      if (value!=0) {
        int v = (int)value;
        int actual_value = (1 << v);
        if (actual_value>=1000) {
          text[0] = (char)(((int)'0')+actual_value/1000);
        } else {
          text[0] = ' ';
        }
        if (actual_value>=100) {
          text[1] = (char)(((int)'0')+(actual_value/100)%10);
        } else {
          text[1] = ' ';
        }
        if (actual_value>=10) {
          text[2] = (char)(((int)'0')+(actual_value/10)%10);
        } else {
          text[2] = ' ';
        }
        text[3] = (char)(((int)'0')+actual_value%10);
        text[4] = '\0';
        #if defined(PBL_ROUND)
          graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GRect((size.w-WIDTH)/2+WIDTH/4*m, (size.w-WIDTH)/2+WIDTH/4*n+WIDTH/8-15, WIDTH/4, 30), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
        #else
          graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect((size.w-WIDTH)/2+WIDTH/4*m, (size.w-WIDTH)/2+WIDTH/4*n+WIDTH/8-15, WIDTH/4, 30), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
        #endif
      }
    }
  }
}

static void select_handler() {
  short collapsed = collapse(RIGHT);
  if (is_full()==0 && collapsed==1) {
    add_new_number();
  }
  layer_mark_dirty(s_2048_layer);
}

static void up_handler() {
  short collapsed = collapse(UP);
  if (is_full()==0 && collapsed==1) {
    add_new_number();
  }
  layer_mark_dirty(s_2048_layer);
}

static void back_handler() {
  short collapsed = collapse(LEFT);
  if (is_full()==0 && collapsed==1) {
    add_new_number();
  }
  layer_mark_dirty(s_2048_layer);
}

static void down_handler() {
  short collapsed = collapse(DOWN);
  if (is_full()==0 && collapsed==1) {
    add_new_number();
  }
  layer_mark_dirty(s_2048_layer);
}

static void up_long_handler() {
  window_stack_pop(s_2048_window);
}

static void reset() {
  init_board_state();
  layer_mark_dirty(s_2048_layer);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, reset, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 1000, up_long_handler, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_2048_layer = layer_create(bounds);
  two048_game_state = malloc(16*sizeof(short));
  
  if (persist_exists(TWO048_BOARD_STATE_KEY)) {
    persist_read_data(TWO048_BOARD_STATE_KEY, two048_game_state, 16*sizeof(short));
  } else {
    init_board_state();
  }

  //short state[16] = {2, 3, 5, 1, 4, 9, 8, 4, 5, 3, 4, 3, 1, 0, 1, 1};
  //set_board_state(state);
  
  background = gbitmap_create_with_resource(RESOURCE_ID_GRAY_BACKGROUND);

  layer_set_update_proc(s_2048_layer, draw_2048);
  layer_add_child(window_get_root_layer(window), s_2048_layer);
}

static void window_unload(Window *window) {
  persist_write_data(TWO048_BOARD_STATE_KEY, two048_game_state, 16*sizeof(short));
  gbitmap_destroy(background);
  free(two048_game_state);
  layer_destroy(s_2048_layer);
  window_destroy(s_2048_window);
}

void two048_init(void) {
  s_2048_window = window_create();
  window_set_click_config_provider(s_2048_window, click_config_provider);
  
  window_set_background_color(s_2048_window, GColorWhite);
  
  window_set_window_handlers(s_2048_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  const bool animated = true;
  window_stack_push(s_2048_window, animated);
}