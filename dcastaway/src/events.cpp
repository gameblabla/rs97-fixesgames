#include "dcastaway.h"

#include "menu.h"

#include "vkbd.h"
#include "keymap.h"
#include "config.h"
#include "st.h"
#include "mem.h"
#include "m68k/m68k_intrf.h"
#include "sound/sound.h"

static SDL_Joystick *jstick[2]={NULL,NULL};

#ifndef NO_MENU
extern int mainMenu_frameskip;
extern int mainMenu_sound;
#else
int mainMenu_frameskip=-1;
int mainMenu_sound=1;
#endif

extern int maxframeskip;
extern int mouse_mul;

#ifdef DREAMCAST
extern int  __sdl_dc_emulate_keyboard;
extern int  __sdl_dc_emulate_mouse;
#endif

extern char *dcastaway_image_file;
extern char *dcastaway_image_file2;

extern int fdc_commands_executed;
extern Uint32 autoframeskip_next;

extern int mainMenu_savedisk;

int emulated_mouse=0;

int changed_fdc0;
int changed_fdc1;

void show_please_wait_loading(SDL_Surface *);
void show_please_wait_saving(SDL_Surface *);

#define JOY_UP       1
#define JOY_DOWN     2
#define JOY_LEFT     4
#define JOY_RIGHT    8
#define JOY_BUTTON 128

extern int saveMenu_n_savestate;

static int emulated_mouse_speed=4;

static void inc_mouse_speed(void)
{
    char n[40];
    if (emulated_mouse_speed<16)
	emulated_mouse_speed++;
    sprintf((char *)&n[0],"Mouse %i speed",emulated_mouse_speed);
//printf("x=%i, y=%i, xrel=%i, yrel=%i\n",event.motion.x*mouse_mul, event.motion.y*mouse_mul, event.motion.xrel*mouse_mul, event.motion.yrel*mouse_mul);
    set_message((char *)&n[0], 40);
}

static void dec_mouse_speed(void)
{
    char n[40];
    if (emulated_mouse_speed>1)
	emulated_mouse_speed--;
    sprintf((char *)&n[0],"Mouse %i speed",emulated_mouse_speed);
    set_message((char *)&n[0], 40);
}

static void inc_n_savestate(void)
{
    char n[40];
    saveMenu_n_savestate=(saveMenu_n_savestate+1)%4;
    sprintf((char *)&n[0],"Savestate %i selected",saveMenu_n_savestate);
    set_message((char *)&n[0], 40);
}

static void dec_n_savestate(void)
{
    char n[40];
    if (saveMenu_n_savestate)
    	saveMenu_n_savestate--;
    else
	saveMenu_n_savestate=3;
    sprintf((char *)&n[0],"Savestate %i selected",saveMenu_n_savestate);
    set_message((char *)&n[0], 40);
}

#ifdef DINGOO
static void show_mhz(void)
{
    extern int dingoo_get_clock(void);
    char n[40];
    sprintf((char *)&n[0],"Dingoo at %iMHz",dingoo_get_clock());
    set_message((char *)&n[0], 50);
}

static void show_brightness(void)
{
    extern int dingoo_get_brightness(void);
    char n[40];
    sprintf((char *)&n[0],"Brightness %i%%",dingoo_get_brightness());
    set_message((char *)&n[0], 40);
}

static void show_volumen(void)
{
    extern int dingoo_get_volumen(void);
    char n[40];
    sprintf((char *)&n[0],"Volumen %i%%",dingoo_get_volumen());
    set_message((char *)&n[0], 40);
}

static void inc_dingoo_mhz(void)
{
	extern void dingoo_set_clock(unsigned int);
	extern unsigned int dingoo_get_clock(void);
	dingoo_set_clock(dingoo_get_clock()+25);
	show_mhz();
}

static void dec_dingoo_mhz(void)
{
	extern void dingoo_set_clock(unsigned int);
	extern unsigned int dingoo_get_clock(void);
	dingoo_set_clock(dingoo_get_clock()-25);
	show_mhz();
}

static void inc_dingoo_brightness(void)
{
	extern void dingoo_set_brightness(int);
	extern int dingoo_get_brightness(void);
	dingoo_set_brightness(dingoo_get_brightness()+5);
	show_brightness();
}

static void dec_dingoo_brightness(void)
{
	extern void dingoo_set_brightness(int);
	extern int dingoo_get_brightness(void);
	dingoo_set_brightness(dingoo_get_brightness()-5);
	show_brightness();
}

static void inc_dingoo_volumen(void)
{
	extern void dingoo_set_volumen(int);
	extern int dingoo_get_volumen(void);
	dingoo_set_volumen(dingoo_get_volumen()+5);
	show_volumen();
}

static void dec_dingoo_volumen(void)
{
	extern void dingoo_set_volumen(int);
	extern int dingoo_get_volumen(void);
	dingoo_set_volumen(dingoo_get_volumen()-5);
	show_volumen();
}

#else
#define show_mhz()
#define inc_dingoo_mhz()
#define dec_dingoo_mhz()
#define inc_dingoo_brightness()
#define dec_dingoo_brightness()
#define inc_dingoo_volumen()
#define dec_dingoo_volumen()
#endif

void goMenu(void){
	int exitmode;
	int autosave=mainMenu_savedisk;

#ifdef DEBUG_FRAMESKIP
	print_frameskip();
#endif
#ifndef NO_RENDER
	change_redraw_to_menu();
#endif
	changed_fdc0=0;
	changed_fdc1=0;
    	audio_stop();
#ifdef DREAMCAST
	vkbd_quit();
#endif
	render_no_border();
	init_text(0);
	if (!mainMenu_sound)
		SDL_PauseAudio(0);
	exitmode=run_mainMenu();
	if ((!changed_fdc0)&&(!changed_fdc1))
		menu_unraise();
	quit_text();
	maxframeskip=mainMenu_frameskip;
#ifdef DREAMCAST
        __sdl_dc_emulate_keyboard=0;
#endif
	if (changed_fdc0)
		FDCInit(0);
	if (changed_fdc1)
		FDCInit(1);
	render_blank_screen();
	fdc_commands_executed=0;
#ifndef NO_MENU
	if (exitmode==2)
#endif
	{
		if (autosave!=mainMenu_savedisk)
		{
			FDCInit(0);
			FDCInit(1);
		}
//		emulating=0;
		MemReInit();
		IOInit();
  		Ikbd_Reset();
    		HWReset();
	}
#if defined(AUTOSAVESTATE) && defined(NO_MENU)
	loadstate_vmu(0);
#endif
#ifdef DREAMCAST
        vkbd_init();
#endif
	render_down_screen();
	if (mainMenu_sound)
	{
		Sound_Update_VBL();
    		audio_init();
	}
    	else
		SDL_PauseAudio(1);
	events_init();
	reset_frameskip();
	vid_flag=1;
}

static int nowSuperThrottle=0, goingSuperThrottle=0;
#ifndef  DINGOO
static int goingMenu=0;
#endif
static int back_mainMenu_sound=0;

static void goSuperThrottle(void)
{
	if (!nowSuperThrottle)
	{
		back_mainMenu_sound=mainMenu_sound;
		mainMenu_sound=0;
		maxframeskip=80;
		if (back_mainMenu_sound)
			audio_stop();
		nowSuperThrottle=1;
		set_message("SuperThrottle On",50);
	}
}

static void leftSuperThrottle(void)
{
	if (nowSuperThrottle)
	{
		mainMenu_sound=back_mainMenu_sound;
		if (mainMenu_sound)
		{
			Sound_Update_VBL();
    			audio_init();
		}
		maxframeskip=mainMenu_frameskip;
		reset_frameskip();
		nowSuperThrottle=0;
		set_message("SuperThrottle Off",50);
	}
}

static void swapDisk(void)
{
	char *tmp=(char *)calloc(1,128);
	strcpy(tmp,dcastaway_image_file);
	strcpy(dcastaway_image_file,dcastaway_image_file2);
	strcpy(dcastaway_image_file2,tmp);
	free(tmp);
	FDCInit(1);
	FDCInit(0);
}

void events_init(void)
{
     int it;
#ifdef DREAMCAST
    __sdl_dc_emulate_keyboard=0;
#endif
    SDL_JoystickEventState(SDL_ENABLE);
    if (jstick[0]==NULL)
 	   jstick[0]=SDL_JoystickOpen(0);
    if (jstick[1]==NULL)
 	   jstick[1]=SDL_JoystickOpen(1);
    for(it=0;it<20;it++)
    {
	    SDL_Event event;
	    while( SDL_PollEvent(&event) )
		    SDL_Delay(5);
	    SDL_Delay(15);
    }
    {
	static int yet=0;
	if (!yet)
	{
    		show_mhz();
		yet++;
	}
    }
}

void do_events (void) 
{
#if !defined(DREAMCAST) && !defined(DINGOO)
   static int resized=0,resized_w=320,resized_h=240;
#endif
#if !defined(DEBUG_FAME) && !defined(AUTOEVENTS)
    static int display_vkbd;
    static int joystateold[2]={0,0};
    int hat,jsn=0,joystate[2];
    SDL_Event event;
    SDLKey sym;
    static Uint32 ticks=(Uint32)-1;
    Uint32 now=SDL_GetTicks();
#ifdef EMULATED_JOYSTICK 
    static int return_pressed=0;
    static int escape_pressed=0;
    static int tab_pressed=0;
    static int backspace_pressed=0;
    static int emulated_mouse_up=0;
    static int emulated_mouse_down=0;
    static int emulated_mouse_left=0;
    static int emulated_mouse_right=0;
#endif

    joystate[0]=joystateold[0];
    joystate[1]=joystateold[1];
#ifndef NO_VKBD
    if (vkbd_mode)
    {
	if (draw_border)
		vkbd_redraw();
//	maybe_border=0;
    	if (now-ticks>100)
    	{
	    render_vkbd_background();
	    vkbd_key=vkbd_process();
	    ticks=now;
    	}
    }
#endif

    /* Just process events, if there */
    while( SDL_PollEvent(&event) ) {

        switch(event.type) {
	case SDL_JOYBUTTONDOWN:
		jsn=event.jbutton.which;
		switch(event.jbutton.button)
		{
#ifndef DREAMCAST
			case 0:
			case 4:
			case 7:
#endif
			case 2:
#ifndef NO_VKBD
				if (vkbd_mode)
					vkbd_move=VKBD_BUTTON;
				else
#endif
					joystate[jsn] |= JOY_BUTTON;
				break;
#ifndef DREAMCAST
			case 1:
			case 3:
			case 5:
#endif
			case 6:
#ifndef NO_VKBD
				if (vkbd_mode)
					vkbd_move = VKBD_BUTTON2;
				else
					if (vkbd_button2)
            					IkbdKeyPress(keysymToAtari[vkbd_button2]);
					else
#endif
						joystate[jsn] |= JOY_UP;
				break;
#if defined(DREAMCAST) && !defined(NO_VKBD)
			case 5:
				if (vkbd_mode)
					vkbd_move = VKBD_BUTTON3;
				else
					if  (vkbd_button3)
						IkbdKeyPress(keysymToAtari[vkbd_button3]);
					else
						if (!__sdl_dc_emulate_mouse)
                					IkbdMousePress(2);
				break;
			case 1:

				if (vkbd_mode)
					vkbd_move = VKBD_BUTTON4;
				else
					if  (vkbd_button4)
						IkbdKeyPress(keysymToAtari[vkbd_button4]);
					else
						if (!__sdl_dc_emulate_mouse)
 			               			IkbdMousePress(1);
				break;
			case 3:
				goingMenu=1;

#endif
		}
		break;
	case SDL_JOYBUTTONUP:
		jsn=event.jbutton.which;
		switch(event.jbutton.button)
		{
#ifndef DREAMCAST
			case 0:
			case 4:
			case 7:
#endif
			case 2:
#ifndef NO_VKBD
				if (vkbd_mode)
					vkbd_move &= ~VKBD_BUTTON;
				else
#endif
					joystate[jsn] &= ~JOY_BUTTON;
				break;
#ifndef DREAMCAST
			case 1:
			case 3:
			case 5:
#endif
			case 6:
#ifndef NO_VKBD
				if (vkbd_mode)
					vkbd_move &= ~VKBD_BUTTON2;
				else
					if (vkbd_button2)
            					IkbdKeyRelease(keysymToAtari[vkbd_button2]);
					else
#endif
						joystate[jsn] &= ~JOY_UP;
				break;
#ifdef DREAMCAST
			case 3:
				switch(goingMenu) {
					case 2:
						if (mainMenu_sound)
							audio_stop();
						show_please_wait_loading(screen);
						if (loadstate(saveMenu_n_savestate))
							set_message("Failed: Savestate not found", 100);
						else
							set_message("Savestate loaded", 50);
						if (mainMenu_sound)
						{
							Sound_Update_VBL();
 			   				audio_init();
						}
						events_init();
						reset_frameskip();
						vid_flag=1;
						break;
					case 3:
						if (mainMenu_sound)
							audio_stop();
						show_please_wait_saving(screen);
						if (savestate(saveMenu_n_savestate))
							set_message("Failed Savestate", 100);
						else
							set_message("Savestate saved", 50);
						if (mainMenu_sound)
						{
							Sound_Update_VBL();
 			   				audio_init();
						}
						events_init();
						reset_frameskip();
						vid_flag=1;
						break;
					default:
						goMenu();
				}
				goingMenu=0;
				break;
#endif
#if defined(DREAMCAST) && !defined(NO_VKBD)
			case 5:
				if (vkbd_mode)
					vkbd_move &= ~VKBD_BUTTON3;
				else
					if (vkbd_button3)
            					IkbdKeyRelease(keysymToAtari[vkbd_button3]);
					else
						if (!__sdl_dc_emulate_mouse)
                					IkbdMouseRelease(2);
				break;
			case 1:
				if (vkbd_mode)
					vkbd_move &= ~VKBD_BUTTON4;
				else
					if (vkbd_button4)
            					IkbdKeyRelease(keysymToAtari[vkbd_button4]);
					else
						if (!__sdl_dc_emulate_mouse)
                					IkbdMouseRelease(1);
				break;

#endif
		}
		break;
#ifndef DINGOO
	case SDL_JOYAXISMOTION:
		switch(event.jaxis.axis)
		{
			case 3:
				if (goingMenu) {
					goingMenu=3;
					goingSuperThrottle=0;
					break;
				}
				if ((event.jaxis.value<=16)&&(nowSuperThrottle))
					leftSuperThrottle();
				else
				if ((goingSuperThrottle)
#ifndef NO_VKBD
						&&(!vkbd_mode)
#endif
				   )
					goSuperThrottle();
				else
				if ((event.jaxis.value>=16)&&(!nowSuperThrottle)
#ifndef NO_VKBD
						&&(!vkbd_mode)
#endif
				   )
				{
					if (!emulated_mouse)
					{
						render_force_background();
						render_up_screen();
#ifndef NO_VKBD
						vkbd_mode=1;
						vkbd_redraw();
#endif
					}
				}
				break;
			case 2:
#ifndef NO_VKBD
				if (goingMenu) {
					goingMenu=2;
					goingSuperThrottle=0;
					break;
				}
				if (vkbd_mode)
				{
					render_force_background();
					render_down_screen();
					vkbd_mode=0;
					goingSuperThrottle=0;
				}
				else
#endif
					goingSuperThrottle=(event.jaxis.value>16);
				break;
#ifndef DREAMCAST
			case 1:
				if (event.jaxis.value < -3200)
				{
					joystate[0] |= JOY_UP;
					joystate[0] &= ~JOY_DOWN;
				}
				else
				if (event.jaxis.value > 3200)
				{
					joystate[0] |= JOY_DOWN;
					joystate[0] &= ~JOY_UP;
				}
				else
				{
					joystate[0] &= ~JOY_UP;
					joystate[0] &= ~JOY_DOWN;
				}
				break;
			case 0:
				if (event.jaxis.value < -3200)
				{
					joystate[0] |= JOY_LEFT;
					joystate[0] &= ~JOY_RIGHT;
				}
				else
				if (event.jaxis.value > 3200)
				{
					joystate[0] |= JOY_RIGHT;
					joystate[0] &= ~JOY_LEFT;
				}
				else
				{
					joystate[0] &= ~JOY_LEFT;
					joystate[0] &= ~JOY_RIGHT;
				}
				break;
#endif
		}
		break;
	case SDL_JOYHATMOTION:
		jsn=event.jhat.which;
             	hat=SDL_JoystickGetHat(jstick[jsn],event.jhat.hat);
#ifndef NO_VKBD
		if (vkbd_mode)
		{
			if (hat & SDL_HAT_LEFT)
				vkbd_move |= VKBD_LEFT;
			else
			{
				vkbd_move &= ~VKBD_LEFT;
				if (hat & SDL_HAT_RIGHT)
					vkbd_move |= VKBD_RIGHT;
				else
					vkbd_move &= ~VKBD_RIGHT;
			}
			if (hat & SDL_HAT_UP)
				vkbd_move |= VKBD_UP;
			else
				vkbd_move &= ~VKBD_UP;
				if (hat & SDL_HAT_DOWN)
					vkbd_move |= VKBD_DOWN;
				else
					vkbd_move &= ~VKBD_DOWN;
		}
		else
#endif
		{
			if (hat & SDL_HAT_LEFT)
				joystate[jsn] |= JOY_LEFT;
			else 
			{
				joystate[jsn] &= ~JOY_LEFT;
				if (hat & SDL_HAT_RIGHT)
					joystate[jsn] |= JOY_RIGHT;
				else
					joystate[jsn] &= ~JOY_RIGHT;
			}
			if (hat & SDL_HAT_UP)
				joystate[jsn] |= JOY_UP;
			else
			{
				joystate[jsn] &= ~JOY_UP;
				if (hat & SDL_HAT_DOWN)
					joystate[jsn] |= JOY_DOWN;
				else
					joystate[jsn] &= ~JOY_DOWN;
			}
		}
		break;
#endif
        case SDL_KEYDOWN:
#ifndef DREAMCAST
	    if (event.key.keysym.sym==SDLK_F4 && (SDL_GetModState()&(KMOD_LALT|KMOD_RALT)))
		    exit(0);
#ifdef EMULATED_JOYSTICK
	    if (event.key.keysym.sym==SDLK_DOWN)
	    {
		if (escape_pressed)
			dec_dingoo_brightness();
		else
#ifndef NO_VKBD
		if (emulated_mouse)
			emulated_mouse_down=1;
		else
		if (vkbd_mode)
			vkbd_move|=VKBD_DOWN;
		else
#endif
		    joystate[0] |= JOY_DOWN;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_UP)
	    {
		if (escape_pressed)
			inc_dingoo_brightness();
		else
#ifndef NO_VKBD
#ifdef EMULATED_JOYSTICK 
		if (emulated_mouse)
			emulated_mouse_up=1;
		else
#endif
		if (vkbd_mode)
			vkbd_move|=VKBD_UP;
		else
#endif
		    joystate[0] |= JOY_UP;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_LEFT)
	    {
		if (escape_pressed)
			dec_dingoo_volumen();
		else
#ifndef NO_VKBD
		if (emulated_mouse)
			emulated_mouse_left=1;
		else
		if (vkbd_mode)
			vkbd_move|=VKBD_LEFT;
		else
#endif
		    joystate[0] |= JOY_LEFT;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_RIGHT)
	    {
		if (escape_pressed)
			inc_dingoo_volumen();
		else
#ifndef NO_VKBD
		if (emulated_mouse)
			emulated_mouse_right=1;
		else
		if (vkbd_mode)
			vkbd_move|=VKBD_RIGHT;
		else
#endif
		    joystate[0] |= JOY_RIGHT;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_LCTRL || event.key.keysym.sym==SDLK_RCTRL)
	    {
		if (escape_pressed)
			inc_dingoo_mhz();
		else
#ifndef NO_VKBD
		if (vkbd_mode)
			vkbd_move|=VKBD_BUTTON;
		else
#endif
		    joystate[0] |= JOY_BUTTON;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_LALT || event.key.keysym.sym==SDLK_RALT)
	    {
		if (escape_pressed)
			dec_dingoo_mhz();
		else
#ifndef NO_VKBD
		if (!vkbd_mode)
#endif
		{
#ifndef NO_VKBD
			if (vkbd_button2)
            			IkbdKeyPress(keysymToAtari[vkbd_button2]);
			else
#endif
				joystate[jsn] |= JOY_UP;
		}
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_LSHIFT || event.key.keysym.sym==SDLK_RSHIFT)
	    {
		if (escape_pressed)
		{
			if (emulated_mouse)
				dec_mouse_speed();
			else
				dec_n_savestate();
		}
		else
#ifndef NO_VKBD
		if (vkbd_mode)
			vkbd_move=VKBD_BUTTON4;
		else
		if (vkbd_button4)
			IkbdKeyPress(keysymToAtari[vkbd_button4]);
		else
#endif
             		IkbdMousePress(2);
		break;
// Si no pulsa el boton SHITF de ST
	    }
	    else
	    if (event.key.keysym.sym==SDLK_SPACE)
	    {
		if (escape_pressed)
		{
			if (emulated_mouse)
				inc_mouse_speed();
			else
				inc_n_savestate();
			break;
		}
		else
		if (emulated_mouse)
		{
             		IkbdMousePress(1);
			break;
		}
		else
#ifndef NO_VKBD
		if (vkbd_mode)
		{
			vkbd_move=VKBD_BUTTON3;
			break;
		}
		else
		if (vkbd_button3)
		{
			IkbdKeyPress(keysymToAtari[vkbd_button3]);
			break;
		}
#endif
// Si no pulsa el boton SPACE de ST
	    }
	    else
	    if (event.key.keysym.sym==SDLK_BACKSPACE)
	    {
		backspace_pressed=1;
		if (escape_pressed)
		{
// LOAD SNAP!
			if (mainMenu_sound)
				audio_stop();
			show_please_wait_loading(screen);
			if (loadstate(saveMenu_n_savestate))
				set_message("Failed: Savestate not found", 100);
			else
				set_message("Savestate loaded", 50);
			if (mainMenu_sound)
			{
				Sound_Update_VBL();
    				audio_init();
			}
			events_init();
			reset_frameskip();
			vid_flag=1;
			return_pressed=escape_pressed=tab_pressed=backspace_pressed=0;
		}
		else
		{
			if ((!nowSuperThrottle)
#ifndef NO_VKBD
				&&(!vkbd_mode)
#endif
			   )
			{
				if (!emulated_mouse)
				{
					render_force_background();
					render_up_screen();
#ifndef NO_VKBD
					vkbd_mode=1;
					vkbd_redraw();
#endif
				}
				else
				{
    					if (emulated_mouse_speed>=16)
						emulated_mouse_speed=0;
					inc_mouse_speed();
				}
			}
			else
			if (vkbd_mode)
			{
				render_force_background();
				render_down_screen();
				vkbd_mode=0;
				goingSuperThrottle=0;
			}
		}
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_TAB)
	    {
		tab_pressed=1;
		if (escape_pressed)
		{
// SAVE SNAP!
			if (mainMenu_sound)
				audio_stop();
			show_please_wait_saving(screen);
			if (savestate(saveMenu_n_savestate))
				set_message("Failed Savestate", 100);
			else
				set_message("Savestate saved", 50);
			if (mainMenu_sound)
			{
				Sound_Update_VBL();
    				audio_init();
			}
			events_init();
			reset_frameskip();
			vid_flag=1;
			return_pressed=escape_pressed=tab_pressed=backspace_pressed=0;
		}
		else
		{
			if (!vkbd_mode)
			{
				extern unsigned short render_pal16_copy0;
				emulated_mouse=~emulated_mouse;
				render_pal16_copy0--;
/*
				if (nowSuperThrottle)
					leftSuperThrottle();
				else
#ifndef NO_VKBD
				if (!vkbd_mode)
#endif
					goSuperThrottle();
*/
			}
		}
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_RETURN)
	    {
		return_pressed=1;
		if (escape_pressed)
		{
			goMenu();
			return_pressed=escape_pressed=tab_pressed=backspace_pressed=0;
		}
		else
		{
			if (nowSuperThrottle)
				leftSuperThrottle();
			else
				goSuperThrottle();
		}
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_ESCAPE)
	    {
		escape_pressed=1;
		if (backspace_pressed)
		{
			goMenu();
			return_pressed=escape_pressed=tab_pressed=backspace_pressed=0;
		}
		break;
	    }
#ifndef NO_VKBD
	    if (vkbd_mode)
		    break;
#endif
#endif
#ifndef DINGOO
	    if (event.key.keysym.sym==SDLK_PAGEUP)
		    goSuperThrottle();
	    else
	    if (event.key.keysym.sym==SDLK_PAGEDOWN)
		    swapDisk();
	    else
	    if (event.key.keysym.sym!=SDLK_F12 && event.key.keysym.sym!=SDLK_F11 && event.key.keysym.sym!=SDLK_LMETA && event.key.keysym.sym!=SDLK_RMETA)
#endif
#endif
            IkbdKeyPress(keysymToAtari[event.key.keysym.sym]);
            break;
        case SDL_KEYUP:
#ifndef DREAMCAST
#ifdef EMULATED_JOYSTICK
	    if (event.key.keysym.sym==SDLK_DOWN)
	    {
		if (escape_pressed)
			break;
#ifndef NO_VKBD
		if (emulated_mouse)
		{
			emulated_mouse_down=0;
			break;
		}
		else
		if (vkbd_mode)
		{
			vkbd_move &= ~VKBD_DOWN;
			break;
		}
		else
#endif
		    joystate[0] &= ~JOY_DOWN;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_UP)
	    {
		if (escape_pressed)
			break;
#ifndef NO_VKBD
#ifdef EMULATED_JOYSTICK 
		if (emulated_mouse)
		{
			emulated_mouse_up=0;
			break;
		}
		else
#endif
		if (vkbd_mode)
		{
			vkbd_move &= ~VKBD_UP;
			break;
		}
		else
#endif
		    joystate[0] &= ~JOY_UP;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_LEFT)
	    {
		if (escape_pressed)
			break;
#ifndef NO_VKBD
		if (emulated_mouse)
		{
			emulated_mouse_left=0;
			break;
		}
		else
		if (vkbd_mode)
		{
			vkbd_move &= ~VKBD_LEFT;
			break;
		}
		else
#endif
		    joystate[0] &= ~JOY_LEFT;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_RIGHT)
	    {
		if (escape_pressed)
			break;
#ifndef NO_VKBD
		if (emulated_mouse)
		{
			emulated_mouse_right=0;
			break;
		}
		else
		if (vkbd_mode)
		{
			vkbd_move &= ~VKBD_RIGHT;
			break;
		}
		else
#endif
		    joystate[0] &= ~JOY_RIGHT;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_LCTRL || event.key.keysym.sym==SDLK_RCTRL)
	    {
		if (escape_pressed)
			break;
#ifndef NO_VKBD
		if (vkbd_mode)
			vkbd_move &= ~VKBD_BUTTON;
		else
#endif
		    joystate[0] &= ~JOY_BUTTON;
		break;
	    }
	    else
		    
#endif
#ifdef EMULATED_JOYSTICK
	    if (event.key.keysym.sym==SDLK_SPACE)
	    {
		if (escape_pressed)
			break;
		if (emulated_mouse)
		{
             		IkbdMouseRelease(1);
			break;
		}
#ifndef NO_VKBD
		else
		if (vkbd_mode)
		{
			vkbd_move=VKBD_BUTTON3;
			break;
		}
		else
		if (vkbd_button3)
		{
            		IkbdKeyRelease(keysymToAtari[vkbd_button3]);
		}
#endif
	    }
	    else
	    if (event.key.keysym.sym==SDLK_LALT || event.key.keysym.sym==SDLK_RALT)
	    {
		if (escape_pressed)
			break;
#ifndef NO_VKBD
		if (vkbd_mode)
			vkbd_move=VKBD_BUTTON2;
		else
			if (vkbd_button2)
            			IkbdKeyRelease(keysymToAtari[vkbd_button2]);
			else
#endif
				joystate[jsn] &= ~JOY_UP;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_LSHIFT || event.key.keysym.sym==SDLK_RSHIFT)
	    {
		if (escape_pressed)
			break;
#ifndef NO_VKBD
		if (vkbd_mode)
			vkbd_move=VKBD_BUTTON4;
		else
		if (vkbd_button4)
            		IkbdKeyRelease(keysymToAtari[vkbd_button4]);
		else
#endif
             		IkbdMouseRelease(2);
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_BACKSPACE)
	    {
		backspace_pressed=0;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_TAB)
	    {
		tab_pressed=0;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_RETURN)
	    {
		return_pressed=0;
		break;
	    }
	    else
	    if (event.key.keysym.sym==SDLK_ESCAPE)
	    {
		escape_pressed=0;
		break;
	    }
#ifndef NO_VKBD
	    if (vkbd_mode)
		    break;
#endif
#endif
#ifndef DINGOO
	    if (event.key.keysym.sym==SDLK_F11 || event.key.keysym.sym==SDLK_LMETA || event.key.keysym.sym==SDLK_RMETA)
		    goMenu();
	    else
#ifndef NO_RENDER
	    if (event.key.keysym.sym==SDLK_F12)
		    video_fullscreen();
	    else
#endif
	    if (event.key.keysym.sym==SDLK_PAGEUP)
		    leftSuperThrottle();
	    else
#endif
#endif
            IkbdKeyRelease(keysymToAtari[event.key.keysym.sym]);
            break;
        case SDL_MOUSEMOTION:
//printf("x=%i, y=%i, xrel=%i, yrel=%i\n",event.motion.x*mouse_mul, event.motion.y*mouse_mul, event.motion.xrel*mouse_mul, event.motion.yrel*mouse_mul);
            IkbdMouseMotion(event.motion.x*mouse_mul, event.motion.y*mouse_mul, event.motion.xrel*mouse_mul, event.motion.yrel*mouse_mul);
            break;
        case SDL_MOUSEBUTTONDOWN:
#ifdef DREAMCAST
		if (__sdl_dc_emulate_mouse)
		{
#ifndef NO_VKBD
			if (!vkbd_mode)
#endif
			{

				if (event.button.button==5)
                			IkbdMousePress(2);
				else if (event.button.button==1)
                			IkbdMousePress(1);
			}
		}
		else
		if (!event.button.button)
#else
		if (event.button.button<=1)
#endif
			IkbdMousePress(2);
		else
			IkbdMousePress(1);
            break;
        case SDL_MOUSEBUTTONUP:
#ifdef DREAMCAST
		if (__sdl_dc_emulate_mouse)
		{
#ifndef NO_VKBD
			if (!vkbd_mode)
#endif
			{
				if (event.button.button==5)
                			IkbdMouseRelease(2);
				else if (event.button.button==1)
                			IkbdMouseRelease(1);
			}
		}
		else
		if (!event.button.button)
#else
		if (event.button.button<=1)
#endif
			IkbdMouseRelease(2);
		else
			IkbdMouseRelease(1);
            break;
#if !defined(DREAMCAST) && !defined(DINGOO)
	case SDL_VIDEORESIZE:
	    resized_w=event.resize.w;
	    resized_h=event.resize.h;
	    resized=40;
	    break;
#endif
        case SDL_QUIT:
            exit(0);
        }
    	if (joystateold[0]!=joystate[0])
    	{
#ifndef NO_VKBD
	    if (!vkbd_mode)
#endif
	    	IkbdJoystickChange(0,joystate[0]);
	    joystateold[0]=joystate[0];
    	}
    	if (joystateold[1]!=joystate[1])
    	{
#ifndef NO_VKBD
	    if (!vkbd_mode)
#endif
	    	IkbdJoystickChange(1,joystate[1]);
	    joystateold[1]=joystate[1];
    	}
    }
#ifndef NO_VKBD
#ifdef EMULATED_JOYSTICK 
    if (emulated_mouse)
    {
	    static unsigned mx=160;
	    static unsigned my=100;
	    unsigned new_x=mx, new_y=my;
	    if (emulated_mouse_up)
	    {
		    new_y-=emulated_mouse_speed;
		    if (new_y<0)
			    new_y=0;
	    }
	    if (emulated_mouse_down)
	    {
		    new_y+=emulated_mouse_speed;
		    if (new_y>400)
			    new_y=400;
	    }
	    if (emulated_mouse_left)
	    {
		    new_x-=emulated_mouse_speed;
		    if (new_x<0)
			    new_x=0;
	    }
	    if (emulated_mouse_right)
	    {
		    new_x+=emulated_mouse_speed;
		    if (new_x>640)
			    new_x=640;
	    }
	    if (new_x!=mx || new_y!=my)
	    {
//printf("x=%i, y=%i, xrel=%i, yrel=%i\n",mx*mouse_mul, my*mouse_mul, (new_x-mx)*mouse_mul, (new_y-my)*mouse_mul);
		IkbdMouseMotion(mx*mouse_mul, my*mouse_mul, (new_x-mx)*mouse_mul, (new_y-my)*mouse_mul);
		mx=new_x; my=new_y;
	    }
    }
#endif
    if (vkbd_key)
    {
	    if (vkbd_keysave==-1234567)
	    {
	    	vkbd_keysave=keysymToAtari[vkbd_key];
 	        IkbdKeyPress(vkbd_keysave);
	    }
    }
    else
    if (vkbd_keysave!=-1234567)
    {
       	IkbdKeyRelease(vkbd_keysave);
	vkbd_keysave=-1234567;
    }
#endif
#if !defined(DREAMCAST) && !defined(DINGOO)
    if (resized>0)
    {
	    resized--;
	    if (resized==20)
	  	    TV_ResizeWindow(resized_w,resized_h);
	    if (!resized)
		    show_icon();
    }
#endif
#else
// AUTOEVENTS
    SDL_Event event;
    static int vez=0;
    static int joystate;
    switch (vez&127)
    {
	    case 5: IkbdKeyPress(keysymToAtari[SDLK_1]); break;
	    case 10: IkbdKeyRelease(keysymToAtari[SDLK_1]); break;
/*
	    case 10: joystate |= JOY_LEFT; IkbdJoystickChange(0,joystate); break;
	    case 20: joystate |= JOY_BUTTON; IkbdJoystickChange(0,joystate); break;
	    case 22: joystate &= ~JOY_BUTTON; IkbdJoystickChange(0,joystate); break;
	    case 24: joystate &= ~JOY_LEFT; IkbdJoystickChange(0,joystate); break;
//	    case 40: joystate |= JOY_RIGHT; IkbdJoystickChange(0,joystate); break;
//	    case 60: joystate &= ~JOY_RIGHT; IkbdJoystickChange(0,joystate); break;
*/
	    case 20: joystate |= JOY_BUTTON; IkbdJoystickChange(0,joystate); break;
	    case 30: joystate &= ~JOY_BUTTON; IkbdJoystickChange(0,joystate); break;
    }
    vez++;
#ifdef AUTOEVENTS_MAX
    if (vez>=AUTOEVENTS_MAX)
    {
#ifdef DEBUG_FAME_FILE
	extern FILE *fame_debug_file;
	if (fame_debug_file)
		fclose(fame_debug_file);
	fame_debug_file=NULL;
#endif
	    SDL_Delay(800);
	    exit(0);
    }
#endif
    while( SDL_PollEvent(&event) )
    {
	    if (event.type==SDL_QUIT)
            	exit(0);
    }
#endif
}

