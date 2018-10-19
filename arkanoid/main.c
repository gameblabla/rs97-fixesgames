#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif
#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

// Arkanoid Like.
// Code: 17o2!!
// Contact: Clement CORDE, c1702@yahoo.com

// Credits for the material used here:
//
// Some of the graphics were ripped from the ST version.
// Backgrounds taken from the StrategyWiki online arcade screenshots (http://strategywiki.org/wiki/Arkanoid/Walkthrough).
// This means some of the graphics are probably the property of Taito.
// As there is no profit intended, I hope I won't get sewed.
//
// Font found on Daniel Guldkrans's website (http://www.algonet.se/~guld1/freefont.htm), and slightly retouched by me.
//
// All additional graphics by me.
//

#include "includes.h"

// Variables générales.
struct SGene gVar;
struct SExg gExg;

unsigned char done = 1;
unsigned short time_start = 0;
SDL_Surface *ScreenSurface;

// Gestionnaire d'évènements.
int EventHandler(u32 nInGame)
{
	SDL_Event event;
	static u32	nLastPhase;

	while (SDL_PollEvent(&event))
	{
		int nWait=0;
		if (nWait) nWait--;
		switch (event.type)
		{
		case SDL_KEYDOWN:
			gVar.pKeys = SDL_GetKeyState(NULL);

			// Toggle fullscreen/windowed.
			if (gVar.pKeys[SDLK_F10])
			{
				SDL_Surface *pTmp = gVar.pScreen;
				u8	nToggle = gVar.nScreenMode ^ 1;

				gVar.pScreen = SDL_SetVideoMode(SCR_Width, SCR_Height, 8, SDL_HWSURFACE | SDL_ANYFORMAT | (nToggle ? SDL_FULLSCREEN : 0));
				if (gVar.pScreen == NULL)
				{
					// Raté.
					fprintf(stderr, "Couldn't set video mode: %sn",SDL_GetError());
					gVar.pScreen = pTmp;	// Récupère l'ancien.
				}
				else
				{
					// Ok.
					SDL_SetPalette(gVar.pScreen, SDL_LOGPAL | SDL_PHYSPAL, gVar.pColors, 0, 256);
					SDL_FreeSurface(pTmp);	// Libère l'ancien.
					gVar.nScreenMode = nToggle;	// Update flag.
				}
			}

			//if (gVar.pKeys[SDLK_ESCAPE] & gVar.pKeys[SDLK_BACKSPACE] & gVar.pKeys[SDLK_TAB]) return (1);	// Emergency exit.
			if(gVar.pKeys[51]){ // exit for retrogame
				return (1);
			}

			// Gestion de la pause.
			// TODO DINGUX <- Fixed, kinda
			// HAXXXXXX DAMNIT
			if (nInGame == 1 && (gVar.pKeys[SDLK_RETURN] && !gBreak.nTimerLevelDisplay && gBreak.nPhase != e_Game_GameOver)  & (nWait==0))
			{
				nWait=10;
				if (gBreak.nPhase == e_Game_Pause)
				{
					// On sort de la pause.
					gBreak.nPhase = nLastPhase;
					// Cache le pointeur de la souris.
					SDL_ShowCursor(SDL_DISABLE);
					// Replace la souris à l'endroit du joueur.
					SDL_WarpMouse(gBreak.nPlayerPosX, gBreak.nPlayerPosY);
					
				}
				else
				{
					// On passe en pause.
					nLastPhase = gBreak.nPhase;
					gBreak.nPhase = e_Game_Pause;

					// Affichage du texte de pause. En pause, plus de Flip !
					{	
						// On se remet sur le buffer précédent.
						//SDL_BlitSurface(gVar.pScreen, NULL, gVar.hwscreen, NULL);
						//SDL_UpdateRect(gVar.hwscreen, 0, 0, SCR_Width, SCR_Height);
						//SDL_Flip(gVar.pScreen);
						// Affichage du texte.
						char	pStrPause[] = "PAUSE";
						u32 i = Font_Print(0, 10, pStrPause, FONT_NoDisp);	// Pour centrage.
						Font_Print((SCR_Width / 2) - (i / 2), 123, pStrPause, 0);
						SprDisplayAll();
						//SDL_BlitSurface(gVar.pScreen, NULL, gVar.hwscreen, NULL);
						//SDL_UpdateRect(gVar.hwscreen, 0, 0, SCR_Width, SCR_Height);
						// On remet sur le buffer à l'écran.
						//SDL_Flip(gVar.pScreen);
						
						SDL_BlitSurface(gVar.pScreen, NULL, gVar.hwscreen, NULL);
						
						//SDL_UpdateRect(gVar.hwscreen, 0, 0, SCR_Width, SCR_Height);
						{
						  /*if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
						  int x, y;
						  uint32_t *s = (uint32_t*)gVar.hwscreen->pixels;
						  uint32_t *d = (uint32_t*)ScreenSurface->pixels;
						  for(y=0; y<240; y++){
							for(x=0; x<160; x++){
							  *d++ = *s++;
							}
							d+= 160;
						  }
						  if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);*/
						  SDL_SoftStretch(gVar.hwscreen, NULL, ScreenSurface, NULL);
						  SDL_Flip(ScreenSurface);
						}
						gVar.pKeys[SDLK_RETURN] = 0;
						while(done)
						{
							SDL_Event eventa;
							SDL_PollEvent(&eventa);
							time_start++;
							switch(eventa.type)
							{
								case SDL_KEYDOWN:
								switch(eventa.key.keysym.sym)
								{
									case SDLK_RETURN:
										if (time_start > 60) done = 0;
									break;
									case SDLK_LCTRL:
									case SDLK_LALT:
									case SDLK_ESCAPE:
										done = 0;
									break;
								}
								break;
							}
						}
						done = 1;
						time_start = 0;
						gBreak.nPhase = nLastPhase;
					}
					SDL_ShowCursor(SDL_DISABLE);		// Cache le pointeur de la souris.
				}
			}
			
			break;

		case SDL_KEYUP:
			gVar.pKeys = SDL_GetKeyState(NULL);
			break;

		case SDL_MOUSEMOTION:
			gVar.nMousePosX = event.motion.x;
			gVar.nMousePosY = event.motion.y;
			break;

		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
				gVar.nMouseButtons |= MOUSE_BtnLeft;
				break;

			default:
				break;
			}

			break;

		case SDL_QUIT:		// Fermeture de la fenêtre.
			Mix_CloseAudio();
			return (1);
			break;
		}
	}
	return (0);
}


// Création de la palette :
// On recopie dans la palette générale la partie de palette correspondant au décor +
// la palette des sprites.
void SetPalette(SDL_Surface *pBkg, SDL_Color *pSprPal, u32 nSprPalIdx)
{
	u32	i;
	SDL_Color	*pSrcPal = pBkg->format->palette->colors;

	// Couleurs du décor.
	for (i = 0; i < nSprPalIdx; i++)
	{
		gVar.pColors[i] = pSrcPal[i];
	}

	// Couleurs des sprites.
	for (; i < 256; i++)
	{
		gVar.pColors[i] = pSprPal[i - SPR_Palette_Idx];
	}

	// Palette logique.
	SDL_SetPalette(gVar.pScreen, SDL_LOGPAL, gVar.pColors, 0, 256);

	//SDL_SetPalette(gVar.pScreen, SDL_LOGPAL | SDL_PHYSPAL, gVar.pColors, 0, 256);

}


// Le Menu (générique).
u32 Menu(void (*pFctInit)(void), u32 (*pFctMain)(void))
{
	u32	nMenuVal = MENU_Null;

	gVar.pBackground = gVar.pBkg[0];		// Décor par défaut.
	gVar.pBkgRect = NULL;					// Par défaut, NULL (toute la surface).

	(*pFctInit)();
	// Sets up palette.
	SetPalette(gVar.pBackground, gVar.pSprColors, SPR_Palette_Idx);

	// Main loop.
	gVar.pKeys = SDL_GetKeyState(NULL);		// Lecture dans le vide, pour init du ptr.
	FrameInit();
	while (nMenuVal == MENU_Null)
	{
		// Gestion des évenements.
		gVar.nMouseButtons = 0;		// Raz mouse buttons.
		//EventHandler();
		if (EventHandler(0) != 0) { nMenuVal = MENU_Quit; break; }

		// Recopie le décor.
		if (SDL_BlitSurface(gVar.pBackground, gVar.pBkgRect, gVar.pScreen, NULL) < 0) fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());

		// Menu Main.
		nMenuVal = (*pFctMain)();

		SprDisplayAll();
		// Wait for frame.
		FrameWait();
		//SDL_Flip(gVar.pScreen);
		Fade(gVar.nFadeVal);
		SDL_BlitSurface(gVar.pScreen, NULL, gVar.hwscreen, NULL);
		//SDL_UpdateRect(gVar.hwscreen, 0, 0, SCR_Width, SCR_Height);
    {
      /*if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
      int x, y;
      uint32_t *s = (uint32_t*)gVar.hwscreen->pixels;
      uint32_t *d = (uint32_t*)ScreenSurface->pixels;
      for(y=0; y<240; y++){
        for(x=0; x<160; x++){
          *d++ = *s++;
        }
        d+= 160;
      }
      if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);*/
      SDL_SoftStretch(gVar.hwscreen, NULL, ScreenSurface, NULL);
      SDL_Flip(ScreenSurface);
    }
	}

	return (nMenuVal);

}


// La boucle de jeu.
void Game(void)
{


	// Cinématique d'intro.
//todo:...


	// Init.
	ExgBrkInit();
	// Sets up palette (Même palette pour tous les niveaux).
	SetPalette(gVar.pLev[0], gVar.pSprColors, SPR_Palette_Idx);
	//>> (Mettre le fader ?)
	SDL_FillRect(gVar.pScreen, NULL, 0);	// Clear screen.
	gVar.nFadeVal = 256;
	Fade(gVar.nFadeVal);					// Remet la palette physique.
	//<<

	// Main loop.
	gVar.pKeys = SDL_GetKeyState(NULL);		// Lecture dans le vide, pour init du ptr.
	FrameInit();
	while (gExg.nExitCode == 0)
	{
		
		// Gestion des évenements.
		gVar.nMouseButtons = 0;		// Raz mouse buttons.
		if (EventHandler(1) != 0) break;

		// Copie de l'image de fond.
        if (SDL_BlitSurface(gVar.pLevel, NULL, gVar.pScreen, NULL) < 0) fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());

		Breaker();
		SprDisplayAll();

		// Wait for frame.
		FrameWait();
		if (gBreak.nPhase != e_Game_Pause) {
      SDL_Flip(gVar.pScreen);
    }
		//Fade(gVar.nFadeVal);

	SDL_BlitSurface(gVar.pScreen, NULL, gVar.hwscreen, NULL);
	//SDL_UpdateRect(gVar.hwscreen, 0, 0, SCR_Width, SCR_Height);
    {
      /*if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
      int x, y;
      uint32_t *s = (uint32_t*)gVar.hwscreen->pixels;
      uint32_t *d = (uint32_t*)ScreenSurface->pixels;
      for(y=0; y<240; y++){
        for(x=0; x<160; x++){
          *d++ = *s++;
        }
        d+= 160;
      }
      if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);*/
      SDL_SoftStretch(gVar.hwscreen, NULL, ScreenSurface, NULL);
      SDL_Flip(ScreenSurface);
    }
	}


	// Si jeu terminé, cinématique de fin.
//	if (gExg.nExitCode == e_Game_AllClear) {}
//todo:...

	//TODO DINGUX
	// High score ?
	/*if (gExg.nExitCode == e_Game_GameOver || gExg.nExitCode == e_Game_AllClear)
	{
		if (Scr_CheckHighSc(gExg.nScore) >= 0)
		{
			// Saisie du nom.
			Menu(MenuGetName_Init, MenuGetName_Main);
			// Affichage de la table des high scores.
			Menu(MenuHighScores_Init, MenuHighScores_Main);
		}
	}*/

}


// Point d'entrée.
int main(int argc, char *argv[])
{
	u32	nLoop;
	u32	nMenuVal;
	u32	i;

	assert(MstCheckStructSizes() == 0);		// Debug : Vérifie la taille des structures spécifiques des monstres.

	// SDL Init.
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}
	// atexit : Quand on quittera (exit, return...), SDL_Quit() sera appelée.
	atexit(SDL_Quit);
	
	 //Initialisation de SDL_mixer
   	 if( Mix_OpenAudio( 11025, MIX_DEFAULT_FORMAT, 2, 256 ) == -1 )
	// Video mode init.
	gVar.nScreenMode = 0; 
	//gVar.pScreen = SDL_SetVideoMode(SCR_Width, SCR_Height, 8, SDL_HWSURFACE | SDL_DOUBLEBUF);
	//gVar.pScreen = SDL_SetVideoMode(SCR_Width, SCR_Height, 16, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
	gVar.pScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCR_Width, SCR_Height, 8, 0, 0, 0, 0);
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_SWSURFACE);
	SDL_ShowCursor(SDL_DISABLE);
	gVar.hwscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCR_Width, SCR_Height, 16, 0, 0, 0, 0);
	if (gVar.pScreen == NULL)
	{
		fprintf(stderr, "Couldn't set video mode: %sn",SDL_GetError());
		exit(1);
	}

	// Preca Sinus et Cosinus.
	PrecaSinCos();

	// Load sprites.
	SprInitEngine();
	SprLoadBMP("gfx/bricks.bmp", gVar.pSprColors, SPR_Palette_Idx);
	SprLoadBMP("gfx/font_small.bmp", NULL, 0);

	// Load levels backgound pictures.
	char	*pBkgLevFilenames[GFX_NbBkg] = { "gfx/lev1.bmp", "gfx/lev2.bmp", "gfx/lev3.bmp", "gfx/lev4.bmp", "gfx/levdoh.bmp" };
	for (i = 0; i < GFX_NbBkg; i++)
	{
		if ((gVar.pLev[i] = SDL_LoadBMP(pBkgLevFilenames[i])) == NULL) {
			fprintf(stderr, "Couldn't load picture '%s' : %s\n", pBkgLevFilenames[i], SDL_GetError());
			exit(1);
		}
	}
	gVar.pLevel = gVar.pLev[0];

	// Load menus backgound pictures.
	char	*pBkgMenFilenames[MENU_NbBkg] = { "gfx/bkg1.bmp", "gfx/bkg2.bmp" };
	for (i = 0; i < MENU_NbBkg; i++)
	{
		if ((gVar.pBkg[i] = SDL_LoadBMP(pBkgMenFilenames[i])) == NULL) {
			fprintf(stderr, "Couldn't load picture '%s' : %s\n", pBkgMenFilenames[i], SDL_GetError());
			exit(1);
		}
	}
	gVar.pBackground = gVar.pBkg[0];


	MenuInit();
	// Lecture de la table des high-scores.
	Scr_Load();

	SDL_ShowCursor(SDL_DISABLE);	// Cache le pointeur de la souris.


	// Boucle infinie.
	nMenuVal = MENU_Main;//MENU_Game;
	nLoop = 1;
	while (nLoop)
	{
		switch (nMenuVal)
		{
		case MENU_Main :	// Main menu.
			nMenuVal = Menu(MenuMain_Init, MenuMain_Main);
			break;

		case MENU_Game :	// Jeu.
			Game();
			nMenuVal = MENU_Main;
			break;

		case MENU_HallOfFame :	// High scores.
			Menu(MenuHighScores_Init, MenuHighScores_Main);
			nMenuVal = MENU_Main;
			break;

		case MENU_Quit :	// Sortie.
			nLoop = 0;
			break;
		}

	}


	SDL_ShowCursor(1);		// Réautorise l'affichage du curseur de la souris.

	// Libère les ressources des sprites.
	SprRelease();

	// Free the allocated surfaces.
	SDL_FreeSurface(gVar.pScreen);
	for (i = 0; i < GFX_NbBkg; i++)
	{
		SDL_FreeSurface(gVar.pLev[i]);
	}
	for (i = 0; i < MENU_NbBkg; i++)
	{
		SDL_FreeSurface(gVar.pBkg[i]);
	}

	return (0);

}



