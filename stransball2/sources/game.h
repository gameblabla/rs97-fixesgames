#ifndef __SUPER_TRANSBALL_GAME
#define __SUPER_TRANSBALL_GAME

bool gamecycle(SDL_Surface *screen,int sx,int sy);

bool state_logo_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_mainmenu_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_instructions_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_typetext_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_chooseship_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_interphase_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_game_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_replaymanager_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_replay_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_gameover_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_endsequence_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_keyredefinition_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_changepack_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);
bool state_levelfinished_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard);


#endif
