#include <pebble.h>
#include "blackjack.h"
#include "cards.h"
  
#define CARD_HEIGHT 40
#define CARD_WIDTH 20
#define ANIMATION_TIME 800
#define BLACKJACK_PERSON_SCORE_KEY 122
#define BLACKJACK_DEALER_SCORE_KEY 428
#define SCORE_HEIGHT 22
#define SCORE_WIDTH 30
#define MARGIN 0

#if defined(PBL_ROUND)
  #define CENTER_MARGIN 5
#elif defined(PBL_SDK_3)
  #define CENTER_MARGIN (168/2-2*MARGIN-CARD_HEIGHT-SCORE_HEIGHT)
#else
  #define CENTER_MARGIN (152/2-2*MARGIN-CARD_HEIGHT-SCORE_HEIGHT)
#endif
  
static Window *s_blackjack_window;
static Layer *s_blackjack_layer;
static ActionBarLayer *s_blackjack_actionbar;

static GBitmap *hearts_icon;
static GBitmap *clubs_icon;
static GBitmap *diamonds_icon;
static GBitmap *spades_icon;
static GBitmap *hit_icon;
static GBitmap *done_icon;

static short person_size=2;
static short dealer_size=2;
static short deck_size=48;

static short dealer_show = 0;
static short person_done = 0;

static short person_score = 0;
static short dealer_score = 0;

static short person_value = 0;
static short dealer_value = 0;

static AppTimer *timer;

static short *person;
static short *dealer;
static short *deck;

void init_to_nil() {
  person = malloc(11*sizeof(short));
  dealer = malloc(11*sizeof(short));
  deck = malloc(48*sizeof(short));
  short m;
  for (m=0;m<11;m++) {
    *(person+m) = -1;
    *(dealer+m) = -1;
  }
  for (m=0;m<48;m++) {
    *(deck+m) = -1;
  }
}

void shuffle(short *array) {
  size_t i;
  for (i = 0; i < 52; i++) {
    size_t j = (rand() % (52-i)) + i;
    int t = array[j];
    array[j] = array[i];
    array[i] = t;
  }
}

static short get_value(short *cards, short length) {
  short m, value_of_not_aces=0, mutations=1;
  short *values;
  for (m=0;m<length;m++) {
    if ((*(cards+m)/4)==0) {
      mutations++;
    } else {
      if (*(cards+m)/4>9) {
        value_of_not_aces += 10;
      } else {
        value_of_not_aces += *(cards+m)/4+1;
      }
    }
  }
  values = malloc(mutations*sizeof(short));
  for (m=0;m<mutations;m++) {
    *(values+m) = value_of_not_aces+mutations-1+m*10;
  }
  short done = 0, value = 0;
  for (m=mutations-1;m>=0;m--) {
    if (*(values+m) < 22 && done == 0) {
      value = *(values+m);
      done = 1;
    }
  }
  if (done == 0) {
    value = *(values);
  }
  free(values);
  return value;
}

static void reset_all() {
  person_size = 2;
  dealer_size = 2;
  deck_size = 48;
  dealer_show = 0;
}

static void free_cards() {
  free(person);
  free(dealer);
  free(deck);
}

static void reset_cards() {
  init_to_nil();
  short cards[52];
  short m;
  for (m=0;m<52;m++) {
    cards[m] = m; // 0 is 2 hearts, then 2dia, 2club, 2spade, 3heart, etc. m%4 == suit, m/4 = number
  }
  shuffle(cards);
  
  for (m=0;m<2;m++) {
    *(person +m) = cards[m];
  }
  for (m=0;m<2;m++) {
    *(dealer +m) = cards[m+2];
  }
  for (m=0;m<48;m++) {
    *(deck +m) = cards[m+4];
  }
  
  person_value = get_value(person, person_size);
  dealer_value = get_value(dealer, dealer_size);
}

static void add_card(short *cards, short person) {
  short length;
  if (person==1) {
    if (person_done == 1) {
      return;
    }
    length = person_size;
    person_size++;
  } else {
    length = dealer_size;
    dealer_size++;
  }
  deck_size--;
  *(cards+length) = *(deck+deck_size);
}

static void reset_game() {
  reset_all();
  free_cards();
  reset_cards();
  person_done = 0;
  layer_mark_dirty(s_blackjack_layer);  
}

static void pick_winner_and_restart() {
  short winner;
  if (person_value>21) {
    winner = 0; // dealer
  } else if (dealer_value>21) {
    winner = 1; // person
  } else if (person_value>dealer_value) {
    winner = 1;
  } else {
    winner = 0;
  }
  if (winner == 1) {
    person_score++;
  } else {
    dealer_score++;
  }
  timer = app_timer_register(ANIMATION_TIME, reset_game, NULL);
}

static void dealer_decide() {
  if (dealer_value<17) {
    add_card(dealer, 0);
    dealer_value = get_value(dealer, dealer_size);
    layer_mark_dirty(s_blackjack_layer);
    timer = app_timer_register(ANIMATION_TIME, dealer_decide, NULL);
  } else {
    pick_winner_and_restart();
  }
}

static void done() {
  dealer_show = 1;
  layer_mark_dirty(s_blackjack_layer);
  timer = app_timer_register(ANIMATION_TIME, dealer_decide, NULL);
}

static void up_handler() {
  if (person_done == 1) {
    return;
  }
  add_card(person, 1);
  person_value = get_value(person, person_size);
  if (person_value>21) {
    person_done = 1;
    done();
  }
  layer_mark_dirty(s_blackjack_layer);
}

static void down_handler() {
  person_done = 1;
  done();
}

static void select_handler() {
  app_timer_cancel(timer);
  reset_all();
  free_cards();
  reset_cards();
  person_done = 0;
  layer_mark_dirty(s_blackjack_layer);
}

static void back_handler() {
  window_stack_pop(true);
}

static void reset_score() {
  select_handler();
  person_score = 0;
  dealer_score = 0;
}

void blackjack_config_provider(Window *window) {
  // set click listeners
  window_single_click_subscribe(BUTTON_ID_UP, up_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, reset_score, NULL);
}

static void draw_card(short card, short x, short y, short shown, GContext *ctx) {// stacks are 0-7, 8-11 for aces, 12 for pile
  if (shown == 0) {
    graphics_fill_rect(ctx, GRect(x,y,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
    return;
  }
  short card_number = card/4;
  char card_text[3];
  if (card_number == 0) {
    strcpy(card_text, "A");
  } else if (card_number == 10) {
    strcpy(card_text, "J");
  } else if (card_number == 11) {
    strcpy(card_text, "Q");
  } else if (card_number == 12) {
    strcpy(card_text, "K");
  } else if (card_number == 9) {
    strcpy(card_text, "10");
  } else {
    card_text[0] = (char)(((int)'0')+card_number+1);
    card_text[1] = '\0';
  }
  short suit = card%4;
  #if defined(PBL_COLOR)
    if (suit==0 || suit==1) {
      graphics_context_set_text_color(ctx, GColorRed);
    } else {
      graphics_context_set_text_color(ctx, GColorBlack);
    }
  #else
    graphics_context_set_text_color(ctx, GColorBlack);
  #endif
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(x,y,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_draw_round_rect(ctx, GRect(x,y,CARD_WIDTH,CARD_HEIGHT), 4);
  graphics_draw_text(ctx, card_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(x,y,CARD_WIDTH,20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  GBitmap *bmp_to_draw;
  if (suit==0) {
    bmp_to_draw = hearts_icon;
  } else if (suit==1) {
    bmp_to_draw = diamonds_icon;
  } else if (suit==2) {
    bmp_to_draw = clubs_icon;
  } else {
    bmp_to_draw = spades_icon;
  }
  graphics_draw_bitmap_in_rect(ctx, bmp_to_draw, GRect(x+3,y+CARD_HEIGHT/2+3, 14, 14));
  return;
}

static void draw_score(GContext *ctx, short score, short x, short y) {
  graphics_context_set_text_color(ctx, GColorBlack);
  char score_text[5] = "";
  if (score >= 1000) {
    score_text[0] = (char)(((int)'0')+score/1000);
  } else {
    score_text[0] = ' ';
  }
  if (score >= 100) {
    score_text[1] = (char)(((int)'0')+score/100%10);
  } else {
    score_text[1] = ' ';
  }
  if (score >= 10) {
    score_text[2] = (char)(((int)'0')+score/10%10);
  } else {
    score_text[2] = ' ';
  }
  score_text[3] = (char)(((int)'0')+score%10);
  score_text[4] = '\0';
  graphics_draw_text(ctx, score_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(x,y,SCORE_WIDTH,SCORE_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void draw_scores(GContext *ctx) {
  GSize size = layer_get_bounds(s_blackjack_layer).size;
  draw_score(ctx, person_score, size.w-ACTION_BAR_WIDTH-SCORE_WIDTH, size.h/2);
  draw_score(ctx, dealer_score, size.w-ACTION_BAR_WIDTH-SCORE_WIDTH, size.h/2 - SCORE_HEIGHT);
  draw_score(ctx, person_value, (size.w-ACTION_BAR_WIDTH-SCORE_WIDTH)/2, size.h/2+CENTER_MARGIN+CARD_HEIGHT+MARGIN);
  if (dealer_show==1) {
    draw_score(ctx, dealer_value, (size.w-ACTION_BAR_WIDTH-SCORE_WIDTH)/2, size.h/2-CENTER_MARGIN-CARD_HEIGHT-MARGIN-SCORE_HEIGHT);
  }
}

static void draw_blackjack(Layer *layer, GContext *ctx) {
  //120 2140 4160 6281 83102 103122 123142
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);

  GSize size = layer_get_bounds(s_blackjack_layer).size;
  
  draw_scores(ctx);
  
  short m;
  for (m=0;m<person_size;m++) {
    if (person_size<7) {
      draw_card(*(person+m), size.w/2-9*person_size+18*m-ACTION_BAR_WIDTH/2, size.h/2+CENTER_MARGIN, 1, ctx);
    } else {
      draw_card(*(person+m), size.w/2-7*person_size+14*m-ACTION_BAR_WIDTH/2, size.h/2+CENTER_MARGIN, 1, ctx);
    }
  }
  for (m=0;m<dealer_size;m++) {
    short show = 1;
    if (dealer_show==0&&m==0) {
      show = 0;
    }
    if (dealer_size<7) {
      draw_card(*(dealer+m), size.w/2-9*dealer_size+18*m-ACTION_BAR_WIDTH/2, size.h/2-CENTER_MARGIN-CARD_HEIGHT, show, ctx);
    } else {
      draw_card(*(dealer+m), size.w/2-7*dealer_size+14*m-ACTION_BAR_WIDTH/2, size.h/2-CENTER_MARGIN-CARD_HEIGHT, show, ctx);
    }
  }
}

static void load_bitmaps() {
  hearts_icon = gbitmap_create_with_resource(RESOURCE_ID_HEARTS_ICON);
  diamonds_icon = gbitmap_create_with_resource(RESOURCE_ID_DIAMONDS_ICON);
  clubs_icon = gbitmap_create_with_resource(RESOURCE_ID_CLUBS_ICON);
  spades_icon = gbitmap_create_with_resource(RESOURCE_ID_SPADES_ICON);
  hit_icon = gbitmap_create_with_resource(RESOURCE_ID_HIT_ICON);
  done_icon = gbitmap_create_with_resource(RESOURCE_ID_DONE_ICON);
}

static void destroy_bitmaps() {
  gbitmap_destroy(hearts_icon);
  gbitmap_destroy(diamonds_icon);
  gbitmap_destroy(clubs_icon);
  gbitmap_destroy(spades_icon);
  gbitmap_destroy(hit_icon);
  gbitmap_destroy(done_icon);
}

static void blackjack_window_load(Window *window) {
  load_bitmaps();
  
  srand((unsigned) time(NULL));
  reset_cards();
  
  if (persist_exists(BLACKJACK_PERSON_SCORE_KEY)) {
    person_score = persist_read_int(BLACKJACK_PERSON_SCORE_KEY);
  }
  if (persist_exists(BLACKJACK_DEALER_SCORE_KEY)) {
    dealer_score = persist_read_int(BLACKJACK_DEALER_SCORE_KEY);
  }
  
  person_value = get_value(person, person_size);
  dealer_value = get_value(dealer, dealer_size);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  
  s_blackjack_layer = layer_create(bounds);
  
  s_blackjack_actionbar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_blackjack_actionbar, window);
  
  action_bar_layer_set_icon(s_blackjack_actionbar, BUTTON_ID_UP, hit_icon);
  action_bar_layer_set_icon(s_blackjack_actionbar, BUTTON_ID_DOWN, done_icon);
  
  layer_set_update_proc(s_blackjack_layer, draw_blackjack);
  layer_add_child(window_get_root_layer(window), s_blackjack_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) blackjack_config_provider);
  window_set_background_color(window, GColorWhite);
}

static void blackjack_window_unload(Window *window) {
  persist_write_int(BLACKJACK_PERSON_SCORE_KEY, person_score);
  persist_write_int(BLACKJACK_DEALER_SCORE_KEY, dealer_score);
  app_timer_cancel(timer);
  destroy_bitmaps();
  free_cards();
  layer_destroy(s_blackjack_layer);
  action_bar_layer_destroy(s_blackjack_actionbar);
  window_destroy(s_blackjack_window);
}

void blackjack_init() {
  // Create main Window element and assign to pointer
  s_blackjack_window = window_create();
  
  window_set_background_color(s_blackjack_window, GColorWhite);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_blackjack_window, (WindowHandlers) {
    .load = blackjack_window_load,
    .unload = blackjack_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_blackjack_window, true);
}