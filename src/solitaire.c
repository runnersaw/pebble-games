
#include <pebble.h>
#include "pebble-games.h"
#include "cards.h"
#include "solitaire.h"

#define SELECTING_ROW 0
#define SELECTING_CARD 1
#define SELECTING_LOC 2

#define ARROW_WIDTH 12

#if defined(PBL_ROUND)
  #define WIDTH 132
  #define HEIGHT 132
  #define CARD_HEIGHT 40
  #define CARD_WIDTH 17
  #define MARGIN 1
  #define CARD_SHOW_GAP 25
  #define NOT_SHOWN_GAP 2
  #define HIDDEN_GAP 4
  #define SUIT_SIZE 14
  #define SUIT_MARGIN 2
#else
  #define WIDTH 144
  #define HEIGHT 168
  #define CARD_HEIGHT 40
  #define CARD_WIDTH 19
  #define MARGIN 1
  #define CARD_SHOW_GAP 30
  #define NOT_SHOWN_GAP 3
  #define HIDDEN_GAP 5
  #define SUIT_SIZE 14
  #define SUIT_MARGIN 3
#endif
  
static Window *s_solitaire_window;
static Layer *s_solitaire_layer;

static GBitmap *hearts_icon;
static GBitmap *clubs_icon;
static GBitmap *diamonds_icon;
static GBitmap *spades_icon;
static GBitmap *down_arrow;

static Deck deck;
static short pile_number[13];
static short draw_pile_num;
static short num_shown[7];
static short can_move[11];
static short status = SELECTING_ROW;
static short selected_row = 0;
static short selected_to_row = 0;
static short selected_card = -1;
static short num_selected_cards = 1;
static short num_pile_resets = 0;

static short no_matches_found = 0;

static short win = 0; // 1 for win, -1 for loss

static void set_pile_number() {
  short cards = 52;
  for (short i=0; i<12; i++) {
    cards -= pile_number[i];
  }
  pile_number[12] = cards;
}

static void reset() {
  status = SELECTING_ROW;
  selected_row = 0;
  selected_to_row = 0;
  selected_card = -1;
  num_selected_cards = 1;
  num_pile_resets = 0;

  no_matches_found = 0;

  win = 0; // 1 for win, -1 for loss

  for (short i=0; i<7; i++) {
    pile_number[i] = i+1;
    num_shown[i] = 1;
  }
  for (short i=7; i<12; i++) {
    pile_number[i] = 0;
  }
  set_pile_number();
  deck = shuffle_deck(create_deck());

  layer_mark_dirty(s_solitaire_layer);
}

short get_card_number(short row, short card) {
  // row 11 behaves differently
  if (row == 11) {
    short cards = 52;
    return cards - pile_number[11] + card;
  }

  short cards = 0;
  for (int i = 0; i<row; i++) {
    cards += pile_number[i];
  }
  cards += card;
  return cards;
}

static void findNextLocation() {
  short count = 0;
  do {
    selected_to_row++;
    if (selected_to_row > 10) {
      selected_to_row = 0;
    }
    count++;
  } while (can_move[selected_to_row]==0 && count < 11); // count just in case
}

static void findPreviousLocation() {
  short count = 0;
  do {
    selected_to_row--;
    if (selected_to_row < 0) {
      selected_to_row = 10;
    }
    count++;
  } while (can_move[selected_to_row]==0 && count < 11); // count just in case
}

static void findPlaceForCard(short card, short is_last_card) {
  no_matches_found = 0;

  short value = card/4;
  short suit = card%4;
  short red;
  if (suit==0 || suit==1) {
    red = true;
  } else {
    red = false;
  }

  short moveable = 0;
  // go through rows to find cards it can move to 
  short i, card_red;
  for (i=0;i<11;i++) {
    can_move[i] = 0;
  }
  for (i=0;i<7;i++) {
    // if empty
    if (value == 0) { // don't move aces into any of the piles
      continue;
    }

    if (pile_number[i] == 0 && value == 12) {
      can_move[i] = 1;
      moveable = 1;
      continue;
    }

    if (pile_number[i] == 0) {
      continue;
    }

    // if not empty
    short card_num = get_card_number(i, 0);
    short card = deck.cards[card_num];
    short card_value = card/4;
    short card_suit = card%4;
    if (card_suit==0 || card_suit==1) {
      card_red = true;
    } else {
      card_red = false;
    }
    if (card_red != red && card_value == (value+1)) {
      can_move[i] = 1;
      moveable = 1;
    }
  }

  for (i=7; i<11; i++) {
    short is_suit_right = (suit+7 == i);
    short is_ace = ((value == 0) && (pile_number[i] == 0));
    if (is_suit_right && is_last_card) {
      if (is_ace) {
        can_move[i] = 1;
        moveable = 1;
      } else {
        if (pile_number[i] > 0) {
          short card_num = get_card_number(i, 0);
          short card_value = deck.cards[card_num]/4;
          if (value == card_value+1) {
            can_move[i] = 1;
            moveable = 1;
          }
        }
      }
    }
  }

  // if not moveable
  if (moveable==0) {
    no_matches_found = 1;
  }
}

static void check() {
  short i,j,card1,card2;
  for (i=0;i<52;i++) {
    card1 = deck.cards[i];
    for (j=i+1;j<52;j++) {
      card2 = deck.cards[j];
      if (card1 == card2) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "FAIL!! duplicate cards value %d places %d and %d", card1, i, j);
      }
    }
  }
}

static void check_win() {
  short i;
  short pile;
  for (i=0; i<4; i++) {
    pile = i+7;
    if (pile_number[pile] < 13) {
      return;
    }
  }

  // WIN!
  win = 1;
}

void move_cards(short toRow, short fromRow, short numCards) {
  short i;
  short direction = (toRow > fromRow);
  for (i=numCards-1;i>=0;i--) {
    short from = get_card_number(fromRow, i);
    short to = get_card_number(toRow, 0);
    deck = move_card(deck, from, to - direction);

    pile_number[toRow] = pile_number[toRow] + 1;
    pile_number[fromRow] = pile_number[fromRow] - 1;

    if (toRow < 7) { // don't update num_shown for aces pile or draw pile or deck
      num_shown[toRow] = num_shown[toRow] + 1;
    }
    if (fromRow < 7) { // don't update num_shown for aces pile or draw pile or deck
      num_shown[fromRow] = num_shown[fromRow] - 1;
      if (num_shown[fromRow]==0 && pile_number[fromRow]!=0) {
        num_shown[fromRow] = 1;
      }
    }
  }

  check();
  check_win();
}

static void reset_draw_pile() {
  pile_number[11] = 0;

  set_pile_number();
}

static void draw_cards() {
  short num_to_draw = 3;
  if (pile_number[12] < 3) {
    num_to_draw = pile_number[12];
  }
  pile_number[11] += num_to_draw;

  set_pile_number();
}

static void up_handler() {
  if (status == SELECTING_ROW) {
    // change row until on a row with cards
    do {
      if (selected_row == 11) {
        selected_row = 6;
        continue;
      }
      selected_row--;
      if (selected_row < 0) {
        selected_row = 12;
      }
    } while (pile_number[selected_row]==0 && selected_row != 12);
  } else if (status == SELECTING_CARD) {
    short min_card = get_card_number(selected_row, 0); // first in pile plus number not shown
    short max_card = get_card_number(selected_row, 0) + num_shown[selected_row] - 1; // min plus number shown
    if (selected_row == 11) { // handles num_shown and always should be one card selectable
      max_card = min_card;
    }
    selected_card++;
    if (selected_card>max_card) {
      selected_card = min_card;
    }
    num_selected_cards = selected_card - min_card + 1;
  } else if (status == SELECTING_LOC) {
    findPreviousLocation();
  }
  layer_mark_dirty(s_solitaire_layer);
}


static void down_handler() {
  if (status == SELECTING_ROW) {
    do {
      if (selected_row == 12) {
        selected_row = 0;
        continue;
      }
      selected_row++;
      if (selected_row == 7) {
        selected_row = 11;
      }
    } while (pile_number[selected_row]==0 && selected_row != 12);
  } else if (status == SELECTING_CARD) {
    short min_card = get_card_number(selected_row, 0); // first in pile plus number not shown
    short max_card = get_card_number(selected_row, 0) + num_shown[selected_row] - 1; // min plus number shown
    if (selected_row == 11) {
      max_card = min_card;
    }
    selected_card--;
    if (selected_card<min_card) {
      selected_card = max_card;
    }
    num_selected_cards = selected_card - min_card + 1;
  } else if (status == SELECTING_LOC) {
    findNextLocation();
  }
  layer_mark_dirty(s_solitaire_layer);
}

static void select_handler() {
  if (status == SELECTING_ROW) {
    if (selected_row == 12) {
      if (pile_number[12] == 0) {
        reset_draw_pile();
      } else {
        draw_cards();
      }
    } else if (selected_row == 11) { // don't do num_shown thing for 11
      selected_card = get_card_number(selected_row, 0); // first in pile plus number not shown
      num_selected_cards = 1;
      status = SELECTING_CARD;
    } else {
      selected_card = get_card_number(selected_row, 0) + num_shown[selected_row] - 1; // first in pile plus number not shown
      num_selected_cards = num_shown[selected_row];
      status = SELECTING_CARD;
    }
  } else if (status == SELECTING_CARD) {
    short is_last_card = 0;
    if (get_card_number(selected_row, 0) == selected_card) {
      is_last_card = 1;
    }
    findPlaceForCard(deck.cards[selected_card], is_last_card);
    if (no_matches_found == 0) {
      findNextLocation();
      status = SELECTING_LOC;
    }
  } else if (status == SELECTING_LOC) {
    // move cards here
    move_cards(selected_to_row, selected_row, num_selected_cards);
    status = SELECTING_ROW;
  }
  layer_mark_dirty(s_solitaire_layer);
}

static void back_handler() {
  if (status == SELECTING_ROW) {
    window_stack_pop(true);
  } else if (status == SELECTING_CARD) {
    no_matches_found = 0;
    status = SELECTING_ROW;
  } else if (status == SELECTING_LOC) {
    status = SELECTING_CARD;
  }
  layer_mark_dirty(s_solitaire_layer);
}

void solitaire_config_provider(Window *window) {
  // set click listeners
  window_single_click_subscribe(BUTTON_ID_UP, up_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, reset, NULL);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_handler);
}

static void draw_card(short card, short x, short y, short shown, short highlight, GContext *ctx) {// stacks are 0-7, 8-11 for aces, 12 for pile
  if (shown == 0) {
    #if defined(PBL_COLOR)
    graphics_context_set_fill_color(ctx, GColorDukeBlue);
    #else
    graphics_context_set_fill_color(ctx, GColorBlack);
    #endif
    graphics_fill_rect(ctx, GRect(x,y,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_round_rect(ctx, GRect(x,y,CARD_WIDTH,CARD_HEIGHT), 4);
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
  #if defined(PBL_COLOR)
  if (highlight) {
    graphics_context_set_fill_color(ctx, GColorYellow);
    graphics_fill_rect(ctx, GRect(x,y,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
  } else {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(x,y,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
  }
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_draw_round_rect(ctx, GRect(x,y,CARD_WIDTH,CARD_HEIGHT), 4);
  #else
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(x,y,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
  if (highlight) {
    graphics_context_set_stroke_width(ctx, 2);
  } else {
    graphics_context_set_stroke_width(ctx, 1);
  }
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_draw_round_rect(ctx, GRect(x,y,CARD_WIDTH,CARD_HEIGHT), 4);
  graphics_context_set_stroke_width(ctx, 1);
  #endif
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
  graphics_draw_bitmap_in_rect(ctx, bmp_to_draw, GRect(x+SUIT_MARGIN,y+CARD_HEIGHT/2+SUIT_MARGIN, SUIT_SIZE, SUIT_SIZE));
  return;
}

static void draw_solitaire(Layer *layer, GContext *ctx) {
  GSize size = layer_get_bounds(layer).size;
  int left = (size.w - WIDTH)/2;
  int top = (size.h - HEIGHT)/2;
  // draw the deck
  #if defined(PBL_COLOR)
  if (selected_row == 12) {
    graphics_context_set_fill_color(ctx, GColorYellow);
  } else {
    graphics_context_set_fill_color(ctx, GColorDukeBlue);
  }
  graphics_fill_rect(ctx, GRect(left+WIDTH-CARD_WIDTH,top,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_round_rect(ctx, GRect(left+WIDTH-CARD_WIDTH,top,CARD_WIDTH,CARD_HEIGHT), 4);
  #else
  if (selected_row == 12) {
    graphics_context_set_fill_color(ctx, GColorWhite);
  } else {
    graphics_context_set_fill_color(ctx, GColorBlack);
  }
  graphics_fill_rect(ctx, GRect(left+WIDTH-CARD_WIDTH,top,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_round_rect(ctx, GRect(left+WIDTH-CARD_WIDTH,top,CARD_WIDTH,CARD_HEIGHT), 4);
  #endif

  // draw the draw pile
  short down = ((status == SELECTING_ROW) && (selected_row == 11));
  short i = pile_number[11] - 1;
  if (i > 2) {
    i = 2;
  }
  for (; i>=0; i--) {
    short card = get_card_number(11, i);
    short highlight = ((status == SELECTING_CARD) && (selected_row == 11) && (card==get_card_number(11,0)));
    draw_card(deck.cards[card], left+WIDTH-2*CARD_WIDTH-MARGIN-12*i, top+down*12, 1, highlight, ctx); // always show
  }

  // draw the place for aces
  for (short i=0; i<4; i++) {
    short pile = i+7;
    graphics_draw_round_rect(ctx, GRect(left+(CARD_WIDTH+MARGIN)*i,top,CARD_WIDTH,CARD_HEIGHT), 4);
    #if defined(PBL_COLOR)
    if (selected_to_row == pile && status == SELECTING_LOC) {
      graphics_context_set_fill_color(ctx, GColorYellow);
      graphics_fill_rect(ctx, GRect(left+(CARD_WIDTH+MARGIN)*i,top,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
    }
    #else
    if (selected_to_row == pile && status == SELECTING_LOC) {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx, GRect(left+(CARD_WIDTH+MARGIN)*i,top,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_context_set_stroke_width(ctx, 2);
      graphics_draw_round_rect(ctx, GRect(left+(CARD_WIDTH+MARGIN)*i,top,CARD_WIDTH,CARD_HEIGHT), 4);
      graphics_context_set_stroke_width(ctx, 1);
    }
    #endif
    for (short j=pile_number[pile]-1; j>=0; j--) {
      short card = get_card_number(pile, j);
      short highlight = ((status == SELECTING_LOC) && (pile == selected_to_row));
      draw_card(deck.cards[card], left+(CARD_WIDTH+MARGIN)*i, top, 1, highlight, ctx); // 1 means show
    }
  }

  // draw the 7 piles
  short num_not_shown;
  for (short i=0; i<7; i++) {
    graphics_draw_round_rect(ctx, GRect(left+(CARD_WIDTH+MARGIN)*i,top+CARD_HEIGHT+10,CARD_WIDTH,CARD_HEIGHT), 4);
    if (selected_to_row == i && status == SELECTING_LOC && pile_number[i] == 0) {
      #if defined(PBL_COLOR)  
      graphics_context_set_fill_color(ctx, GColorYellow);
      graphics_fill_rect(ctx, GRect(left+(CARD_WIDTH+MARGIN)*i,top+CARD_HEIGHT+10,CARD_WIDTH,CARD_HEIGHT), 4, GCornersAll);
      #else
      graphics_context_set_stroke_width(ctx, 2);
      graphics_draw_round_rect(ctx, GRect(left+(CARD_WIDTH+MARGIN)*i,top+CARD_HEIGHT+10,CARD_WIDTH,CARD_HEIGHT), 4);
      graphics_context_set_stroke_width(ctx, 1);
      #endif
    }

    short y = CARD_HEIGHT + 10;
    for (short j=pile_number[i]-1; j>=0; j--) {
      short card = get_card_number(i, j);
      short highlightA = ((status == SELECTING_CARD) && (selected_row == i) && (card<=selected_card));
      short highlightB = ((status == SELECTING_LOC) && (i == selected_to_row));
      short highlight = highlightA || highlightB;

      short max_height = 168 - CARD_HEIGHT;
      if (y > max_height) {
        y = max_height;
      }
      draw_card(deck.cards[card], left+(CARD_WIDTH+MARGIN)*i, top+y, j<num_shown[i], highlight, ctx);

      if (j>=num_shown[i]) {
        y += NOT_SHOWN_GAP;
      } else if ((status == SELECTING_CARD) && (selected_card == card)) {
        y += CARD_SHOW_GAP;
      } else if (((selected_row != i) || (status != SELECTING_CARD)) && (j == num_shown[i]-1)) {
        y += CARD_SHOW_GAP;
      } else {
        y += HIDDEN_GAP;
      }
    }
  }

  if (status == SELECTING_ROW) {
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    if ((selected_row < 7) && (selected_row >=0)) {
      graphics_draw_bitmap_in_rect(ctx, down_arrow, GRect(left+(CARD_WIDTH+MARGIN)*selected_row+10-ARROW_WIDTH/2, top+CARD_HEIGHT-ARROW_WIDTH+10, ARROW_WIDTH, ARROW_WIDTH));
    } else if (selected_row == 11) {
      graphics_draw_bitmap_in_rect(ctx, down_arrow, GRect(left+WIDTH-2*CARD_WIDTH-MARGIN+10-ARROW_WIDTH/2, top, ARROW_WIDTH, ARROW_WIDTH));
    }
  }

  // draw win
  if (win == 1) {
    graphics_draw_text(ctx, "YOU WIN!", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0,top+HEIGHT-20,left+WIDTH,20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }
}

static void load_bitmaps() {
  hearts_icon = gbitmap_create_with_resource(RESOURCE_ID_HEARTS_ICON);
  diamonds_icon = gbitmap_create_with_resource(RESOURCE_ID_DIAMONDS_ICON);
  clubs_icon = gbitmap_create_with_resource(RESOURCE_ID_CLUBS_ICON);
  spades_icon = gbitmap_create_with_resource(RESOURCE_ID_SPADES_ICON);
  down_arrow = gbitmap_create_with_resource(RESOURCE_ID_DOWN_ARROW);
}

static void destroy_bitmaps() {
  gbitmap_destroy(hearts_icon);
  gbitmap_destroy(diamonds_icon);
  gbitmap_destroy(clubs_icon);
  gbitmap_destroy(spades_icon);
  gbitmap_destroy(down_arrow);
}

static void save_game() {
  if (win == 0) {
    persist_write_data(SOLITAIRE_CARDS_KEY, deck.cards, sizeof(deck.cards));
    persist_write_data(SOLITAIRE_PILE_KEY, pile_number, sizeof(pile_number));
    persist_write_data(SOLITAIRE_SHOWN_KEY, num_shown, sizeof(num_shown));
  } else {
    persist_delete(SOLITAIRE_CARDS_KEY);
    persist_delete(SOLITAIRE_PILE_KEY);
    persist_delete(SOLITAIRE_SHOWN_KEY);
  }
}

static void load_game() {
  short c[52];
  if (persist_exists(SOLITAIRE_CARDS_KEY) && persist_exists(SOLITAIRE_SHOWN_KEY) && persist_exists(SOLITAIRE_PILE_KEY)) {
    persist_read_data(SOLITAIRE_CARDS_KEY, c, sizeof(c));
    persist_read_data(SOLITAIRE_PILE_KEY, pile_number, sizeof(pile_number));
    persist_read_data(SOLITAIRE_SHOWN_KEY, num_shown, sizeof(num_shown));
    for (short i=0;i<13;i++) {
      APP_LOG(APP_LOG_LEVEL_INFO, "pile_number[%d] = %d", i, pile_number[i]);
    }

    deck = deck_from_cards(c);

    status = SELECTING_ROW;
    selected_row = 0;
    selected_to_row = 0;
    selected_card = -1;
    num_selected_cards = 1;
    num_pile_resets = 0;

    no_matches_found = 0;

    win = 0; // 1 for win, -1 for loss
    set_pile_number();

    layer_mark_dirty(s_solitaire_layer);
  } else {
    reset();
  }
}

static void solitaire_window_load(Window *window) {    
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);    

  s_solitaire_layer = layer_create(bounds);

  srand((unsigned) time(NULL));
  load_game();
  load_bitmaps();
  
  layer_set_update_proc(s_solitaire_layer, draw_solitaire);
  layer_add_child(window_get_root_layer(window), s_solitaire_layer);
  
  window_set_click_config_provider(window, (ClickConfigProvider) solitaire_config_provider);
  window_set_background_color(window, GColorWhite);
}

static void solitaire_window_unload(Window *window) {
  save_game();
  destroy_bitmaps();
  layer_destroy(s_solitaire_layer);
  window_destroy(s_solitaire_window);
  s_solitaire_window = NULL;
}

void solitaire_init() {
  // Create main Window element and assign to pointer
  s_solitaire_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_solitaire_window, (WindowHandlers) {
    .load = solitaire_window_load,
    .unload = solitaire_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_solitaire_window, true);
}