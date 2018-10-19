/* sdloshw.c: Top-level SDL management functions.
 *
 * Copyright (C) 2001-2006 by Brian Raiter, under the GNU General Public
 * License. No warranty. See COPYING for details.
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<SDL.h>
#include	"sdlgen.h"
#include	"../err.h"
#include "port_cfg.h"



/* Values global to this library.
 */
oshwglobals	sdlg;

//DKS - modified
/* This is an automatically-generated file, which contains a
 * representation of the program's icon.
 */
#if !defined(PLATFORM_GP2X) && !defined(PLATFORM_GCW)
#include	"ccicon.c"
#endif

//DKS - modified
/* Dispatch all events sitting in the SDL event queue.
 */
static void _eventupdate(int wait)
{


	static int	mouselastx = -1, mouselasty = -1;
	SDL_Event	event;

	if (wait) {

		SDL_WaitEvent(NULL);
		int eventoccured = 0;
		while (!eventoccured) {
			eventoccured = SDL_PollEvent(NULL);
			if (!eventoccured) {
				SDL_Delay(10);
			}
		}
	} 


	SDL_PumpEvents();
	while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_ALLEVENTS)) {
		switch (event.type) {
			case SDL_KEYDOWN:
				if (windowmappos(mouselastx, mouselasty) < 0)
#if !defined(PLATFORM_GP2X) && !defined(PLATFORM_GCW)
					SDL_ShowCursor(SDL_DISABLE);
#endif
				keyeventcallback(event.key.keysym.sym, TRUE);
				if (event.key.keysym.unicode
						&& event.key.keysym.unicode != event.key.keysym.sym) {
					keyeventcallback(event.key.keysym.unicode, TRUE);
					keyeventcallback(event.key.keysym.unicode, FALSE);
				}
				break;
			case SDL_KEYUP:
				if (windowmappos(mouselastx, mouselasty) < 0)
#if !defined(PLATFORM_GP2X) && !defined(PLATFORM_GCW)
					SDL_ShowCursor(SDL_DISABLE);
#endif
				keyeventcallback(event.key.keysym.sym, FALSE);
				break;

#ifdef PLATFORM_GP2X
				//new joystick stuff
			case SDL_JOYBUTTONDOWN:
				if (windowmappos(mouselastx, mouselasty) < 0)
					joybuttoneventcallback(event.jbutton.button, TRUE);
				break;
			case SDL_JOYBUTTONUP:
				if (windowmappos(mouselastx, mouselasty) < 0)
					joybuttoneventcallback(event.jbutton.button, FALSE);
				break;
#endif

#ifdef PLATFORM_GCW
				// handle movement of analog nub (joy0)
			case SDL_JOYAXISMOTION:
				if (!port_cfg_settings.analog_enabled)
					break;
				switch (event.jaxis.axis)
				{
					case 0:		//axis 0 (left or right)
						if (event.jaxis.value < -sdlg.deadzone)
						{
							//left movement
							keyeventcallback(SDLK_LEFT, TRUE);
							keyeventcallback(SDLK_RIGHT, FALSE);
						} else if (event.jaxis.value > sdlg.deadzone)
						{
							//right movement
							keyeventcallback(SDLK_LEFT, FALSE);
							keyeventcallback(SDLK_RIGHT, TRUE);
						} else
						{
							// no left or right movement
							keyeventcallback(SDLK_LEFT, FALSE);
							keyeventcallback(SDLK_RIGHT, FALSE);
						}
						break;
					case 1:	// axis 1 (up or down)
						if (event.jaxis.value < -sdlg.deadzone)
						{
							//up movement
							keyeventcallback(SDLK_UP, TRUE);
							keyeventcallback(SDLK_DOWN, FALSE);
						} else if (event.jaxis.value > sdlg.deadzone)
						{
							//down movement
							keyeventcallback(SDLK_UP, FALSE);
							keyeventcallback(SDLK_DOWN, TRUE);
						} else
						{
							// no up or down movement
							keyeventcallback(SDLK_UP, FALSE);
							keyeventcallback(SDLK_DOWN, FALSE);
						}
						break;
					default:
						break;
				}
				break;
#endif

#if !defined(PLATFORM_GP2X) && !defined(PLATFORM_GCW)
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				SDL_ShowCursor(SDL_ENABLE);
				mouselastx = event.motion.x;
				mouselasty = event.motion.y;
				mouseeventcallback(event.button.x, event.button.y,
						event.button.button,
						event.type == SDL_MOUSEBUTTONDOWN);
				break;
			case SDL_MOUSEMOTION:
				SDL_ShowCursor(SDL_ENABLE);
				mouselastx = event.motion.x;
				mouselasty = event.motion.y;
				break;
#endif // PLATFORM_GP2X/GCW
			case SDL_QUIT:
				exit(EXIT_SUCCESS);
		}
	}
}


//DKS - modified
/* Alter the window decoration.
 */
void setsubtitle(char const *subtitle)
{
#ifdef PLATFORM_PC
	char	buf[270];

	if (subtitle && *subtitle) {
		sprintf(buf, "Tile World - %.255s", subtitle);
		SDL_WM_SetCaption(buf, "Tile World");
	} else {
		SDL_WM_SetCaption("Tile World", "Tile World");
	}
#endif //PLATFORM_PC
}

//DKS - modified, we're going to explicity shutdown SDL, not with atexit()
/* Shut down SDL.
 */
static void shutdown(void)
{
	//    SDL_Quit();
}




//DKS - modified
/* Initialize SDL, create the program's icon, and then initialize
 * the other modules of the library.
 */
int oshwinitialize(int silence, int soundbufsize,
		int showhistogram, int fullscreen)
{
	SDL_Surface	       *icon;

	sdlg.eventupdatefunc = _eventupdate;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		errmsg(NULL, "Cannot initialize SDL system: %s\n", SDL_GetError());
		return FALSE;
	}
	//atexit(shutdown);

#ifndef PLATFORM_PC
	SDL_ShowCursor(SDL_DISABLE);
#endif // PLATFORM_PC

#ifdef PLATFORM_PC
	setsubtitle(NULL);
	icon = SDL_CreateRGBSurfaceFrom(cciconimage, CXCCICON, CYCCICON,
			32, 4 * CXCCICON,
			0x0000FF, 0x00FF00, 0xFF0000, 0);
	if (icon) {
		SDL_WM_SetIcon(icon, cciconmask);
		SDL_FreeSurface(icon);
	} else
		warn("couldn't create icon surface: %s", SDL_GetError());
#endif //PLATFORM_PC

	return _sdltimerinitialize(showhistogram)
		&& _sdltextinitialize()
		&& _sdltileinitialize()
		&& _sdlinputinitialize()
		&& _sdloutputinitialize(fullscreen)
		&& _sdlsfxinitialize(silence, soundbufsize);

}

//DKS - modified
void controlleddelay(int milsecs)
{
	SDL_Delay(milsecs);
}


/* The real main().
 */
int main(int argc, char *argv[])
{
	return tworld(argc, argv);
}
