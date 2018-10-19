// Apricots main program
// Author: M.D.Snellgrove
// Date: 17/3/2002
// History:

// Changed by M Snellgrove 3/8/2003
// Conditional compilation switch for Cygwin (thanks judebert)

#include "apricots.h"
#include "menu.h"

enum GameStateType GameState;
struct gamedata newg;
bool quit;
bool demo;
char *ap_home = NULL;

// Definition of Main
// Cygwin expects a WinMain function
#ifdef CYGWIN
WINAPI int WinMain(HINSTANCE, HINSTANCE, LPSTR, int){
#else
int main(int, char**){
#endif

  // Initialize data
  gamedata g;
  init_data(g);
  init_data(newg);

  // Setup game
  setup_game(g);
  menuLoadAll(g);

  GameState = STATE_INTRO;
  demo = true;

  // Enter main loop
  game(g);

  // Finish game
  finish_game(g);

  // Shutdown
  menuDeleteAll();
  SDL_Quit();
  g.sound.close();
  
  return 0;

}
