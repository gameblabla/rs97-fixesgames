
#ifndef NO_MENU

#ifdef DREAMCAST
#include <kos.h>
#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#ifndef DREAMCAST
#include <unistd.h>
#endif


#include <SDL_mixer.h>
#include <SDL_image.h>

#include "menu.h"
#include "sound/sound.h"
#include "sfont.h"
//#include "msg.h"
#include "fade.h"
#include "background.h"

#define TRANS_COLOR 0xFFFFFFFF
#define TRANS_COLOR2 0xC616C616
#define MUSIC_VOLUME 80

#ifdef DREAMCAST
#define VIDEO_FLAGS_INIT SDL_HWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF
#else
#ifdef WINDOWS
#define VIDEO_FLAGS_INIT SDL_SWSURFACE
#else
#define VIDEO_FLAGS_INIT SDL_HWSURFACE | SDL_HWPALETTE
#endif
#endif

#ifdef USE_DOUBLE_BUFFER
#if defined(USE_FULLSCREEN) && !defined(DEBUG_FAME)
#define VIDEO_FLAGS VIDEO_FLAGS_INIT | SDL_DOUBLEBUF | SDL_FULLSCREEN
#else
#define VIDEO_FLAGS VIDEO_FLAGS_INIT | SDL_DOUBLEBUF
#endif
#else
#if defined(USE_FULLSCREEN) && !defined(DEBUG_FAME)
#define VIDEO_FLAGS VIDEO_FLAGS_INIT | SDL_FULLSCREEN
#else
#define VIDEO_FLAGS VIDEO_FLAGS_INIT
#endif
#endif

SDL_Surface *text_screen=NULL, *text_image, *text_image2, *text_mac_upper_left, *text_mac_upper_right, *text_mac_upper, *text_mac_left, *text_mac_right, *text_mac_bottom_right, *text_mac_bottom_left, *text_mac_bottom;

extern SDL_Surface *ScreenSurface;
extern void quick_flip(SDL_Surface *src);

static SFont_FontInfo font_inv;
static Uint32 menu_inv_color=0, menu_win0_color=0, menu_win1_color=0;
static Uint32 menu_barra0_color=0, menu_barra1_color=0;
static Uint32 menu_win0_color_base=0, menu_win1_color_base=0;

#include "db.h"
int use_gamesdb=0;

void write_num(int x, int y, int v);
//int menu_msg_pos=330;
//int menu_moving=1;
//Uint32 menu_msg_time=0x12345678;

#ifdef DREAMCAST
extern int __sdl_dc_emulate_keyboard;
#endif

static void obten_colores(void)
{
	FILE *f=fopen(DATA_PREFIX "colors.txt", "rt");
	if (f)
	{
		Uint32 r,g,b;
		fscanf(f,"menu_inv_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_inv_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_win0_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_win0_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_win1_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_win1_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_barra0_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_barra0_color=SDL_MapRGB(text_screen->format,r,g,b);
		fscanf(f,"menu_barra1_color=0x%X,0x%X,0x%X\n",&r,&g,&b);
		menu_barra1_color=SDL_MapRGB(text_screen->format,r,g,b);
		fclose(f);
	}
	else
	{
		menu_inv_color=SDL_MapRGB(text_screen->format, 0x20, 0x20, 0x40);
		menu_win0_color=SDL_MapRGB(text_screen->format, 0x10, 0x08, 0x08);
		menu_win1_color=SDL_MapRGB(text_screen->format, 0x20, 0x10, 0x10);
		menu_barra0_color=SDL_MapRGB(text_screen->format, 0x30, 0x20, 0x20);
		menu_barra1_color=SDL_MapRGB(text_screen->format, 0x50, 0x40, 0x40);
	}
	menu_win0_color_base=menu_win0_color;
	menu_win1_color_base=menu_win1_color;
}

void menu_raise(void)
{
	int i;
	for(i=80;i>=0;i-=16)
	{
		Mix_VolumeMusic(MUSIC_VOLUME-(i<<1));
		text_draw_background();
		fade16(text_screen,i);
		text_flip();
		SDL_Delay(10);
	}
}

void menu_unraise(void)
{
	int i;
	for(i=0;i<=80;i+=16)
	{
		Mix_VolumeMusic(MUSIC_VOLUME-(i<<1));
		text_draw_background();
		fade16(text_screen,i);
		text_flip();
		SDL_Delay(10);
	}
}

static void text_draw_menu_msg()
{
/*
	write_text_pos(menu_msg_pos,0,menu_msg);
	if (menu_msg_pos<MAX_SCROLL_MSG)
		menu_msg_pos=330;
	else
		menu_msg_pos--;
*/
}


static void update_window_color(void)
{
	static int cambio=0;
	static int spin=0;

	Uint8 r,g,b;
	int cambio2=cambio>>3;
	SDL_GetRGB(menu_win0_color_base,text_screen->format,&r,&g,&b);
	if (((int)r)-cambio2>0) r-=cambio2;
	else r=0;
	if (((int)g)-cambio2>0) g-=cambio2;
	else g=0;
	if (((int)b)-cambio2>0) b-=cambio2;
	else b=0;
	menu_win0_color=SDL_MapRGB(text_screen->format,r,g,b);
	SDL_GetRGB(menu_win1_color_base,text_screen->format,&r,&g,&b);
	if (((int)r)-cambio>0) r-=cambio;
	else r=0;
	if (((int)g)-cambio>0) g-=cambio;
	else g=0;
	if (((int)b)-cambio>0) b-=cambio;
	else b=0;
	menu_win1_color=SDL_MapRGB(text_screen->format,r,g,b);
	if (spin)
	{
		if (cambio<=0) spin=0;
		else cambio-=2;

	}
	else
	{
		if (cambio>=24) spin=1;
		else cambio+=2;
	}
}

void text_draw_background()
{
/*
	static int pos_x=12345678;
	static int pos_y=12345678;
	SDL_Rect r;
	int i,j;
	int w=text_screen->w+text_background->w-1;
	int h=text_screen->h+text_background->h-1;

	if (menu_moving)
	{
		if (pos_x>=0) pos_x=-text_screen->w;
		else pos_x++;
		if (pos_y>=0) pos_y=-text_screen->h;
		else pos_y++;
	}

	for(i=pos_x;i<w;i+=text_background->w)
		for(j=pos_y;j<h;j+=text_background->h)
		{
			r.x=i;
			r.y=j;
			r.w=text_background->w;
			r.h=text_background->h;
			SDL_BlitSurface(text_background,NULL,text_screen,&r);
		}
	if (menu_moving)
	{
		text_draw_menu_msg();
		update_window_color();
	}
*/
	draw_background(text_screen);
}

void text_flip(void)
{
#ifndef DREAMCAST
	SDL_Delay(10);
#endif
	SDL_BlitSurface(text_screen,NULL,screen,NULL);
	//TV_Flip(screen);
	quick_flip(screen);
}

void init_text(int splash)
{
	SDL_Event ev;
	SDL_Surface *tmp;
	int toexit=0;
	SDL_Surface *sur;
	SDL_Rect r;
	int i,j;

	if (screen==NULL)
	{
		SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK);
		//screen=TV_Init(320,240,16,VIDEO_FLAGS);
		ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
		screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);
#ifdef DREAMCAST
		puts("Change SDL video driver");fflush(stdout);
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
#ifdef USE_DOUBLE_BUFFER
		SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
#else
		SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
#endif
		SDL_InitSubSystem(SDL_INIT_VIDEO);
 	  screen = SDL_SetVideoMode(320, 240, 16, VIDEO_FLAGS);
#endif
    		SDL_ShowCursor(SDL_DISABLE);
 	   	SDL_JoystickEventState(SDL_ENABLE);
    		SDL_JoystickOpen(0);
    		SDL_JoystickOpen(1);
		show_icon();
	}
#ifdef DREAMCAST
        __sdl_dc_emulate_keyboard=1;
#endif
	if (!splash)
	{
		menu_raise();
		return;
	}
#ifdef DREAMCAST
	text_screen=SDL_DisplayFormat(screen); //SDL_CreateRGBSurface(screen->flags,screen->w,screen->h,screen->format->BitsPerPixel,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
#else
	text_screen=SDL_CreateRGBSurfaceFrom(calloc(screen->h,screen->pitch),screen->w,screen->h,screen->format->BitsPerPixel,screen->pitch,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
#endif



	tmp=IMG_Load(MENU_FILE_TEXT2);
	if (text_screen==NULL || tmp==NULL)
		exit(-1);
	text_image2=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	if (text_image2==NULL)
		exit(-2);
	SDL_SetColorKey(text_image2,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_screen -> format, 0, 0, 0));
	font_inv.Surface=text_image2;
	SFont_InitFontInfo(&font_inv);

	tmp=IMG_Load(MENU_FILE_TEXT);
	if (text_screen==NULL || tmp==NULL)
		exit(-1);
	text_image=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	if (text_image==NULL)
		exit(-2);
	SDL_SetColorKey(text_image,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_screen -> format, 0, 0, 0));
	SFont_InitFont(text_image);

	tmp=IMG_Load(MENU_FILE_MAC_UPPER_LEFT);
	if (tmp==NULL)
		exit(-3);
	text_mac_upper_left=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	if (text_mac_upper_left==NULL)
		exit(-3);
	SDL_SetColorKey(text_mac_upper_left,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_screen -> format, 255, 255, 255));

	tmp=IMG_Load(MENU_FILE_MAC_UPPER_RIGHT);
	if (tmp==NULL)
		exit(-4);
	text_mac_upper_right=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	if (text_mac_upper_right==NULL)
		exit(-5);
	SDL_SetColorKey(text_mac_upper_right,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_screen -> format, 255, 255, 255));

	tmp=IMG_Load(MENU_FILE_MAC_UPPER);
	if (tmp==NULL)
		exit(-3);
	text_mac_upper=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	if (text_mac_upper==NULL)
		exit(-3);
	SDL_SetColorKey(text_mac_upper,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_screen -> format, 255, 255, 255));

	tmp=IMG_Load(MENU_FILE_MAC_LEFT);
	if (tmp==NULL)
		exit(-3);
	text_mac_left=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	if (text_mac_left==NULL)
		exit(-3);
	SDL_SetColorKey(text_mac_left,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_screen -> format, 255, 255, 255));

	tmp=IMG_Load(MENU_FILE_MAC_RIGHT);
	if (tmp==NULL)
		exit(-3);
	text_mac_right=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	if (text_mac_right==NULL)
		exit(-3);
	SDL_SetColorKey(text_mac_right,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_screen -> format, 255, 255, 255));

	tmp=IMG_Load(MENU_FILE_MAC_BOTTOM_RIGHT);
	if (tmp==NULL)
		exit(-3);
	text_mac_bottom_right=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	if (text_mac_bottom_right==NULL)
		exit(-3);
	SDL_SetColorKey(text_mac_bottom_right,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_screen -> format, 255, 255, 255));

	tmp=IMG_Load(MENU_FILE_MAC_BOTTOM_LEFT);
	if (tmp==NULL)
		exit(-3);
	text_mac_bottom_left=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	if (text_mac_bottom_left==NULL)
		exit(-3);
	SDL_SetColorKey(text_mac_bottom_left,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_screen -> format, 255, 255, 255));

	tmp=IMG_Load(MENU_FILE_MAC_BOTTOM);
	if (tmp==NULL)
		exit(-3);
	text_mac_bottom=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	if (text_mac_bottom==NULL)
		exit(-3);
	SDL_SetColorKey(text_mac_bottom,(SDL_SRCCOLORKEY | SDL_RLEACCEL),SDL_MapRGB(text_screen -> format, 255, 255, 255));

	init_background();

	obten_colores();
#if !defined(DEBUG_ZX4ALL) && !defined(PROFILER_ZX4ALL) && !defined(AUTO_RUN) && !defined(AUTO_FRAMERATE)
	audio_init_music();
	tmp=IMG_Load(MENU_FILE_SPLASH);
	if (tmp==NULL)
		exit(-6);
	sur = SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	r.x=(text_screen->w - sur->w)/2;
	r.y=(text_screen->h - sur->h)/2;
	r.h=sur->w;
	r.w=sur->h;
	SDL_FillRect(text_screen,NULL,0); //0xFFFFFFFF);
	while(SDL_PollEvent(&ev)) SDL_Delay(50);
#if !defined(AUTOLOAD) && !defined(DEBUG_FAME)
	for (i=128;(i>-8)&&(!toexit);i-=8)
	{
#ifdef DREAMCAST
		vid_waitvbl();
#else
		SDL_Delay(50);
#endif
		SDL_FillRect(text_screen,NULL,0); //0xFFFFFFFF);
		SDL_BlitSurface(sur,NULL,text_screen,&r);
		fade16(text_screen,i);
		text_flip();
		while(SDL_PollEvent(&ev)) toexit=1;
	}
	for(i=0;(i<23)&&(!toexit);i++)
	{
		while(SDL_PollEvent(&ev)) toexit=1;
		SDL_Delay(100);
	}
	for(i=0;(i<128)&&(!toexit);i+=16)
	{
#ifdef DREAMCAST
		vid_waitvbl();
#else
		SDL_Delay(50);
#endif
		SDL_FillRect(text_screen,NULL,0); //0xFFFFFFFF);
		SDL_BlitSurface(sur,NULL,text_screen,&r);
		fade16(text_screen,i);
		text_flip();
		while(SDL_PollEvent(&ev)) toexit=1;
	}
	for(i=128;(i>-8)&&(!toexit);i-=8)
	{
#ifdef DREAMCAST
		vid_waitvbl();
#else
		SDL_Delay(50);
#endif
		text_draw_background();
		fade16(text_screen,i);
		text_flip();
		while(SDL_PollEvent(&ev)) toexit=1;
	}
#endif
	SDL_FreeSurface(sur);
#else
#ifndef DREAMCAST
	chdir(DISK_PATH_PREFIX);
#else
	fs_chdir("/");
#endif
#endif
	db_crc_init();
	use_gamesdb = db_retrieve();
//	menu_msg_time=SDL_GetTicks();
}


void quit_text(void)
{
/*
	SDL_FreeSurface(text_image);
	SDL_FreeSurface(text_background);
	SDL_FreeSurface(text_window_background);
//	SDL_FreeSurface(text_screen);
	quit_background();
*/
}

/*
void write_text_pos(int x, int y, char * str)
{
  int i, c;
  SDL_Rect src, dest;
  
  for (i = 0; i < strlen(str); i++)
    {
      c = -1;
      
      if (str[i] >= '0' && str[i] <= '9')
	c = str[i] - '0';
      else if (str[i] >= 'A' && str[i] <= 'Z')
	c = str[i] - 'A' + 10;
      else if (str[i] >= 'a' && str[i] <= 'z')
	c = str[i] - 'a' + 36;
      else if (str[i] == '#')
	c = 62;
      else if (str[i] == '=')
	c = 63;
      else if (str[i] == '.')
	c = 64;
      else if (str[i] == '_')
	c = -2;
      else if (str[i] == '-')
	c = -3;
      else if (str[i] == '(')
	c = 65;
      else if (str[i] == ')')
	c = 66;
      
      if (c >= 0)
	{
	  src.x = c * 8;
	  src.y = 0;
	  src.w = 8;
	  src.h = 8;
	  
	  dest.x = x + (i * 8);
	  dest.y = y;
	  dest.w = 8;
	  dest.h = 8;
	  
	  SDL_BlitSurface(text_image, &src,
			  text_screen, &dest);
	}
      else if (c == -2 || c == -3)
	{
	  dest.x = x + (i * 8);
	  
	  if (c == -2)
	    dest.y = y  + 7;
	  else if (c == -3)
	    dest.y = y  + 3;
	  
	  dest.w = 8;
	  dest.h = 1;
	  
	  SDL_FillRect(text_screen, &dest, menu_barra0_color);
	}
    }
}
*/

void _write_text_shadow(SDL_Surface *sf, int x, int y, char * str)
{
	SFont_PutStringInfo(sf, &font_inv, x*8, y*8, str);
	SFont_PutString(sf, (x*8)-2, (y*8)-2, str);
}

void write_text_shadow(int x, int y, char * str)
{
	_write_text_shadow(text_screen,x,y,str);
}

void _write_text(SDL_Surface *sf, int x, int y, char * str)
{
  SFont_PutString(sf, x*8, y*8, str);
}

void write_text(int x, int y, char * str)
{
	_write_text(text_screen,x,y,str);
}

/* Write text, inverted: */
void _write_text_inv(SDL_Surface *sf, int x, int y, char * str)
{
	SFont_PutStringInfo(sf, &font_inv, x*8, y*8, str);
}

void write_text_inv(int x, int y, char * str)
{
	_write_text_inv(text_screen,x,y,str);
}

void _write_text_inv_n(SDL_Surface *sf, int x, int y, int n, char * str)
{
	SDL_Rect dest;
	dest.x = (x * 8) -2 ;
	dest.y = (y * 8) /*10*/ - 2;
	dest.w = (n*8)+4;
	dest.h = 16;
	SDL_FillRect(sf, &dest, menu_inv_color);
	SFont_PutString(sf, x*8, y*8, str);
}

void write_text_inv_n(int x, int y, int n, char * str)
{
	_write_text_inv_n(text_screen,x,y,n,str);
}


void write_text_sel(int x, int y, int w, char * str)
{
	int i,j,h=14;
	int x8=(x*8)-4;
	int y8=y*8;
#ifndef DREAMCAST
	if (SDL_MUSTLOCK(text_screen))
		SDL_LockSurface(text_screen);
#endif
	if ((y8+h)>text_screen->h)
		h=text_screen->h-y8;
	if (y8<text_screen->h)
	{
		register Uint16 *buf=(Uint16 *)
			(((unsigned)text_screen->pixels)+(x8*2)+(y8*text_screen->pitch));
		register unsigned dx=(text_screen->pitch/2)-w;
		for(j=0;j<h;j++,buf+=dx)
		{
			for(i=0;i<w;i+=2,buf++)
				*buf++=TRANS_COLOR2;
			if (j&1)
				buf++;
			else
				buf--;
		}
	}
#ifndef DREAMCAST
	if (SDL_MUSTLOCK(text_screen))
		SDL_UnlockSurface(text_screen);
#endif
	write_text_inv(x,y,str);
}


/* Write text, horizontally centered... */

void write_centered_text(int y, char * str)
{
  write_text(20 - (strlen(str) / 2), y/2, str);
}


/* Write numbers on the option screen: */

void write_num(int x, int y, int v)
{
  char str[24];
  
  sprintf(str, "%d", v);
  write_text(x, y, str);
}

void write_num_inv(int x, int y, int v)
{
  char str[24];
  
  sprintf(str, "%d", v);
  write_text_inv(x, y, str);
}

void write_num_sel(int x, int y, int w, int v)
{
  char str[24];
  
  sprintf(str, "%d", v);
  write_text_sel(x, y, w, str);
}

void text_draw_barra(int x, int y, int w, int h, int per, int max)
{
	SDL_Rect dest;
if (h>5) h-=4;
	dest.x=x-1;
	dest.y=y-1;
	dest.w=w+2;
	dest.h=h+2;
	SDL_FillRect(text_screen, &dest, 0xdddd); //menu_barra0_color);
	dest.x=x;
	dest.y=y;
	dest.h=h;
	dest.w=(w*per)/max;
	SDL_FillRect(text_screen, &dest, 0x8888); //menu_barra1_color);
}


void text_draw_window(int x, int y, int w, int h, char *title)
{
	int i,j;
	SDL_Rect dest;
	
	dest.x=x-6;
	dest.y=y-22;
	dest.w=80;
	dest.h=40;
	SDL_BlitSurface(text_mac_upper_left,NULL,text_screen,&dest);

	for(i=0;i<(w-80);i+=16)
	{
		dest.x=x-8+80+i;
		dest.y=y-22;
		dest.w=16;
		dest.h=40;
		SDL_BlitSurface(text_mac_upper,NULL,text_screen,&dest);
	}

	dest.x=x+w-80+6;
	dest.y=y-22;
	dest.w=80;
	dest.h=40;
	SDL_BlitSurface(text_mac_upper_right,NULL,text_screen,&dest);


	for(i=0;i<(h-16);i+=16)
	{
		dest.x=x-6;
		dest.y=y+i;
		dest.w=80;
		dest.h=16;
		SDL_BlitSurface(text_mac_left,NULL,text_screen,&dest);

		dest.x=x+w-80+6;
		dest.y=y+i;
		dest.w=80;
		dest.h=16;
		SDL_BlitSurface(text_mac_right,NULL,text_screen,&dest);
	}

	dest.x=x-6;
	dest.y=y+h-24;
	dest.w=80;
	dest.h=40;
	SDL_BlitSurface(text_mac_bottom_left,NULL,text_screen,&dest);

	for(i=0;i<(w-80);i+=16)
	{
		dest.x=x-8+80+i;
		dest.y=y+h-24;
		dest.w=16;
		dest.h=40;
		SDL_BlitSurface(text_mac_bottom,NULL,text_screen,&dest);
	}

	dest.x=x+w-80+6;
	dest.y=y+h-24;
	dest.w=80;
	dest.h=40;
	SDL_BlitSurface(text_mac_bottom_right,NULL,text_screen,&dest);


	write_text_inv((x+72)/8, (y-12)/8 , title);

	y+=4; h-=16;
	x+=2; w-=4;
#ifndef DREAMCAST
	if (SDL_MUSTLOCK(text_screen))
		SDL_LockSurface(text_screen);
#endif
	if ((y+h)>text_screen->h)
		h=text_screen->h-y;
	if (y<text_screen->h)
	{
		register Uint16 *buf=(Uint16 *)text_screen->pixels;
		buf=(Uint16 *)&buf[x+(y*(text_screen->pitch/2))];
		register unsigned dx=(text_screen->pitch/2)-w;
		for(j=0;j<h;j++,buf+=dx)
		{
			for(i=0;i<w;i+=2,buf++)
				*buf++=TRANS_COLOR;
			if (j&1)
				buf++;
			else
				buf--;
		}
	}
#ifndef DREAMCAST
	if (SDL_MUSTLOCK(text_screen))
		SDL_UnlockSurface(text_screen);
#endif

/*
	int i,j;
	int r8x = x / 8;
	int r8y = y / 8;
	int rx = r8x * 8;
	int ry = r8y * 8;
	int r32w =  w / 32;
	int r24h =  h / 24;
	int rw = r32w * 32;
	int rh = r24h * 24;
	int r8w = rw / 8;


	SDL_Rect dest;

	dest.x = rx - 2;
	dest.y = ry - 10;
	dest.w = rw + 4;
	dest.h = 10; //rh + 18;
	SDL_FillRect(text_screen, &dest, menu_win1_color);


	dest.x = rx - 2;
	dest.y = ry; 
	dest.w = 2; //rw + 4;
	dest.h = rh;// + 14; //16;
	SDL_FillRect(text_screen, &dest, menu_win0_color);
	dest.x = rx - 2;
	dest.y = ry+rh; 
	dest.w = rw+4; //rw + 4;
	dest.h = 2;// + 14; //16;
	SDL_FillRect(text_screen, &dest, menu_win0_color);
	dest.x = rx+rw;
	dest.y = ry; 
	dest.w = 2; //rw + 4;
	dest.h = rh;// + 14; //16;
	SDL_FillRect(text_screen, &dest, menu_win0_color);

	write_text(r8x, r8y - 1, "OOO");
	write_text(r8x + ((r8w-strlen(title)) / 2), r8y - 1, title);

	r32w = w / text_window_background->w;
	r24h = h / text_window_background->h;
	dest.x=rx;
	dest.y=ry;
	dest.w=rw;//-4;
	dest.h=rh;//-8;
	SDL_SetClipRect(text_screen, &dest);
	for(i=0;i<4;i++)
		for(j=0;j<2;j++)
		{
			dest.x=rx+i*text_window_background->w;
			dest.y=ry+j*text_window_background->h;
			dest.w=text_window_background->w; //32;
			dest.h=text_window_background->h; //24;
			SDL_BlitSurface(text_window_background,NULL,text_screen,&dest);
		}
	SDL_SetClipRect(text_screen, NULL);
*/
}
#endif
