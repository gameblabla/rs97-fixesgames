#include <SDL/SDL.h>
#include "misc.h"
#include "render.h"
#include "update.h"
#include "input.h"
#include "menu.h"


int FRAMES_PER_SECOND=60;
int GameState = GAME_INIT;

Uint32 lastTime = 0, frameTime = 0;


bool Init(){
	if(!InitVideo()) {
		return false;
	}
	if(!InitSound()) {
		return false;
	}
	if(!InitGfx()) {
		return false;
	}
	
	return true;
}




void CleanUp(){
	CleanUpVideo();
	CleanUpSound();
	SDL_Quit();
}

void waitframe(void)
{
    static Uint32   next_tick = 0;
    Uint32          this_tick;

    /*
     * Wait for the next frame
     */
    this_tick = SDL_GetTicks();
    if (this_tick < next_tick) {
        SDL_Delay(next_tick - this_tick);
    }
    next_tick = this_tick + (1000 / FRAMES_PER_SECOND);
}




// Main Begins Here
int main() {
	int runUntil;
	while (GameState != GAME_EXIT){
		switch (GameState) {
			
		case GAME_INIT:
			if (Init() == false) { return -1; } //if you cannot get the resources you need then quit
			PlayIntroMusic();
			GameState = GAME_MENU;
			break;
		case GAME_MENU:
			GetInput();
			UpdateMenu();  //menu decides what happens next
			RenderMenu();
			waitframe();
			break;
		case GAME_START:
			Init_Game();
			GameState = LEVEL_START;
			break;
		case LEVEL_START:
			StopMusic();
			PlayGameMusic();
			Next_Level();
			runUntil = SDL_GetTicks()+(1000*1);
			while (SDL_GetTicks()< runUntil ) {
				UpdateStart();
				RenderStart();
				waitframe();
			}
			GameState = GAME_RUN;
			break;
		case GAME_RUN:
			GetInput();
			update();
			Render();
			waitframe();
			break;
		case LEVEL_END:
			StopMusic();
			PlayGameOverMusic();
			playLevelComplete();
			runUntil = SDL_GetTicks()+(10000);
			while (SDL_GetTicks()< runUntil) {
				updateLevelOver();
				RenderLevelOver();
				waitframe();
			}
			GameState = LEVEL_START;
			break;
		case GAME_END:
			StopMusic();
			PlayGameOverMusic();
			playGameOver();
			GameState = GAME_OVER;
			break;
		case GAME_OVER:
			runUntil = SDL_GetTicks()+(2000);
			while (SDL_GetTicks()< runUntil ) {
				RenderGameOver();
				waitframe();
			}
			GameState = GAME_MENU;
			break;
		}
	}
	CleanUp();
	
	
	return 0;
}



