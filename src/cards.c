#if defined(PBL_COLOR)
  #include <pebble.h>
  #include "cards.h"
  
  Deck create_deck() {
    Deck d;
    for (short i=0; i<52; i++) {
      d.cards[i] = i;
    }
    return d;
  }
  
  Deck shuffle_deck(Deck d) {
    size_t i;
    for (i = 0; i < 52; i++) {
      size_t j = (rand() % (52-i)) + i;
      int t = d.cards[j];
      d.cards[j] = d.cards[i];
      d.cards[i] = t;
    }
    return d;
  }

  Deck move_card(Deck d, short from, short to) {
    short temp = d.cards[from];
    short direction;
    if (from > to) {
      direction = -1;
    } else {
      direction = 1;
    }
    for (short i=from; direction*i<direction*to; i=i+direction) {
      d.cards[i] = d.cards[i+direction];
    }
    d.cards[to] = temp;
    return d;
  }

  Deck deck_from_cards(short *cards) {
    Deck d;
    for (short i=0;i<52;i++) {
      d.cards[i] = cards[i];
    }
    return d;
  }

  void print_deck(Deck d) {
    for (short i=0; i<52; i++) {
      APP_LOG(APP_LOG_LEVEL_INFO, "%d", d.cards[i]);
    }
  }
#endif


