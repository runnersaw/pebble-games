#include <pebble.h>
#include "windowHandler.h"
#include "menuHandler.h"
#include "food.h"
#include "tennis.h"
#include "chess.h"
  
void food_chosen() {
  food_init();
}
  
void tennis_chosen() {
  tennis_init();
}
  
void chess_chosen() {
  chess_init();
}

int main() {
  menu_init();
  app_event_loop();
  menu_deinit();
}