#pragma once

#ifdef PBL_PLATFORM_BASALT
	typedef struct Deck_s {
		short cards[52];
	} Deck;
	Deck create_deck();
	Deck shuffle_deck(Deck d);
	Deck move_card(Deck d, short from, short to);
	Deck deck_from_cards(short *cards);
#endif
