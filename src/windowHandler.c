#include <pebble.h>
#include "windowHandler.h"
#include "menuHandler.h"
#include "snake.h"
#include "pong.h"
#include "chess.h"
  
void snake_chosen() {
  snake_init();
}
  
void pong_chosen() {
  pong_init();
}
  
void chess_chosen() {
  chess_init();
}

int main() {
  menu_init();
  app_event_loop();
  menu_deinit();
}