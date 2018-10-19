
#ifndef NO_MENU

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "menu.h"

#include "vkbd/vkbd.h"

#include "sound/sound.h"

#ifdef DREAMCAST
#include<SDL_dreamcast.h>
#endif

#include "tvfilter/tvfilter.h"

extern int emulating;
extern int changed_fdc0;

static char *text_str_title="DCaSTaway rc8";
static char *text_str_load="Filemanager (X)";
static char *text_str_save="SaveStates (Y)";
static char *text_str_disk="Save Disks";
static char *text_str_0="0";
static char *text_str_50="50";
static char *text_str_100="100";
static char *text_str_150="150";
static char *text_str_200="200";
static char *text_str_frameskip="Frameskip";
static char *text_str_1="1";
static char *text_str_2="2";
static char *text_str_3="3";
static char *text_str_4="4";
static char *text_str_5="5";
static char *text_str_auto="auto";
static char *text_str_sound="Sound";
static char *text_str_on="on";
static char *text_str_off="off";
static char *text_str_vsync="VSync";
static char *text_str_separator="~~~~~~~~~~~~~~~~~~~~~~";
static char *text_str_reset="Reset (L)";
static char *text_str_run="Run (R)";
#ifdef DREAMCAST
static char *text_str_exit= "Reboot Dreamcast";
#else
static char *text_str_exit="Exit";
#endif

int mainMenu_frameskip=-1;
int mainMenu_sound=1;
int mainMenu_case=-1;
int mainMenu_vsync=0;
int mainMenu_savedisk=-1;

enum { MAIN_MENU_CASE_REBOOT, MAIN_MENU_CASE_LOAD, MAIN_MENU_CASE_RUN, MAIN_MENU_CASE_RESET, MAIN_MENU_CASE_CANCEL, MAIN_MENU_CASE_SAVE };

static  void draw_mainMenu(int c)
{
//	static int b=0;
//	int bb=(b%6)/3;

	text_draw_background();
	text_draw_window(40,24,260,200,text_str_title);
//	write_text(6,3,text_str_separator);

	if ((c==0)) //&&(bb))
		write_text_sel(6,4,252,text_str_load);
	else
		write_text(6,4,text_str_load);

	write_text(6,5,text_str_separator);
	write_text(6,6,text_str_separator);

	if ((c==1)) //&&(bb))
		write_text_sel(6,7,252,text_str_save);
	else
		write_text(6,7,text_str_save);

	write_text(6,8,text_str_separator);
	
	if (c==2)
	{
		write_text_sel(6,10,252,text_str_frameskip);
		switch(mainMenu_frameskip)
		{
			case 0: write_text_inv(19,10,"||||");break;
			case 1: write_text_inv(21,10,"||||"); break;
			case 2: write_text_inv(23,10,"||||"); break;
			case 3: write_text_inv(25,10,"||||"); break;
			case 4: write_text_inv(27,10,"||||"); break;
			case 5: write_text_inv(29,10,"||||"); break;
			default:
				write_text_inv(31,10,"||||||||||||||");
		}
		write_text_inv(19,10,text_str_0);
		write_text_inv(21,10,text_str_1);
		write_text_inv(23,10,text_str_2);
		write_text_inv(25,10,text_str_3);
		write_text_inv(27,10,text_str_4);
		write_text_inv(29,10,text_str_5);
		write_text_inv(31,10,text_str_auto);
		
	}
	else
	{
		write_text(6,10,text_str_frameskip);
		switch(mainMenu_frameskip)
		{
			case 0: write_text(19,10,"||||"); break;
			case 1: write_text(21,10,"||||"); break;
			case 2: write_text(23,10,"||||"); break;
			case 3: write_text(25,10,"||||"); break;
			case 4: write_text(27,10,"||||"); break;
			case 5: write_text(29,10,"||||"); break;
			default:
				write_text(31,10,"||||||||||||||");
		}
		write_text(19,10,text_str_0);
		write_text(21,10,text_str_1);
		write_text(23,10,text_str_2);
		write_text(25,10,text_str_3);
		write_text(27,10,text_str_4);
		write_text(29,10,text_str_5);
		write_text(31,10,text_str_auto);
	}

	if (c==3)
	{
		write_text_sel(6,12,252,text_str_sound);
		if (mainMenu_sound)
			write_text_inv(24,12,"|||||||");
		else
			write_text_inv(19,12,"|||||||||");

		write_text_inv(19,12,text_str_off);
		write_text_inv(24,12,text_str_on);
	}
	else
	{
		write_text(6,12,text_str_sound);
		if (mainMenu_sound)
			write_text(24,12,"|||||||");
		else
			write_text(19,12,"|||||||||");

		write_text(19,12,text_str_off);
		write_text(24,12,text_str_on);
	}

	if (c==4)
	{
		write_text_sel(6,14,252,text_str_vsync);
		if (mainMenu_vsync)
			write_text_inv(24,14,"|||||||");
		else
			write_text_inv(19,14,"|||||||||");
		write_text_inv(19,14,text_str_off);
		write_text_inv(24,14,text_str_on);
	}
	else
	{
		write_text(6,14,text_str_vsync);
		if (mainMenu_vsync)
			write_text(24,14,"|||||||");
		else
			write_text(19,14,"|||||||||");
		write_text(19,14,text_str_off);
		write_text(24,14,text_str_on);
	}
	
	if (c==5)
	{
		write_text_sel(6,16,252,text_str_disk);
		if (mainMenu_savedisk)
			write_text_inv(24,16,"|||||||");
		else
			write_text_inv(19,16,"|||||||||");
		write_text_inv(19,16,text_str_off);
		write_text_inv(24,16,text_str_on);
	}
	else
	{
		write_text(6,16,text_str_disk);
		if (mainMenu_savedisk)
			write_text(24,16,"|||||||");
		else
			write_text(19,16,"|||||||||");
		write_text(19,16,text_str_off);
		write_text(24,16,text_str_on);
	}
	
	write_text(6,18,text_str_separator);
	if ((c==6)) //&&(bb))
		write_text_sel(6,19,252,text_str_reset);
	else
		write_text(6,19,text_str_reset);
	write_text(6,20,text_str_separator);

	write_text(6,21,text_str_separator);
	if ((c==7)) //&&(bb))
		write_text_sel(6,22,252,text_str_run);
	else
		write_text(6,22,text_str_run);

	write_text(6,23,text_str_separator);

	write_text(6,24,text_str_separator);
	if ((c==8)) //&&(bb))
		write_text_sel(6,25,252,text_str_exit);
	else
		write_text(6,25,text_str_exit);
//	write_text(6,26,text_str_separator);

	text_flip();
//	b++;
}

static  int key_mainMenu(int *cp)
{
	int c=(*cp);
	int back_c=-1;
	int end=0;
	int left=0, right=0, up=0, down=0;
	int hit0=0, hit1=0, hit2=0, hit3=0, hit4=0, hit5=0;
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		if (event.type == SDL_QUIT)
		{
			mainMenu_case=MAIN_MENU_CASE_REBOOT;
			end=-1;
		}
		else
		if (event.type == SDL_VIDEORESIZE)
			TV_ResizeWindow(event.resize.w, event.resize.h);
		else
		if (event.type == SDL_KEYDOWN)
		{
			audio_play_click();
			switch(event.key.keysym.sym)
			{
				case SDLK_d:
				case SDLK_RIGHT: right=1; break;
				case SDLK_a:
				case SDLK_LEFT: left=1; break;
				case SDLK_w:
				case SDLK_UP: up=1; break;
				case SDLK_s:
				case SDLK_DOWN: down=1; break;
				case SDLK_z:
				case SDLK_RETURN:
				case SDLK_e:
				case SDLK_LCTRL: hit0=1; break;
#ifdef DREAMCAST
				case SDLK_c:
				case SDLK_LSHIFT: hit3=1; break;
				case SDLK_x:
				case SDLK_SPACE: hit4=1; break;
				case SDLK_2:
				case SDLK_BACKSPACE: hit2=1; break;
				case SDLK_1:
				case SDLK_TAB: hit5=1; break;
#else
				case SDLK_c:
				case SDLK_LSHIFT: hit4=1; break;
				case SDLK_x:
				case SDLK_SPACE: hit3=1; break;
				case SDLK_2:
				case SDLK_BACKSPACE: hit5=1; break;
				case SDLK_1:
				case SDLK_TAB: hit2=1; break;
#endif
				case SDLK_q:
				case SDLK_LALT: hit1=1; break;
				case SDLK_F12: TV_ToggleFullScreen(screen); break;
			}
			if (hit1)
			{
				mainMenu_case=MAIN_MENU_CASE_CANCEL;
				end=1;
			}
			else if (hit2)
			{
				back_c=c;
				hit0=1;
				c=6;
			}
			else if (hit3)
			{
				mainMenu_case=MAIN_MENU_CASE_LOAD;
				end=1;
			}
			else if (hit4)
			{
				mainMenu_case=MAIN_MENU_CASE_SAVE;
				end=1;
			}
			else if (hit5)
			{
				back_c=c;
				hit0=1;
				c=7;
			}
			else if (up)
			{
				if (c>0) c=(c-1)%9;
				else c=8;
			}
			else if (down)
				c=(c+1)%9;
			switch(c)
			{
				case 0:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_LOAD;
						end=1;
					}
					break;
				case 1:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_SAVE;
						end=1;
					}
					break;
				case 2:
					if (left)
					{
						if (mainMenu_frameskip>-1)
							mainMenu_frameskip--;
						else
							mainMenu_frameskip=5;
					}
					else if (right)
					{
						if (mainMenu_frameskip<5)
							mainMenu_frameskip++;
						else
							mainMenu_frameskip=-1;
					}
					break;
				case 3:
					if (left)
					{
						if (mainMenu_sound)
							mainMenu_sound=0;
					}
					else if (right)
					{
						if (!mainMenu_sound)
							mainMenu_sound=1;
					}
					break;
				case 4:
					if (left)
					{
						if (mainMenu_vsync)
							mainMenu_vsync=0;
					}
					else if (right)
					{
						if (!mainMenu_vsync)
							mainMenu_vsync=1;
					}
					break;
				case 5:
					if (left)
					{
						if (mainMenu_savedisk)
							mainMenu_savedisk=0;
					}
					else if (right)
					{
						if (!mainMenu_savedisk)
							mainMenu_savedisk=-1;
					}
					break;
				case 6:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_RESET;
						end=1;
					}
					break;
				case 7:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_RUN;
						end=1;
					}
					break;
				case 8:
					if (hit0)
					{
						mainMenu_case=MAIN_MENU_CASE_REBOOT;
						end=1;
					}
					break;
			}
		}
		if (back_c>=0)
		{
			c=back_c;
			back_c=-1;
		}
	}


	(*cp)=c;
	return end;
}

static void clear_events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
		SDL_Delay(20);
}

static  void raise_mainMenu()
{
	int i;

	audio_play_main();
	text_draw_background();
	text_flip();
	for(i=0;i<10;i++)
	{
		text_draw_background();
		text_draw_window(140-(10*i),(10-i)*24,160+(10*i),200,"");
		text_flip();
		SDL_Delay(15);
	}
	clear_events();
}

static  void unraise_mainMenu()
{
	int i;

	for(i=9;i>=0;i--)
	{
		text_draw_background();
		text_draw_window(140-(10*i),(10-i)*24,160+(10*i),200,"");
		text_flip();
		SDL_Delay(15);
	}
	text_draw_background();
	text_flip();
	clear_events();
}

void drawNoRom(void)
{
	int i;
	SDL_Event ev;

	for(i=0;i<20;i++)
	{
//		menu_moving=0;
		text_draw_background();
		text_draw_window(50,96,250,64,"ERROR !");
		write_text(14,14,"ROM not found");
#ifdef DREAMCAST
		write_text(8,16,"Press any button to retry");
#else
		write_text(8,16,"Press any button to exit");
#endif
		text_flip();
	}
	SDL_Delay(333);
    	while(SDL_PollEvent(&ev))
		if (ev.type==SDL_QUIT)
			exit(1);
		else
		    SDL_Delay(10);
    	while(!SDL_PollEvent(&ev))
		    SDL_Delay(10);
    	while(SDL_PollEvent(&ev))
		if (ev.type==SDL_QUIT)
			exit(1);
	text_draw_background();
	text_flip();
	SDL_Delay(333);
}

int run_mainMenu()
{
	static int c=0;
	int end;
	mainMenu_case=-1;
#ifdef DREAMCAST
	SDL_DC_VerticalWait(SDL_FALSE);
#endif

#ifdef DEBUG_FAME
	mainMenu_vsync=0;
	mainMenu_frameskip=0;
	mainMenu_sound=0;
	mainMenu_case=1;
	mainMenu_vsync=0;
	mainMenu_savedisk=0;
#else
#ifdef AUTOLOAD
	mainMenu_case=1;
#endif
#endif
	while(mainMenu_case<0)
	{
		raise_mainMenu();
		end=0;
		while(!end)
		{
			draw_mainMenu(c);
			end=key_mainMenu(&c);
		}
		unraise_mainMenu();
		switch(mainMenu_case)
		{
			case MAIN_MENU_CASE_LOAD:
				run_menuLoad();	
				mainMenu_case=-1;
				break;
			case MAIN_MENU_CASE_SAVE:
				run_saveMenu();
				mainMenu_case=-1;
				break;
			case MAIN_MENU_CASE_RUN:
				mainMenu_case=1;
				break;
			case MAIN_MENU_CASE_RESET:
				if (emulating)
					mainMenu_case=2;
				else
					mainMenu_case=1;
				break;
			case MAIN_MENU_CASE_REBOOT:
				audio_play_goodbye();
				menu_unraise();
				SDL_Delay(333);
				TV_Quit();
				SDL_Quit();
#ifdef DREAMCAST
				arch_reboot();
#else
				exit(0);
#endif
				break;
			case MAIN_MENU_CASE_CANCEL:
				if (emulating)
					mainMenu_case=1;
				else
					mainMenu_case=-1;
				break;
			default:
				mainMenu_case=-1;
		}
	}

	text_draw_window(96,64,140,32,"-------");
//	menu_moving=0;
	if (changed_fdc0)
		audio_play_wait();
	write_text(14,9,"Please wait");
	text_flip();

#ifdef DREAMCAST
	SDL_DC_VerticalWait((SDL_bool)mainMenu_vsync);
#endif
	vkbd_init_button2();
	return mainMenu_case;
}
#endif
