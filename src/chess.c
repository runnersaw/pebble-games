#include <pebble.h>
#include "chess.h"
  
#define WIDTH 120
#define BOX_BORDER 2
  
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
  
// TODO: program castle and pawn conversion
  
static Window *s_chess_window;
static Layer *s_chess_layer;
static Layer *s_chess_indicator_layer;
static Layer *s_chess_container_layer;

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

static void create_bitmaps() {
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
}

static void destroy_bitmaps() {
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

static void generate_black_pawn_threaten(short state[8][8], short x, short y) {
  if (get_piece_at_position(state, x+1, y+1)>0) {
    if (is_valid(x+1, y+1)==1) {
      test_possible_moves[y+1][x+1] = 1;
    }
  }
  if (get_piece_at_position(state, x-1, y+1)>0) {
    if (is_valid(x-1, y+1)==1) {
      test_possible_moves[y+1][x-1] = 1;
    }
  }
  if (get_piece_at_position(state, x, y+1)==0) { // none
    if (is_valid(x, y+1)==1) {
      test_possible_moves[y+1][x] = 1;
      if (y==1) {
        if (get_piece_at_position(state, x, y+2)==0) {
          if (is_valid(x, y+2)==1) {
            test_possible_moves[y+2][x] = 1;
          }
        }
      }
    }
  }
}

static void generate_white_pawn_threaten(short state[8][8], short x, short y) {
  if (get_piece_at_position(state, x+1, y-1)<0) {
    if (is_valid(x+1, y-1)==1) {
      test_possible_moves[y-1][x+1] = 1;
    }
  }
  if (get_piece_at_position(state, x-1, y-1)<0) {
    if (is_valid(x-1, y-1)==1) {
      test_possible_moves[y-1][x-1] = 1;
    }
  }
  if (get_piece_at_position(state, x, y-1)==0) { // none
    if (is_valid(x, y-1)==1) {
      test_possible_moves[y-1][x] = 1;
      if (y==6) {
        if (get_piece_at_position(state, x, y-2)==0) {
          if (is_valid(x, y-2)==1) {
            test_possible_moves[y-2][x] = 1;
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
          generate_black_pawn_threaten(state, k, l);
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
          generate_white_pawn_threaten(state, k, l);
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

static short test_move(short start_x, short start_y, short end_x, short end_y, short team) { // 1 is white, 0 is black
  if (is_valid(end_x, end_y)==0) {
    return 0;
  }
  copy_state(test_board_state, board_state);
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

static void generate_moves(short state[8][8], short team) {
  short temp_possible_moves[8][8];
  reset_state(possible_moves);
  reset_state(possible_pieces_to_move);
  short x,y;
  for (x=0;x<8;x++) {
    for (y=0;y<8;y++) {
      reset_state(test_possible_moves);
      if (team==BLACK_TEAM) {
        if (get_piece_at_position(state, x,y)==BLACK_PAWN) {
          generate_black_pawn_threaten(state, x, y);        
        } else if (get_piece_at_position(state, x,y)==BLACK_KNIGHT) {
          generate_knight_threaten(state, x, y, BLACK_TEAM);
        } else if (get_piece_at_position(state, x,y)==BLACK_ROOK) {
          generate_rook_threaten(state, x, y, BLACK_TEAM);        
        } else if (get_piece_at_position(state, x,y)==BLACK_QUEEN) {
          generate_queen_threaten(state, x, y, BLACK_TEAM);
        } else if (get_piece_at_position(state, x,y)==BLACK_BISHOP) {
          generate_bishop_threaten(state, x, y, BLACK_TEAM);
        } else if (get_piece_at_position(state, x,y)==BLACK_KING) {
          generate_king_threaten(state, x, y, BLACK_TEAM);
        }
      } else {
        if (get_piece_at_position(state, x,y)==WHITE_KNIGHT) {
          generate_knight_threaten(state, x, y, WHITE_TEAM);        
        } else if (get_piece_at_position(state, x,y)==WHITE_PAWN) {
          generate_white_pawn_threaten(state, x, y);
        } else if (get_piece_at_position(state, x,y)==WHITE_ROOK) {
          generate_rook_threaten(state, x, y, WHITE_TEAM);
        } else if (get_piece_at_position(state, x,y)==WHITE_QUEEN) {
          generate_queen_threaten(state, x, y, WHITE_TEAM);
        } else if (get_piece_at_position(state, x,y)==WHITE_BISHOP) {
          generate_bishop_threaten(state, x, y, WHITE_TEAM);
        } else if (get_piece_at_position(state, x,y)==WHITE_KING) {
          generate_king_threaten(state, x, y, WHITE_TEAM);
        }
      }
      copy_state(temp_possible_moves,test_possible_moves);
      short k,l;
      for (k=0;k<8;k++) {
        for (l=0;l<8;l++) {
          if (temp_possible_moves[l][k]==1) {
            if (test_move(x, y, k, l, team)==1) {
              possible_moves[l][k]=1;
              possible_pieces_to_move[y][x]=1;
            }
          }
        }
      }
    }
  }
}

static void generate_move(short state[8][8], short x, short y) {
  short temp_possible_moves[8][8];
  short piece, team;
  reset_state(possible_moves);
  reset_state(possible_pieces_to_move);
  reset_state(test_possible_moves);
  piece=get_piece_at_position(state, x,y);
  if (piece>0) {
    team = WHITE_TEAM;
  } else {
    team = BLACK_TEAM;
  }
  if (piece==BLACK_PAWN) {
    generate_black_pawn_threaten(state, x, y);
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
    generate_white_pawn_threaten(state, x, y);
  } else if (piece==WHITE_ROOK) {
    generate_rook_threaten(state, x, y, WHITE_TEAM);
  } else if (piece==WHITE_QUEEN) {
    generate_queen_threaten(state, x, y, WHITE_TEAM);
  } else if (piece==WHITE_BISHOP) {
    generate_bishop_threaten(state, x, y, WHITE_TEAM);
  } else if (piece==WHITE_KING) {
    generate_king_threaten(state, x, y, WHITE_TEAM);
  }
  copy_state(temp_possible_moves,test_possible_moves);
  short k,l;
  for (k=0;k<8;k++) {
    for (l=0;l<8;l++) {
      if (temp_possible_moves[l][k]==1) {
        if (test_move(x, y, k, l, team)==1) {
          possible_moves[l][k]=1;
          possible_pieces_to_move[y][x]=1;
        }
      }
    }
  }
}

static void move(short state[8][8], short start_x, short start_y, short end_x, short end_y) {
  state[end_y][end_x] = state[start_y][start_x];
  state[start_y][start_x] = 0;
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
  short m,n;
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
  short m,n;
  if (x==0||x==7||y==0||y==7) {
    score += 25*team;
  } else {
    score += 32*team;
  }
  return score;
}

short find_bishop_value(short state[8][8], short x,short y,short team) {
  short score=0;
  short m,n;
  score+=33*team;
  return score;
}

short find_rook_value(short state[8][8], short x,short y,short team) {
  short score=0;
  short m,n;
  score+=51*team;
  return score;
}

short find_queen_value(short state[8][8], short x,short y,short team) {
  short score=0;
  short m,n;
  score+=88*team;
  return score;
}

short find_king_value(short state[8][8], short x,short y,short team) {
  short score=0;
  short m,n;
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
    move(board_state,6,1,6,2);
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

static void change_turn() {
  turn=-turn;
  move_count++;
  if (turn==BLACK_TEAM) {
    find_move(board_state, turn, SEARCH_DEPTH);
  }
}

static void draw_chess(Layer *layer, GContext *ctx) {
  short m,n;
  for (m=0;m<8;m++) {
    for (n=0;n<8;n++) {
      if ((m+n)%2==1) { // black square
        if (board_state[m][n]>0) { // is white
          graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
          if (board_state[m][n] == WHITE_PAWN) {
            graphics_draw_bitmap_in_rect(ctx, s_black_pawn_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == WHITE_ROOK) {
            graphics_draw_bitmap_in_rect(ctx, s_black_rook_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == WHITE_QUEEN) {
            graphics_draw_bitmap_in_rect(ctx, s_black_queen_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == WHITE_BISHOP) {
            graphics_draw_bitmap_in_rect(ctx, s_black_bishop_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == WHITE_KNIGHT) {
            graphics_draw_bitmap_in_rect(ctx, s_black_knight_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == WHITE_KING) {
            graphics_draw_bitmap_in_rect(ctx, s_black_king_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          }
        } else { //black
          graphics_context_set_compositing_mode(ctx, GCompOpAssign);
          if (board_state[m][n] == BLACK_PAWN) {
            graphics_draw_bitmap_in_rect(ctx, s_black_pawn_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == BLACK_BISHOP) {
            graphics_draw_bitmap_in_rect(ctx, s_black_bishop_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == BLACK_QUEEN) {
            graphics_draw_bitmap_in_rect(ctx, s_black_queen_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == BLACK_KING) {
            graphics_draw_bitmap_in_rect(ctx, s_black_king_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == BLACK_KNIGHT) {
            graphics_draw_bitmap_in_rect(ctx, s_black_knight_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == BLACK_ROOK) {
            graphics_draw_bitmap_in_rect(ctx, s_black_rook_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          }
        }
      } else { // white square
        if (board_state[m][n]>0) {
          graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
          if (board_state[m][n] == WHITE_PAWN) {
            graphics_draw_bitmap_in_rect(ctx, s_black_pawn_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == WHITE_ROOK) {
            graphics_draw_bitmap_in_rect(ctx, s_black_rook_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == WHITE_QUEEN) {
            graphics_draw_bitmap_in_rect(ctx, s_black_queen_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == WHITE_BISHOP) {
            graphics_draw_bitmap_in_rect(ctx, s_black_bishop_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == WHITE_KNIGHT) {
            graphics_draw_bitmap_in_rect(ctx, s_black_knight_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == WHITE_KING) {
            graphics_draw_bitmap_in_rect(ctx, s_black_king_black, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          }
        } else { 
          graphics_context_set_compositing_mode(ctx, GCompOpAssign);
          if (board_state[m][n] == BLACK_PAWN) {
            graphics_draw_bitmap_in_rect(ctx, s_black_pawn_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == BLACK_BISHOP) {
            graphics_draw_bitmap_in_rect(ctx, s_black_bishop_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == BLACK_QUEEN) {
            graphics_draw_bitmap_in_rect(ctx, s_black_queen_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == BLACK_KING) {
            graphics_draw_bitmap_in_rect(ctx, s_black_king_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == BLACK_KNIGHT) {
            graphics_draw_bitmap_in_rect(ctx, s_black_knight_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          } else if (board_state[m][n] == BLACK_ROOK) {
            graphics_draw_bitmap_in_rect(ctx, s_black_rook_white, GRect(n*WIDTH/8,m*WIDTH/8,WIDTH/8,WIDTH/8));
          }
        }
      }
    }
  }
}

static void draw_indicator(Layer *layer, GContext *ctx) {
  if (display_box==2) {
    
  } else if (display_box==1) {
    if ((selected_x+selected_y)%2==1) { // black square
      graphics_context_set_stroke_color(ctx, GColorWhite);
    } else {
      graphics_context_set_stroke_color(ctx, GColorBlack);
    }
    graphics_draw_rect(ctx, GRect(selected_x*WIDTH/8+BOX_BORDER,selected_y*WIDTH/8+BOX_BORDER+12,WIDTH/8-2*BOX_BORDER,WIDTH/8-2*BOX_BORDER));
  } else {
    graphics_draw_bitmap_in_rect(ctx, s_down_arrow, GRect(selected_x*WIDTH/8+1,0,12,12));
  }
}

static void draw_chess_container(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, GRect(0, 0, WIDTH, WIDTH));
  graphics_context_set_fill_color(ctx, GColorBlack);
  short i, j;
  for (i=0;i<4;i++) {
    for (j=0;j<4;j++) {
      graphics_fill_rect(ctx, GRect(i*WIDTH/4,(2*j+1)*WIDTH/8,WIDTH/8,WIDTH/8), 0, GCornerNone);
    }
  }
  for (i=0;i<4;i++) {
    for (j=0;j<4;j++) {
      graphics_fill_rect(ctx, GRect((2*i+1)*WIDTH/8,j*WIDTH/4,WIDTH/8,WIDTH/8), 0, GCornerNone);
    }
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
  layer_mark_dirty(s_chess_indicator_layer);
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
  layer_mark_dirty(s_chess_indicator_layer);
}

static void select_handler() {
  if (selected == 0) { // means hasn't selected a piece to move yet
    if (on_x == 1) { // means hasn't selected x yet, selecting now
      on_x=0;
      find_next_y_piece();
      display_box = 1;
      layer_mark_dirty(s_chess_indicator_layer);
    } else { // selected y, lock in piece to move
      selected = 1;
      selected_piece_x = selected_x;
      selected_piece_y = selected_y;
      generate_move(board_state, selected_x, selected_y);
      selected_x = 0;
      selected_y = 0;
      find_next_move();
      on_x=1;
    }
  } else { // has selected piece, just selected place to move
    move(board_state, selected_piece_x, selected_piece_y, selected_x, selected_y);
    change_turn();
    generate_moves(board_state, turn);
    find_next_x_piece();
    find_next_y_piece();
    layer_mark_dirty(s_chess_layer);
    selected = 0;
    display_box=0;
  }
  layer_mark_dirty(s_chess_indicator_layer);
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
  }
  layer_mark_dirty(s_chess_indicator_layer);
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
  
  turn = WHITE_TEAM; //white is 1, black is -1
  init_board_state();
  
  reset_state(possible_moves);
  reset_state(test_possible_moves);
  reset_state(possible_pieces_to_move);
  generate_moves(board_state, turn);
  
  layer_mark_dirty(s_chess_layer);
  layer_mark_dirty(s_chess_indicator_layer);
  layer_mark_dirty(s_chess_container_layer);
}

void chess_config_provider(Window *window) {
  // set click listeners
  window_single_click_subscribe(BUTTON_ID_UP, up_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, chess_reset, NULL);
}

static void chess_window_load(Window *window) {
  create_bitmaps();
  
  s_chess_indicator_layer = layer_create(GRect(12, 0, WIDTH+1, WIDTH+13));
  s_chess_container_layer = layer_create(GRect(12, 12, WIDTH+1, WIDTH+1));
  s_chess_layer = layer_create(GRect(0, 0, WIDTH+1, WIDTH+1));
  
  layer_set_update_proc(s_chess_layer, draw_chess);
  layer_set_update_proc(s_chess_indicator_layer, draw_indicator);
  layer_set_update_proc(s_chess_container_layer, draw_chess_container);
  layer_add_child(window_get_root_layer(s_chess_window), s_chess_container_layer);
  layer_add_child(window_get_root_layer(s_chess_window), s_chess_indicator_layer);
  layer_add_child(s_chess_container_layer, s_chess_layer);
  
  init_board_state();
  
  if (persist_exists(BOARD_STATE_KEY)) {
    persist_read_data(BOARD_STATE_KEY, board_state, sizeof(board_state));
  }
  
  reset_state(possible_moves);
  reset_state(test_possible_moves);
  reset_state(possible_pieces_to_move);
  generate_moves(board_state, turn);
  window_set_click_config_provider(window, (ClickConfigProvider) chess_config_provider);
}

static void chess_window_unload(Window *window) {
  persist_write_data(BOARD_STATE_KEY, board_state, sizeof(board_state));
  
  destroy_bitmaps();
  layer_destroy(s_chess_layer);
  layer_destroy(s_chess_indicator_layer);
  layer_destroy(s_chess_container_layer);
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