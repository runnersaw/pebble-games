#ifdef PBL_PLATFORM_BASALT
  #include <pebble.h>
  #include "cards.h"
  
  struct Deck {
    short cards[52];
  };
  
  struct Deck create_deck() {
    struct Deck d;
    for (short i=0; i<52; i++) {
      d.cards[i] = i;
    }
    return d;
  }
  
  void shuffle_deck(struct Deck d) {
    size_t i;
    for (i = 0; i < 52; i++) {
      size_t j = (rand() % (52-i)) + i;
      int t = d.cards[j];
      d.cards[j] = d.cards[i];
      d.cards[i] = t;
    }
  }

  void move_card(struct Deck d, short from, short to) {
    short temp = d.cards[from];
    short direction;
    if (from > to) {
      direction = -1;
    } else {
      direction = 1;
    }
    for (short i=from; direction*i<direction*to; i=i+direction) {
      APP_LOG(APP_LOG_LEVEL_INFO, "%d", i);
    }
  }

  void test() {
    struct Deck d = create_deck();
    shuffle_deck(d);
    APP_LOG(APP_LOG_LEVEL_INFO, "Should go from 10 to 1");
    move_card(d, 10, 1);
    APP_LOG(APP_LOG_LEVEL_INFO, "Should go from 10 to 1");
    move_card(d, 1, 10);
  }
#endif


