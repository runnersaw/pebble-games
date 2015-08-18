#include <pebble.h>
#include "chess.h"
  
#define WIDTH 136
#define BOX_BORDER 2
#define TOP_BORDER 12
#define LEFT_BORDER 4
  
#define BLACK_TEAM -1
#define WHITE_TEAM 1
  
// black pieces, less than 0
#define BLACK_KING -1
#define BLACK_QUEEN -2
#define BLACK_ROOK -3
#define BLACK_KNIGHT -4
#define BLACK_BISHOP -5
#define BLACK_PAWN -6
  
// white pieces, greater than 0
#define WHITE_KING 1
#define WHITE_QUEEN 2
#define WHITE_ROOK 3
#define WHITE_KNIGHT 4
#define WHITE_BISHOP 5
#define WHITE_PAWN 6
  
#define MAXIMUM_MOVES 80
#define SEARCH_DEPTH 2
  
#define BOARD_STATE_KEY 1423
#define CASTLE_RIGHT_KEY 9357
#define MOVE_NO_KEY 9358
#define CASTLE_LEFT_KEY 7347
  
static Window *s_chess_window;
static Layer *s_chess_layer;

#ifdef PBL_PLATFORM_BASALT
  static GBitmap *s_black_bishop;
  static GBitmap *s_black_rook;
  static GBitmap *s_black_knight;
  static GBitmap *s_black_king;
  static GBitmap *s_black_pawn;
  static GBitmap *s_black_queen;
  static GBitmap *s_white_bishop;
  static GBitmap *s_white_rook;
  static GBitmap *s_white_knight;
  static GBitmap *s_white_king;
  static GBitmap *s_white_pawn;
  static GBitmap *s_white_queen;
  static GBitmap *s_down_arrow;
#else
  static GBitmap *s_black_king_black;
  static GBitmap *s_black_knight_black;
  static GBitmap *s_black_rook_black;
  static GBitmap *s_black_bishop_black;
  static GBitmap *s_black_queen_black;
  static GBitmap *s_black_pawn_black;
  static GBitmap *s_black_king_white;
  static GBitmap *s_black_knight_white;
  static GBitmap *s_black_rook_white;
  static GBitmap *s_black_bishop_white;
  static GBitmap *s_black_queen_white;
  static GBitmap *s_black_pawn_white;
  static GBitmap *s_down_arrow;
#endif

static short team_won = 0; // will change to +1 for white, -1 for black

static short selected = 0;
static short on_x = 1;
static short selected_piece_x = 0;
static short selected_piece_y = 6;
static short selected_x = 0;
static short selected_y = 6;
static short move_count = 0;

static short display_box = 0;

static short board_state[8][8];
static short test_board_state[8][8];
static short test_possible_moves[8][8];
static short possible_moves[8][8];
static short possible_pieces_to_move[8][8];

static short turn = WHITE_TEAM; //white is 1, black is -1

static short white_castle_possible_right = 1;
static short white_castle_possible_left = 1;

static void create_bitmaps() {
  #ifdef PBL_PLATFORM_BASALT
    s_black_king = gbitmap_create_with_resource(RESOURCE_ID_BLACK_KING_BLACK);
    s_black_knight = gbitmap_create_with_resource(RESOURCE_ID_BLACK_KNIGHT_BLACK);
    s_black_rook = gbitmap_create_with_resource(RESOURCE_ID_BLACK_ROOK_BLACK);
    s_black_bishop = gbitmap_create_with_resource(RESOURCE_ID_BLACK_BISHOP_BLACK);
    s_black_queen = gbitmap_create_with_resource(RESOURCE_ID_BLACK_QUEEN_BLACK);
    s_black_pawn = gbitmap_create_with_resource(RESOURCE_ID_BLACK_PAWN_BLACK);
    s_white_king = gbitmap_create_with_resource(RESOURCE_ID_BLACK_KING_WHITE);
    s_white_knight = gbitmap_create_with_resource(RESOURCE_ID_BLACK_KNIGHT_WHITE);
    s_white_rook = gbitmap_create_with_resource(RESOURCE_ID_BLACK_ROOK_WHITE);
    s_white_bishop = gbitmap_create_with_resource(RESOURCE_ID_BLACK_BISHOP_WHITE);
    s_white_queen = gbitmap_create_with_resource(RESOURCE_ID_BLACK_QUEEN_WHITE);
    s_white_pawn = gbitmap_create_with_resource(RESOURCE_ID_BLACK_PAWN_WHITE);
    s_down_arrow = gbitmap_create_with_resource(RESOURCE_ID_DOWN_ARROW);
  #else
    s_black_king_black = gbitmap_create_with_resource(RESOURCE_ID_BLACK_KING_BLACK);
    s_black_knight_black = gbitmap_create_with_resource(RESOURCE_ID_BLACK_KNIGHT_BLACK);
    s_black_rook_black = gbitmap_create_with_resource(RESOURCE_ID_BLACK_ROOK_BLACK);
    s_black_bishop_black = gbitmap_create_with_resource(RESOURCE_ID_BLACK_BISHOP_BLACK);
    s_black_queen_black = gbitmap_create_with_resource(RESOURCE_ID_BLACK_QUEEN_BLACK);
    s_black_pawn_black = gbitmap_create_with_resource(RESOURCE_ID_BLACK_PAWN_BLACK);
    s_black_king_white = gbitmap_create_with_resource(RESOURCE_ID_BLACK_KING_WHITE);
    s_black_knight_white = gbitmap_create_with_resource(RESOURCE_ID_BLACK_KNIGHT_WHITE);
    s_black_rook_white = gbitmap_create_with_resource(RESOURCE_ID_BLACK_ROOK_WHITE);
    s_black_bishop_white = gbitmap_create_with_resource(RESOURCE_ID_BLACK_BISHOP_WHITE);
    s_black_queen_white = gbitmap_create_with_resource(RESOURCE_ID_BLACK_QUEEN_WHITE);
    s_black_pawn_white = gbitmap_create_with_resource(RESOURCE_ID_BLACK_PAWN_WHITE);
    s_down_arrow = gbitmap_create_with_resource(RESOURCE_ID_DOWN_ARROW);
  #endif
}

static void destroy_bitmaps() {
  #ifdef PBL_PLATFORM_BASALT
    gbitmap_destroy(s_black_king);
    gbitmap_destroy(s_black_knight);
    gbitmap_destroy(s_black_rook);
    gbitmap_destroy(s_black_bishop);
    gbitmap_destroy(s_black_queen);
    gbitmap_destroy(s_black_pawn);
    gbitmap_destroy(s_white_king);
    gbitmap_destroy(s_white_knight);
    gbitmap_destroy(s_white_rook);
    gbitmap_destroy(s_white_bishop);
    gbitmap_destroy(s_white_queen);
    gbitmap_destroy(s_white_pawn);
    gbitmap_destroy(s_down_arrow);
  #else
    gbitmap_destroy(s_black_king_black);
    gbitmap_destroy(s_black_knight_black);
    gbitmap_destroy(s_black_rook_black);
    gbitmap_destroy(s_black_bishop_black);
    gbitmap_destroy(s_black_queen_black);
    gbitmap_destroy(s_black_pawn_black);
    gbitmap_destroy(s_black_king_white);
    gbitmap_destroy(s_black_knight_white);
    gbitmap_destroy(s_black_rook_white);
    gbitmap_destroy(s_black_bishop_white);
    gbitmap_destroy(s_black_queen_white);
    gbitmap_destroy(s_black_pawn_white);
    gbitmap_destroy(s_down_arrow);
  #endif
}

static void print_state(short state[8][8]) {
  short m;
  for (m=0;m<8;m++) {
    APP_LOG(APP_LOG_LEVEL_INFO, "%d %d %d %d %d %d %d %d", state[m][0], state[m][1], state[m][2], state[m][3], state[m][4], state[m][5], state[m][6], state[m][7]);
  }
}

static void init_board_state() {
  short k,l;
  for (k=0;k<8;k++) {
    for (l=0;l<8;l++) {
      ResHandle rh = resource_get_handle(RESOURCE_ID_START_CHESS);
      resource_load_byte_range(rh, (8*k+l)*sizeof(short), (uint8_t*)&board_state[k][l], sizeof(short));
    }
  }
}

static void reset_state(short state[8][8]) {
  short k,l;
  for (k=0;k<8;k++) {
    for (l=0;l<8;l++) {
      state[k][l] = 0;
    }
  }
}

static void copy_state(short dest[8][8], short src[8][8]) {
  short k,l;
  for (k=0;k<8;k++) {
    for (l=0;l<8;l++) {
      dest[k][l] = src[k][l];
    }
  }
}

static short get_piece_at_position(short state[8][8], short x, short y) {
  return state[y][x];
}

static short is_valid(short x, short y) {
  if (x>7 || y>7 || x<0 || y<0) {
    return 0;
  } else {
    return 1;
  }
}  

static void generate_pawn_threaten(short state[8][8], short x, short y, short team) {
  if (team*get_piece_at_position(state, x+1, y-team)<0) {
    if (is_valid(x+1, y-team)==1) {
      test_possible_moves[y-team][x+1] = 1;
    }
  }
  if (team*get_piece_at_position(state, x-1, y-team)<0) {
    if (is_valid(x-1, y-team)==1) {
      test_possible_moves[y-team][x-1] = 1;
    }
  }
  if (get_piece_at_position(state, x, y-team)==0) { // none
    if (is_valid(x, y+1)==1) {
      test_possible_moves[y-team][x] = 1;
      if (y==1+5*(team+1)/2) {
        if (get_piece_at_position(state, x, y-2*team)==0) {
          if (is_valid(x, y-2*team)==1) {
            test_possible_moves[y-2*team][x] = 1;
          }
        }
      }
    }
  }
}

static void generate_knight_threaten(short state[8][8], short x, short y, short team) {
  if (-team*get_piece_at_position(state, x+2, y+1)>-1) { // none or opponent
    if (is_valid(x+2, y+1)==1) {
      test_possible_moves[y+1][x+2] = 1;
    }
  }
  if (-team*get_piece_at_position(state, x-2, y+1)>-1) { // none or opponent
    if (is_valid(x-2, y+1)==1) {
      test_possible_moves[y+1][x-2] = 1;
    }
  }
  if (-team*get_piece_at_position(state, x+1, y+2)>-1) { // none or opponent
    if (is_valid(x+1, y+2)==1) {
      test_possible_moves[y+2][x+1] = 1;
    }
  }
  if (-team*get_piece_at_position(state, x-1, y+2)>-1) { // none or opponent
    if (is_valid(x-1, y+2)==1) {
      test_possible_moves[y+2][x-1] = 1;
    }
  }
  if (-team*get_piece_at_position(state, x+2, y-1)>-1) { // none or opponent
    if (is_valid(x+2, y-1)==1) {
      test_possible_moves[y-1][x+2] = 1;
    }
  }
  if (-team*get_piece_at_position(state, x-2, y-1)>-1) { // none or opponent
    if (is_valid(x-2, y-1)==1) {
      test_possible_moves[y-1][x-2] = 1;
    }
  }
  if (-team*get_piece_at_position(state, x+1, y-2)>-1) { // none or opponent
    if (is_valid(x+1, y-2)==1) {
      test_possible_moves[y-2][x+1] = 1;
    }
  }
  if (-team*get_piece_at_position(state, x-1, y-2)>-1) { // none or opponent
    if (is_valid(x-1, y-2)==1) {
      test_possible_moves[y-2][x-1] = 1;
    }
  }
}

static void generate_bishop_threaten(short state[8][8], short k, short l, short team) {
  short x_pos = k+1;
  short y_pos = l+1;
  while (get_piece_at_position(state, x_pos, y_pos)==0 && x_pos<8 && y_pos<8) {
    test_possible_moves[y_pos][x_pos] = 1;
    x_pos = x_pos+1;
    y_pos = y_pos+1;
  }
  if (-team*get_piece_at_position(state, x_pos, y_pos)>0 && x_pos<8 && y_pos<8) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k-1;
  y_pos = l+1;
  while (get_piece_at_position(state, x_pos, y_pos)==0 && x_pos>-1 && y_pos<8) {
    test_possible_moves[y_pos][x_pos] = 1;
    x_pos = x_pos-1;
    y_pos = y_pos+1;
  }
  if (-team*get_piece_at_position(state, x_pos, y_pos)>0 && x_pos>-1 && y_pos<8) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k-1;
  y_pos = l-1;
  while (get_piece_at_position(state, x_pos, y_pos)==0 && x_pos>-1 && y_pos>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
    x_pos = x_pos-1;
    y_pos = y_pos-1;
  }
  if (-team*get_piece_at_position(state, x_pos, y_pos)>0 && x_pos>-1 && y_pos>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k+1;
  y_pos = l-1;
  while (get_piece_at_position(state, x_pos, y_pos)==0 && x_pos<8 && y_pos>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
    x_pos = x_pos+1;
    y_pos = y_pos-1;
  }
  if (-team*get_piece_at_position(state, x_pos, y_pos)>0 && x_pos<8 && y_pos>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
}

static void generate_king_threaten(short state[8][8], short k, short l, short team) {
  short x_pos = k+1;
  short y_pos = l;
  if (x_pos<8 && -team*get_piece_at_position(state, x_pos, y_pos)>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k+1;
  y_pos = l+1;
  if (x_pos<8 && y_pos<8 && -team*get_piece_at_position(state, x_pos, y_pos)>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k;
  y_pos = l+1;
  if (y_pos<8 && -team*get_piece_at_position(state, x_pos, y_pos)>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k-1;
  y_pos = l+1;
  if (x_pos>-1 && y_pos<8 && -team*get_piece_at_position(state, x_pos, y_pos)>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k-1;
  y_pos = l;
  if (x_pos>-1 && -team*get_piece_at_position(state, x_pos, y_pos)>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k-1;
  y_pos = l-1;
  if (x_pos>-1 && y_pos>-1 && -team*get_piece_at_position(state, x_pos, y_pos)>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k+1;
  y_pos = l-1;
  if (x_pos<8 && y_pos>-1 && -team*get_piece_at_position(state, x_pos, y_pos)>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k;
  y_pos = l-1;
  if (y_pos>-1 && -team*get_piece_at_position(state, x_pos, y_pos)>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
}

static void generate_rook_threaten(short state[8][8], short k, short l, short team) {
  short x_pos = k;
  short y_pos = l+1;
  while (get_piece_at_position(state, x_pos, y_pos)==0 && y_pos<8) {
    test_possible_moves[y_pos][x_pos] = 1;
    y_pos = y_pos+1;
  }
  if (-team*get_piece_at_position(state, x_pos, y_pos)>0 && y_pos<8) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k;
  y_pos = l-1;
  while (get_piece_at_position(state, x_pos, y_pos)==0 && y_pos>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
    y_pos = y_pos-1;
  }
  if (-team*get_piece_at_position(state, x_pos, y_pos)>0 && y_pos>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k+1;
  y_pos = l;
  while (get_piece_at_position(state, x_pos, y_pos)==0 && x_pos<8) {
    test_possible_moves[y_pos][x_pos] = 1;
    x_pos = x_pos+1;
  }
  if (-team*get_piece_at_position(state, x_pos, y_pos)>0 && x_pos<8) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
  x_pos = k-1;
  y_pos = l;
  while (get_piece_at_position(state, x_pos, y_pos)==0 && x_pos>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
    x_pos = x_pos-1;
  }
  if (-team*get_piece_at_position(state, x_pos, y_pos)>0 && x_pos>-1) {
    test_possible_moves[y_pos][x_pos] = 1;
  }
}

static void generate_queen_threaten(short state[8][8], short k, short l, short team) {
  generate_bishop_threaten(state, k, l, team);
  generate_rook_threaten(state, k, l, team);
}

static void generate_moves_threatening(short state[8][8], short team) {
  reset_state(test_possible_moves);
  short k,l,piece;
  for (k=0;k<8;k++) {
    for (l=0;l<8;l++) {
      piece=get_piece_at_position(state, k, l);
      if (team==BLACK_TEAM) {
        if (piece==BLACK_PAWN) {
          generate_pawn_threaten(state, k, l, BLACK_TEAM);
        } else if (piece==BLACK_KNIGHT) {
          generate_knight_threaten(state, k, l, BLACK_TEAM);
        } else if (piece==BLACK_BISHOP) {
          generate_bishop_threaten(state, k, l, BLACK_TEAM);
        } else if (piece==BLACK_KING) {
          generate_king_threaten(state, k, l, BLACK_TEAM);
        } else if (piece==BLACK_QUEEN) {
          generate_queen_threaten(state, k, l, BLACK_TEAM);
        } else if (piece==BLACK_ROOK) {
          generate_rook_threaten(state, k, l, BLACK_TEAM);
        }
      } else if (team==WHITE_TEAM) {
        if (piece==WHITE_PAWN) {
          generate_pawn_threaten(state, k, l, WHITE_TEAM);
        } else if (piece==WHITE_KNIGHT) {
          generate_knight_threaten(state, k, l, WHITE_TEAM);
        } else if (piece==WHITE_BISHOP) {
          generate_bishop_threaten(state, k, l, WHITE_TEAM);
        } else if (piece==WHITE_KING) {
          generate_king_threaten(state, k, l, WHITE_TEAM);
        } else if (piece==WHITE_QUEEN) {
          generate_queen_threaten(state, k, l, WHITE_TEAM);
        } else if (piece==WHITE_ROOK) {
          generate_rook_threaten(state, k, l, WHITE_TEAM);
        }
      }
    }
  }
}

static short test_move(short state[8][8], short start_x, short start_y, short end_x, short end_y, short team) { // 1 is white, 0 is black
  if (is_valid(end_x, end_y)==0) {
    return 0;
  }
  if (get_piece_at_position(state, start_x, start_y)==WHITE_KING&&start_x==4&&start_y==7) {
    if (end_x==6) {
      generate_moves_threatening(state, -team);
      if (test_possible_moves[7][4]==0&&test_possible_moves[7][5]==0&&test_possible_moves[7][6]==0) {
        return 1;
      } else {
        return 0;
      }
    } else if (end_x==2) {
      generate_moves_threatening(state, -team);
      if (test_possible_moves[7][4]==0&&test_possible_moves[7][3]==0&&test_possible_moves[7][2]==0) {
        return 1;
      } else {
        return 0;
      }
    }
  }
  copy_state(test_board_state, state);
  reset_state(test_possible_moves);
  test_board_state[end_y][end_x] = test_board_state[start_y][start_x];
  test_board_state[start_y][start_x] = 0;
  short searched_piece;
  if (team == WHITE_TEAM) {
    searched_piece = WHITE_KING;
  } else {
    searched_piece = BLACK_KING;
  }
  generate_moves_threatening(test_board_state, -team);
  short k,l;
  for (k=0;k<8;k++) {
    for (l=0;l<8;l++) {
      if (get_piece_at_position(test_board_state, k, l)==searched_piece) {
        if (test_possible_moves[l][k]==1) {
          return 0;
        } else {
          return 1;
        }
      }
    }
  }
  return 0;
}

void generate_castle_possible(short state[8][8]) {
  if (white_castle_possible_right==1&&get_piece_at_position(state,5,7)==0&&get_piece_at_position(state,6,7)==0) {
    test_possible_moves[7][6]=1;
  }
  if (white_castle_possible_left==1&&get_piece_at_position(state,1,7)==0&&get_piece_at_position(state,2,7)==0&&get_piece_at_position(state,3,7)==0) {
    test_possible_moves[7][2]=1;
  }
}

static void generate_move(short state[8][8], short x, short y) {
  short temp_possible_moves[8][8];
  short piece, team;
  piece=get_piece_at_position(state, x,y);
  if (piece>0) {
    team = WHITE_TEAM;
  } else {
    team = BLACK_TEAM;
  }
  if (piece==BLACK_PAWN) {
    generate_pawn_threaten(state, x, y, BLACK_TEAM);
  } else if (piece==BLACK_KNIGHT) {
    generate_knight_threaten(state, x, y, BLACK_TEAM);
  } else if (piece==BLACK_ROOK) {
    generate_rook_threaten(state, x, y, BLACK_TEAM);
  } else if (piece==BLACK_QUEEN) {
    generate_queen_threaten(state, x, y, BLACK_TEAM);
  } else if (piece==BLACK_BISHOP) {
    generate_bishop_threaten(state, x, y, BLACK_TEAM);
  } else if (piece==BLACK_KING) {
    generate_king_threaten(state, x, y, BLACK_TEAM);
  } else if (piece==WHITE_KNIGHT) {
    generate_knight_threaten(state, x, y, WHITE_TEAM);
  } else if (piece==WHITE_PAWN) {
    generate_pawn_threaten(state, x, y, WHITE_TEAM);
  } else if (piece==WHITE_ROOK) {
    generate_rook_threaten(state, x, y, WHITE_TEAM);
  } else if (piece==WHITE_QUEEN) {
    generate_queen_threaten(state, x, y, WHITE_TEAM);
  } else if (piece==WHITE_BISHOP) {
    generate_bishop_threaten(state, x, y, WHITE_TEAM);
  } else if (piece==WHITE_KING) {
    generate_king_threaten(state, x, y, WHITE_TEAM);
    generate_castle_possible(state);
  }
  copy_state(temp_possible_moves,test_possible_moves);
  short k,l;
  for (k=0;k<8;k++) {
    for (l=0;l<8;l++) {
      if (temp_possible_moves[l][k]==1) {
        if (test_move(state, x, y, k, l, team)==1) {
          possible_moves[l][k]=1;
          possible_pieces_to_move[y][x]=1;
        }
      }
    }
  }
}

static void generate_moves(short state[8][8], short team) {
  short temp_possible_moves[8][8];
  reset_state(possible_moves);
  reset_state(possible_pieces_to_move);
  short x,y;
  for (x=0;x<8;x++) {
    for (y=0;y<8;y++) {
      if (team*get_piece_at_position(state, x, y) > 0) {
        reset_state(test_possible_moves);
        generate_move(state,x,y);
      }
    }
  }
}

static short is_move_possible(short state[8][8], short team) {
  generate_moves(state, team);
  short canMove = 0;
  short i, j;
  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      if (possible_moves[i][j] == 1) {
        return 1;
      }
    }
  }
  return 0;
}

static void move(short state[8][8], short start_x, short start_y, short end_x, short end_y) {
  if(get_piece_at_position(board_state, start_x, start_y)==WHITE_KING) {
    if (start_x==4&&start_y==7&&end_x==6&&end_y==7) {
      move(state, 7,7,5,7);
    }
    if (start_x==4&&start_y==7&&end_x==2&&end_y==7) {
      move(state, 0,7,3,7);
    }
  }
  
  state[end_y][end_x] = state[start_y][start_x];
  state[start_y][start_x] = 0;
    
  if (get_piece_at_position(state, end_x, end_y)==WHITE_PAWN && end_y==0) {
    state[end_y][end_x] = WHITE_QUEEN;
  }
    
  if (get_piece_at_position(state, end_x, end_y)==BLACK_PAWN && end_y==7) {
    state[end_y][end_x] = BLACK_QUEEN;
  }
}

//http://en.wikipedia.org/wiki/Chess_piece_relative_value#Hans_Berliner.27s_system

/*f(p) = 200(K-K')
       + 9(Q-Q')
       + 5(R-R')
       + 3(B-B' + N-N')
       + 1(P-P')
       - 0.5(D-D' + S-S' + I-I')
       + 0.1(M-M') + ...*/

/*pawn = 1
knight = 3.2
bishop = 3.33
rook = 5.1
queen = 8.8*/

short find_pawn_value(short state[8][8], short x,short y,short team) {
  short score=0;
  if (get_piece_at_position(state, x, y-team)!=0) {
    score +=6*team;
  } else if (get_piece_at_position(state, x, y-2*team)!=0) {
    score +=6*team;
  } else {
    score +=10*team;
  }
  return score;
}

short find_knight_value(short state[8][8], short x,short y,short team) {
  short score=0;
  if (x==0||x==7||y==0||y==7) {
    score += 25*team;
  } else {
    score += 32*team;
  }
  return score;
}

short find_bishop_value(short state[8][8], short x,short y,short team) {
  short score=0;
  score+=33*team;
  return score;
}

short find_rook_value(short state[8][8], short x,short y,short team) {
  short score=0;
  score+=51*team;
  return score;
}

short find_queen_value(short state[8][8], short x,short y,short team) {
  short score=0;
  score+=88*team;
  return score;
}

short find_king_value(short state[8][8], short x,short y,short team) {
  short score=0;
  score+=10000*team;
  return score;
}

static short find_board_score(short state[8][8]) {
  short m,n,piece;
  short score = 0;
  for (m=0;m<8;m++) {
    for (n=0;n<8;n++) {
      piece = get_piece_at_position(state, m, n);
      if (piece==WHITE_PAWN) {
        score += find_pawn_value(state, m, n, WHITE_TEAM);
      } else if (piece==WHITE_KNIGHT) {
        score += find_knight_value(state, m, n, WHITE_TEAM);
      } else if (piece==WHITE_BISHOP) {
        score += find_bishop_value(state, m, n, WHITE_TEAM);
      } else if (piece==WHITE_ROOK) {
        score += find_rook_value(state, m, n, WHITE_TEAM);
      } else if (piece==WHITE_QUEEN) {
        score += find_queen_value(state, m, n, WHITE_TEAM);
      } else if (piece==WHITE_KING) {
        score += find_king_value(state, m, n, WHITE_TEAM);
      } else if (piece==BLACK_PAWN) {
        score += find_pawn_value(state, m, n, BLACK_TEAM);
      } else if (piece==BLACK_KNIGHT) {
        score += find_knight_value(state, m, n, BLACK_TEAM);
      } else if (piece==BLACK_BISHOP) {
        score += find_bishop_value(state, m, n, BLACK_TEAM);
      } else if (piece==BLACK_ROOK) {
        score += find_rook_value(state, m, n, BLACK_TEAM);
      } else if (piece==BLACK_QUEEN) {
        score += find_queen_value(state, m, n, BLACK_TEAM);
      } else if (piece==BLACK_KING) {
        score += find_king_value(state, m, n, BLACK_TEAM);
      }
    }
  }
  return score;
}

static short find_next_x_piece() {
  short temp_x = selected_x;
  short k,l;
  for (k=0;k<8;k++) {
    if (temp_x==7) {
      temp_x=0;
    } else {
      temp_x++;
    }
    for (l=0;l<8;l++) {
      if (possible_pieces_to_move[l][temp_x]==1) {
        selected_x = temp_x;
        return selected_x;
      }
    }
  }
  return -1;
}

static short find_next_y_piece() {
  short k;
  for (k=0;k<8;k++) {
    if (selected_y==7) {
      selected_y=0;
    } else {
      selected_y++;
    }
    if (possible_pieces_to_move[selected_y][selected_x]==1) {
      return selected_y;
    }
  }
  return -1;
}

short find_move(short state[8][8], short team, short depth) {
  short m,n,j,k,score,temp_score;
  short best_move[4] = {0,0,0,0};
  short temp_state[8][8];
  short copy_possible_moves[8][8];
  short copy_possible_pieces[8][8];
  
  if (move_count==1) {
    move(board_state,0,1,0,2);
    turn=-turn;
    move_count++;
    generate_moves(board_state, turn);
    find_next_x_piece();
    find_next_y_piece();
    layer_mark_dirty(s_chess_layer);
    selected = 0;
    display_box=0;
    return -1;
  }
  
  if (move_count==3) {
    move(board_state,0,2,0,3);
    turn=-turn;
    move_count++;
    generate_moves(board_state, turn);
    find_next_x_piece();
    find_next_y_piece();
    layer_mark_dirty(s_chess_layer);
    selected = 0;
    display_box=0;
    return -1;
  }
  
  if (move_count==5) {
    move(board_state,0,3,0,4);
    turn=-turn;
    move_count++;
    generate_moves(board_state, turn);
    find_next_x_piece();
    find_next_y_piece();
    layer_mark_dirty(s_chess_layer);
    selected = 0;
    display_box=0;
    return -1;
  }
  
  if (depth==0) {
    // return the board score, done
    return find_board_score(state);
  } else {
    //for all of the moves, 
    //call it again, with depth-1, 
    //get best score, return that
    // init vars
    generate_moves(state, team);
    score = 1234; // represents uninitialized
    // make all moves and then call it again here, 
    copy_state(copy_possible_pieces, possible_pieces_to_move);
    for (m=0;m<8;m++) {
      for (n=0;n<8;n++) {
        if (copy_possible_pieces[n][m]==1) { // if piece can move
          reset_state(possible_moves);
          reset_state(possible_pieces_to_move);
          reset_state(test_possible_moves);
          generate_move(state,m,n);
          copy_state(copy_possible_moves, possible_moves);
          for (j=0;j<8;j++) {
            for (k=0;k<8;k++) {
              if (copy_possible_moves[k][j]==1) {
                // make the move with temp_state and then call again with temp_state, opposite move, depth-1
                copy_state(temp_state, state);
                move(temp_state,m,n,j,k);
                temp_score = find_move(temp_state, -team, depth-1);
                if (score==1234) { // uninitialized
                  score = temp_score;
                  best_move[0]=m;
                  best_move[1]=n;
                  best_move[2]=j;
                  best_move[3]=k;
                } else {
                  // case for black
                  if (team==BLACK_TEAM) {
                    if (temp_score<score) {
                      score = temp_score;
                      best_move[0]=m;
                      best_move[1]=n;
                      best_move[2]=j;
                      best_move[3]=k;
                    }
                  } else {
                    if (temp_score>score) {
                      score = temp_score;
                      best_move[0]=m;
                      best_move[1]=n;
                      best_move[2]=j;
                      best_move[3]=k;
                    }
                  }
                }
              }
            }
          }
        }
      }
    } 
    if (depth==SEARCH_DEPTH) {
      move(board_state,best_move[0],best_move[1],best_move[2],best_move[3]);
      turn=-turn;
      move_count++;
      generate_moves(board_state, turn);
      find_next_x_piece();
      find_next_y_piece();
      layer_mark_dirty(s_chess_layer);
      selected = 0;
      display_box=0;
      return score;
    }
  }
  return score;
}

static void white_win() {
  team_won = WHITE_TEAM;
  layer_mark_dirty(s_chess_layer);
}

static void black_win() {
  team_won = BLACK_TEAM;
  layer_mark_dirty(s_chess_layer);
}

static void change_turn() {
  // this is called only after white moves, i think
  turn=-turn;
  move_count++;

  if (turn==BLACK_TEAM) {
    if (!is_move_possible(board_state, turn)) {
      white_win();
      return;
    }
    short search_depth=SEARCH_DEPTH, pieces=0, j,k;
    for (j=0;j<8;j++) {
      for (k=0;k<8;k++) {
        if (get_piece_at_position(board_state,j,k)<0) { //number of pieces on black team
          pieces++;
        }
      }
    }
    if (pieces<5) {
      search_depth++;
    }
    if (pieces<3) {
      search_depth++;
    }

    find_move(board_state, turn, search_depth);
  }
}

static void draw_chess(Layer *layer, GContext *ctx) {
  short m,n;
  #ifdef PBL_PLATFORM_BASALT
    graphics_context_set_stroke_color(ctx, GColorRajah);
    graphics_context_set_fill_color(ctx, GColorRajah);
    for (m=0;m<4;m++) {
      for (n=0;n<4;n++) {
        graphics_fill_rect(ctx, GRect(m*WIDTH/4+LEFT_BORDER,n*WIDTH/4+TOP_BORDER,WIDTH/8,WIDTH/8), 0, GCornerNone);
      }
    }
    for (m=0;m<4;m++) {
      for (n=0;n<4;n++) {
        graphics_fill_rect(ctx, GRect((2*m+1)*WIDTH/8+LEFT_BORDER,(2*n+1)*WIDTH/8+TOP_BORDER,WIDTH/8,WIDTH/8), 0, GCornerNone);
      }
    }
    graphics_context_set_stroke_color(ctx, GColorWindsorTan);
    graphics_context_set_fill_color(ctx, GColorWindsorTan);
  #else
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_fill_color(ctx, GColorBlack);
  #endif
  graphics_draw_rect(ctx, GRect(LEFT_BORDER, TOP_BORDER, WIDTH, WIDTH));
  short i, j;
  for (i=0;i<4;i++) {
    for (j=0;j<4;j++) {
      graphics_fill_rect(ctx, GRect(i*WIDTH/4+LEFT_BORDER,(2*j+1)*WIDTH/8+TOP_BORDER,WIDTH/8,WIDTH/8), 0, GCornerNone);
    }
  }
  for (i=0;i<4;i++) {
    for (j=0;j<4;j++) {
      graphics_fill_rect(ctx, GRect((2*i+1)*WIDTH/8+LEFT_BORDER,j*WIDTH/4+TOP_BORDER,WIDTH/8,WIDTH/8), 0, GCornerNone);
    }
  }
  
  // draw pieces
  for (m=0;m<8;m++) {
    for (n=0;n<8;n++) {
      #ifdef PBL_PLATFORM_BASALT
        graphics_context_set_compositing_mode(ctx, GCompOpSet);
        if (get_piece_at_position(board_state,n,m) == WHITE_PAWN) {
          graphics_draw_bitmap_in_rect(ctx, s_white_pawn, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == WHITE_ROOK) {
          graphics_draw_bitmap_in_rect(ctx, s_white_rook, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == WHITE_QUEEN) {
          graphics_draw_bitmap_in_rect(ctx, s_white_queen, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == WHITE_BISHOP) {
          graphics_draw_bitmap_in_rect(ctx, s_white_bishop, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == WHITE_KNIGHT) {
          graphics_draw_bitmap_in_rect(ctx, s_white_knight, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == WHITE_KING) {
          graphics_draw_bitmap_in_rect(ctx, s_white_king, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == BLACK_PAWN) {
          graphics_draw_bitmap_in_rect(ctx, s_black_pawn, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == BLACK_BISHOP) {
          graphics_draw_bitmap_in_rect(ctx, s_black_bishop, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == BLACK_QUEEN) {
          graphics_draw_bitmap_in_rect(ctx, s_black_queen, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == BLACK_KING) {
          graphics_draw_bitmap_in_rect(ctx, s_black_king, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == BLACK_KNIGHT) {
          graphics_draw_bitmap_in_rect(ctx, s_black_knight, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        } else if (get_piece_at_position(board_state,n,m) == BLACK_ROOK) {
          graphics_draw_bitmap_in_rect(ctx, s_black_rook, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
        }
      #else
        if ((m+n)%2==1) { // black square
          if (get_piece_at_position(board_state,n,m)>0) { // is white
            graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
            if (get_piece_at_position(board_state,n,m) == WHITE_PAWN) {
              graphics_draw_bitmap_in_rect(ctx, s_black_pawn_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == WHITE_ROOK) {
              graphics_draw_bitmap_in_rect(ctx, s_black_rook_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == WHITE_QUEEN) {
              graphics_draw_bitmap_in_rect(ctx, s_black_queen_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == WHITE_BISHOP) {
              graphics_draw_bitmap_in_rect(ctx, s_black_bishop_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == WHITE_KNIGHT) {
              graphics_draw_bitmap_in_rect(ctx, s_black_knight_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == WHITE_KING) {
              graphics_draw_bitmap_in_rect(ctx, s_black_king_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            }
          } else { //black
            graphics_context_set_compositing_mode(ctx, GCompOpAssign);
            if (get_piece_at_position(board_state,n,m) == BLACK_PAWN) {
              graphics_draw_bitmap_in_rect(ctx, s_black_pawn_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == BLACK_BISHOP) {
              graphics_draw_bitmap_in_rect(ctx, s_black_bishop_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == BLACK_QUEEN) {
              graphics_draw_bitmap_in_rect(ctx, s_black_queen_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == BLACK_KING) {
              graphics_draw_bitmap_in_rect(ctx, s_black_king_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == BLACK_KNIGHT) {
              graphics_draw_bitmap_in_rect(ctx, s_black_knight_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == BLACK_ROOK) {
              graphics_draw_bitmap_in_rect(ctx, s_black_rook_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            }
          }
        } else { // white square
          if (get_piece_at_position(board_state,n,m)>0) {
            graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
            if (get_piece_at_position(board_state,n,m) == WHITE_PAWN) {
              graphics_draw_bitmap_in_rect(ctx, s_black_pawn_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == WHITE_ROOK) {
              graphics_draw_bitmap_in_rect(ctx, s_black_rook_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == WHITE_QUEEN) {
              graphics_draw_bitmap_in_rect(ctx, s_black_queen_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == WHITE_BISHOP) {
              graphics_draw_bitmap_in_rect(ctx, s_black_bishop_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == WHITE_KNIGHT) {
              graphics_draw_bitmap_in_rect(ctx, s_black_knight_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == WHITE_KING) {
              graphics_draw_bitmap_in_rect(ctx, s_black_king_black, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            }
          } else { 
            graphics_context_set_compositing_mode(ctx, GCompOpAssign);
            if (get_piece_at_position(board_state,n,m) == BLACK_PAWN) {
              graphics_draw_bitmap_in_rect(ctx, s_black_pawn_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == BLACK_BISHOP) {
              graphics_draw_bitmap_in_rect(ctx, s_black_bishop_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == BLACK_QUEEN) {
              graphics_draw_bitmap_in_rect(ctx, s_black_queen_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == BLACK_KING) {
              graphics_draw_bitmap_in_rect(ctx, s_black_king_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == BLACK_KNIGHT) {
              graphics_draw_bitmap_in_rect(ctx, s_black_knight_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            } else if (get_piece_at_position(board_state,n,m) == BLACK_ROOK) {
              graphics_draw_bitmap_in_rect(ctx, s_black_rook_white, GRect(n*WIDTH/8+LEFT_BORDER+1,m*WIDTH/8+TOP_BORDER+1,15,15));
            }
          }
        }
      #endif
    }
  }
  
  // draw indicator
  if (display_box==2) {
    if ((selected_x+selected_y)%2==1) { // black square
      graphics_context_set_stroke_color(ctx, GColorWhite);
    } else {
      graphics_context_set_stroke_color(ctx, GColorBlack);
    }
    #ifdef PBL_PLATFORM_BASALT
      graphics_context_set_stroke_color(ctx, GColorBlack);
    #endif
    graphics_draw_rect(ctx, GRect(selected_x*WIDTH/8+BOX_BORDER+LEFT_BORDER,selected_y*WIDTH/8+BOX_BORDER+TOP_BORDER,WIDTH/8-2*BOX_BORDER,WIDTH/8-2*BOX_BORDER));
    for (m=0;m<8;m++) {
      for (n=0;n<8;n++) {
        if (get_piece_at_position(possible_moves,m,n)==1) {
          #ifdef PBL_PLATFORM_BASALT
            graphics_context_set_fill_color(ctx, GColorBlack);
          #else
            if ((m+n)%2==1) {
              graphics_context_set_fill_color(ctx,GColorWhite);
            } else {
              graphics_context_set_fill_color(ctx,GColorBlack);
            }
          #endif
          graphics_fill_circle(ctx, GPoint(m*WIDTH/8+WIDTH/16+LEFT_BORDER,n*WIDTH/8+WIDTH/16+TOP_BORDER), 3);
        }
      }
    }
  } else if (display_box==1) {
    #ifdef PBL_PLATFORM_BASALT
      graphics_context_set_stroke_color(ctx, GColorBlack);
    #else
      if ((selected_x+selected_y)%2==1) { // black square
        graphics_context_set_stroke_color(ctx, GColorWhite);
      } else {
        graphics_context_set_stroke_color(ctx, GColorBlack);
      }
    #endif
    graphics_draw_rect(ctx, GRect(selected_x*WIDTH/8+BOX_BORDER+LEFT_BORDER,selected_y*WIDTH/8+BOX_BORDER+TOP_BORDER,WIDTH/8-2*BOX_BORDER,WIDTH/8-2*BOX_BORDER));
  } else {
    #ifdef PBL_PLATFORM_BASALT
      graphics_context_set_compositing_mode(ctx, GCompOpSet);
    #else
      graphics_context_set_compositing_mode(ctx, GCompOpAssign);
    #endif
    graphics_draw_bitmap_in_rect(ctx, s_down_arrow, GRect(selected_x*WIDTH/8+2+LEFT_BORDER,0,12,12));
  }

  // draw win conditions
  if (team_won == WHITE_TEAM) {
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, "WHITE WINS", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, 72, 144, 30), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  } else if (team_won == BLACK_TEAM) {
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, "BLACK WINS", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, 72, 144, 30), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

static short find_previous_x_piece() {
  short temp_x = selected_x;
  short k,l;
  for (k=0;k<8;k++) {
    if (temp_x==0) {
      temp_x=7;
    } else {
      temp_x--;
    }
    for (l=0;l<8;l++) {
      if (possible_pieces_to_move[l][temp_x]==1) {
        selected_x = temp_x;
        return selected_x;
      }
    }
  }
  return -1;
}

static short find_previous_y_piece() {
  short k;
  for (k=0;k<8;k++) {
    if (selected_y==0) {
      selected_y=7;
    } else {
      selected_y--;
    }
    if (possible_pieces_to_move[selected_y][selected_x]==1) {
      return selected_y;
    }
  }
  return -1;
}

static short find_next_move() {
  short k,l;
  for (k=0;k<8;k++) {
    for (l=0;l<8;l++) {
      if (selected_x==7) {
        selected_x=0;
        if (selected_y==7) {
          selected_y=0;
        } else {
          selected_y++;
        }
      } else {
        selected_x++;
      }
      if (possible_moves[selected_y][selected_x]==1) {
        return -1;
      }
    }
  }
  return -1;
}

static short find_previous_move() {
  short k,l;
  for (k=0;k<8;k++) {
    for (l=0;l<8;l++) {
      if (selected_x==0) {
        selected_x=7;
        if (selected_y==0) {
          selected_y=7;
        } else {
          selected_y--;
        }
      } else {
        selected_x--;
      }
      if (possible_moves[selected_y][selected_x]==1) {
        return -1;
      }
    }
  }
  return -1;
}

static void up_handler() {
  if (selected == 0) {
    if (on_x == 1) {
      find_previous_x_piece();
    } else {
      find_previous_y_piece();
    }
  } else {
    find_previous_move();
  }
   layer_mark_dirty(s_chess_layer);
}

static void down_handler() {
  if (selected == 0) {
    if (on_x == 1) {
      find_next_x_piece();
    } else {
      find_next_y_piece();
    }
  } else {
    find_next_move();
  }
   layer_mark_dirty(s_chess_layer);
}

static void select_handler() {
  if (selected == 0) { // means hasn't selected a piece to move yet
    if (on_x == 1) { // means hasn't selected x yet, selecting now
      on_x=0;
      find_next_y_piece();
      display_box = 1;
       layer_mark_dirty(s_chess_layer);
    } else { // selected y, lock in piece to move
      selected = 1;
      selected_piece_x = selected_x;
      selected_piece_y = selected_y;
      reset_state(possible_moves);
      reset_state(possible_pieces_to_move);
      reset_state(test_possible_moves);
      generate_move(board_state, selected_x, selected_y);
      selected_x = 0;
      selected_y = 0;
      find_next_move();
      on_x=1;
      display_box=2;
    }
  } else { // has selected piece, just selected place to move    
  
    if ((selected_piece_x==0&& selected_piece_y==7) || (selected_piece_x==4&& selected_piece_y==7)) {
      white_castle_possible_left = 0;
    }
  
    if ((selected_piece_x==7&& selected_piece_y==7) || (selected_piece_x==4&& selected_piece_y==7)) {
      white_castle_possible_right = 0;
    }
    
    move(board_state, selected_piece_x, selected_piece_y, selected_x, selected_y);
    change_turn();
    if (!is_move_possible(board_state, turn) && turn == WHITE_TEAM) {
      black_win();
    }
    generate_moves(board_state, turn);
    find_next_x_piece();
    find_next_y_piece();
    layer_mark_dirty(s_chess_layer);
    selected = 0;
    display_box=0;
  }
   layer_mark_dirty(s_chess_layer);
}

static void back_handler() {
  if (selected == 0) {
    if (on_x == 1) {
      // save game and leave
      window_stack_pop(true);
    } else {
      on_x = 1;
      display_box=0;
      find_next_y_piece();
    }
  } else {
    selected = 0;
    selected_x = selected_piece_x;
    selected_y = selected_piece_y;
    generate_moves(board_state, turn);
    on_x=0;
    display_box=1;
  }
   layer_mark_dirty(s_chess_layer);
}

void chess_reset() {
  selected = 0;
  on_x = 1;
  selected_piece_x = 0;
  selected_piece_y = 6;
  selected_x = 0;
  selected_y = 6;
  move_count = 0;

  display_box = 0;
  
  white_castle_possible_right = 1;
  white_castle_possible_left = 1;
  
  turn = WHITE_TEAM; //white is 1, black is -1
  init_board_state();
  
  reset_state(possible_moves);
  reset_state(test_possible_moves);
  reset_state(possible_pieces_to_move);
  generate_moves(board_state, turn);
  print_state(possible_pieces_to_move);
  print_state(possible_moves);
  
  layer_mark_dirty(s_chess_layer);
}

void chess_config_provider(Window *window) {
  // set click listeners
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 200, up_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 200, down_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, chess_reset, NULL);
}

static void chess_window_load(Window *window) {
  create_bitmaps();
  
  s_chess_layer = layer_create(GRect(0, 0, 144, 152));
  
  layer_set_update_proc(s_chess_layer, draw_chess);
  layer_add_child(window_get_root_layer(s_chess_window), s_chess_layer);
  
  init_board_state();
  
  move_count = 0;
  if (persist_exists(BOARD_STATE_KEY)) {
    persist_read_data(BOARD_STATE_KEY, board_state, sizeof(board_state));
  }
  if (persist_exists(CASTLE_LEFT_KEY)) {
    white_castle_possible_left = persist_read_int(CASTLE_LEFT_KEY);
  }
  if (persist_exists(CASTLE_RIGHT_KEY)) {
    white_castle_possible_right = persist_read_int(CASTLE_RIGHT_KEY);
  }
  if (persist_exists(MOVE_NO_KEY)) {
    move_count = persist_read_int(MOVE_NO_KEY);
  }
  
  reset_state(possible_moves);
  reset_state(test_possible_moves);
  reset_state(possible_pieces_to_move);
  generate_moves(board_state, turn);
  find_next_x_piece();
  find_next_y_piece();
  window_set_click_config_provider(window, (ClickConfigProvider) chess_config_provider);
}

static void chess_window_unload(Window *window) {
  if (team_won == 0) {
    persist_write_data(BOARD_STATE_KEY, board_state, sizeof(board_state));
    persist_write_int(CASTLE_RIGHT_KEY, white_castle_possible_right);
    persist_write_int(CASTLE_LEFT_KEY, white_castle_possible_left);
    persist_write_int(MOVE_NO_KEY, move_count);
  } else {
    init_board_state();
    persist_write_data(BOARD_STATE_KEY, board_state, sizeof(board_state));
    persist_write_int(CASTLE_RIGHT_KEY, 1);
    persist_write_int(CASTLE_LEFT_KEY, 1);
    persist_write_int(MOVE_NO_KEY, 0);
    team_won = 0;
  }
  
  destroy_bitmaps();
  layer_destroy(s_chess_layer);
  window_destroy(s_chess_window);
}
  
void chess_init() {
  // Create main Window element and assign to pointer
  s_chess_window = window_create();
  
  window_set_background_color(s_chess_window, GColorWhite);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_chess_window, (WindowHandlers) {
    .load = chess_window_load,
    .unload = chess_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_chess_window, true);
}