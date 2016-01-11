// define games
#if defined(PBL_COLOR)
  #define CHESS 0
  #define BLACKJACK 1
  #define TWO048 2
  #define DECRYPT 3
  #define SOLITAIRE 4
  #define FOOD 5
  #define TENNIS 6
  #define ABOUT 7
  #define INSTRUCTION 8
#else
  #define SOLITAIRE 0
  #define BLACKJACK 1
  #define TWO048 2
  #define FOOD 3
  #define TENNIS 4
  #define ABOUT 5
#endif

// define has settings, uncomment to give settings
#ifdef CHESS
  #define CHESS_HAS_SETTINGS 1
#endif
#ifdef BLACKJACK
  //#define BLACKJACK_HAS_SETTINGS 1
#endif
#ifdef TWO048
  //#define TWO048_HAS_SETTINGS 1
#endif
#ifdef SOLITAIRE
  //#define SOLITAIRE_HAS_SETTINGS 1
#endif
#ifdef TENNIS
  //#define TENNIS_HAS_SETTINGS 1
#endif
#ifdef FOOD
  //#define FOOD_HAS_SETTINGS 1
#endif
#ifdef DECRYPT
  //#define DECRYPT_HAS_SETTINGS 1
#endif

// define settings indices
#ifdef CHESS
  #define CHESS_DIFFICULTY 0
#endif

// define settings value
#ifdef CHESS
  #define CHESS_DIFFICULTY_SETTING 0
  #define CHESS_EASY 0
  #define CHESS_HARD 1
#endif

// define storage keys
#define SOLITAIRE_CARDS_KEY 23874
#define SOLITAIRE_PILE_KEY 3465
#define SOLITAIRE_SHOWN_KEY 875
#define TWO048_BOARD_STATE_KEY 34
#define BLACKJACK_PERSON_SCORE_KEY 122
#define BLACKJACK_DEALER_SCORE_KEY 428
#define CHESS_BOARD_STATE_KEY 1423
#define CHESS_CASTLE_RIGHT_KEY 9357
#define CHESS_MOVE_NO_KEY 9358
#define CHESS_CASTLE_LEFT_KEY 7347
#if defined(PBL_COLOR)
  #define CHESS_DIFFICULTY_KEY 0
#else
#endif