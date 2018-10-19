#include "dcastaway.h"
#include "config.h"
#include "st.h"
#include "vkbd/vkbd.h"
#include "tvfilter/tvfilter.h"
#include "menu/menu.h"

#ifdef USE_BIG_ENDIAN
#define WRL(A, B) ((B)|(((unsigned)(A)) << 16))
#else
#define WRL(A, B) ((A)|(((unsigned)(B)) << 16))
#endif

SDL_Surface *ScreenSurface=NULL;
SDL_Surface *screen=NULL;
static unsigned int screen_buffer, screen_add=20;

unsigned screen_pitch, screen_width, screen_height;

static unsigned long vm2bm1[256];
static unsigned long vm2bm2[256];

static unsigned short pal16[16]={ 0, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };

unsigned short render_pal16_copy0=0;

static int show_message=0;
static char _show_message_str[40]=
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
static char *show_message_str=(char *)&_show_message_str[0];

void quick_flip(SDL_Surface *src)
{
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
	int x, y;
	uint32_t *s = (uint32_t*)src->pixels;
	uint32_t *d = (uint32_t*)ScreenSurface->pixels;
	for(uint8_t y = 0; y < 240; y++, s += 160, d += 320) 
		memmove(d, s, 1280); // double-line fix by pingflood, 2018
	/*for(y=0; y<240; y++){
		for(x=0; x<160; x++){
			*d++ = *s++;
		}
		d+= 160;
	}*/
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
	SDL_Flip(ScreenSurface);
}

void set_message(char *msg, int t)
{
	show_message=t;
	strncpy(show_message_str, msg, 36);
}

void render_blank_screen(void)
{
  SDL_FillRect(screen,NULL,render_pal16_copy0);
	//TV_Flip(screen);
	quick_flip(screen);
}

void render_force_background(void)
{
	unsigned _p0=pal16[0];
#ifdef DREAMCAST
	unsigned r=(unsigned)( 8.2259 * (double)((_p0>>11)&0x1f));
	unsigned g=(unsigned)( 4.0477 * (double)((_p0>>5)&0x3f));
	unsigned b=(unsigned)( 8.2259 * (double)(_p0&0x1f));
	vid_border_color(r,g,b);
#endif
	render_pal16_copy0=_p0;
}

void render_up_screen(void)
{
#if !defined(USE_DOUBLE_BUFFER) || !defined(DREAMCAST)
	unsigned he=40*(screen_height/240);
#endif
#ifndef DREAMCAST
	if (he==40)
	{
		SDL_Rect r;
		r.x=0;
		r.y=200;
		r.w=screen_width;
		r.h=40;
		SDL_FillRect(screen,&r,render_pal16_copy0);
	}
	else
	{
		int i;
		for(i=1;i<he;i+=2)
		{
			SDL_Rect r;
			r.x=0;
			r.y=screen_height-i;
			r.w=screen_width;
			r.h=1;
			SDL_FillRect(screen,&r,render_pal16_copy0);
		}
	}
#else
	{
		register int i;
		register short col=render_pal16_copy0;
		register int sw4=screen_width>>2;
		for(i=200;i<240;i++)
		{
			register short *p=(short *)((unsigned)(screen->pixels)+i*screen->pitch);
			register int j;
			for(j=0;j<sw4;j++)
			{
				*p++=col;
				*p++=col;
				*p++=col;
				*p++=col;
			}
		}
	}
#endif
#if !defined(USE_DOUBLE_BUFFER) && !defined(DINGOO)
	SDL_UpdateRect(screen, 0 , screen_height-he, screen_width, he);
#else
//	TV_Flip(screen);
#endif
	screen_add=0;
}

void render_down_screen(void)
{
#if !defined(USE_DOUBLE_BUFFER) || !defined(DREAMCAST)
	unsigned he=20*(screen_height/240);
#endif
#ifndef DREAMCAST
#ifdef NO_USE_TV_FILTER
	if (he==20)
#endif
	{
		SDL_Rect r;
		r.x=0;
		r.y=0;
		r.w=screen_width;
		r.h=he;
		SDL_FillRect(screen,&r,render_pal16_copy0);
	}
#ifdef NO_USE_TV_FILTER
	else
	{
		int i;
		for(i=0;i<he;i+=2)
		{
			SDL_Rect r;
			r.x=0;
			r.y=i;
			r.w=screen_width;
			r.h=1;
			SDL_FillRect(screen,&r,render_pal16_copy0);
		}
	}
#endif
#else
	{
		register int i;
		register short col=render_pal16_copy0;
		register int sw4=screen_width>>2;
		for(i=0;i<20;i++)
		{
			register short *p=(short *)((unsigned)(screen->pixels)+i*screen->pitch);
			register int j;
			for(j=0;j<sw4;j++)
			{
				*p++=col;
				*p++=col;
				*p++=col;
				*p++=col;
			}
		}
	}
#endif
#if !defined(USE_DOUBLE_BUFFER) && !defined(DINGOO)
	SDL_UpdateRect(screen, 0 , 0, screen_width, he);
#endif
#ifndef DREAMCAST
#ifdef NO_USE_TV_FILTER
	if (he==20)
#endif
	{
		SDL_Rect r;
		r.x=0;
		r.y=screen_height-he;
		r.w=screen_width;
		r.h=he;
		SDL_FillRect(screen,&r,render_pal16_copy0);
	}
#ifdef NO_USE_TV_FILTER
	else
	{
		int i;
		for(i=2;i<=he;i+=2)
		{
			SDL_Rect r;
			r.x=0;
			r.y=screen_height-i;
			r.w=screen_width;
			r.h=1;
			SDL_FillRect(screen,&r,render_pal16_copy0);
		}
	}
#endif
#else
	{
		register int i;
		register short col=render_pal16_copy0;
		register int sw4=screen_width>>2;
		for(i=220;i<240;i++)
		{
			register short *p=(short *)((unsigned)(screen->pixels)+i*screen->pitch);
			register int j;
			for(j=0;j<sw4;j++)
			{
				*p++=col;
				*p++=col;
				*p++=col;
				*p++=col;
			}
		}
	}
#endif
#if !defined(USE_DOUBLE_BUFFER) && !defined(DINGOO)
	SDL_UpdateRect(screen, 0 , screen_height-he, screen_width, he);
#else
//	TV_Flip(screen);
#endif
	screen_add=20;
}

static int actual_is_med=0;

#if !defined(DREAMCAST) && !defined(DINGOO)
#if defined(USE_FULLSCREEN) && !defined(DEBUG_FAME)
static int full=SDL_FULLSCREEN;
#else
static int full=0;
#endif
void video_fullscreen(void)
{
#ifdef NO_USE_TV_FILTER
#ifndef ALWAYS_LOW
	if (!actual_is_med)
#endif
	{
		Uint32 flags=SDL_HWSURFACE | SDL_HWPALETTE;
		int i;
		void *buff=malloc(320*240*2);
		if (SDL_MUSTLOCK(screen))
			SDL_LockSurface(screen);
		for(i=0;i<240;i++)
			memcpy( (void *)((unsigned)buff+(320*2*i)),
				(void *)((unsigned)screen->pixels+(screen->pitch*i)),
				320*2);
		if (SDL_MUSTLOCK(screen))
			SDL_UnlockSurface(screen);
#ifdef USE_DOUBLE_BUFFER
		flags|=SDL_DOUBLEBUF;
#endif
		if (!full)
			full=SDL_FULLSCREEN;
		else
			full=0;
		//screen=SDL_SetVideoMode(320, 240, 16, flags|full);
		ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
		screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);
		show_icon();
		render_no_border();
		render_vkbd_background();
		Draw_border(0);
		//SDL_Flip(screen);
		quick_flip(screen);
		if (SDL_MUSTLOCK(screen))
			SDL_LockSurface(screen);
		screen_buffer=(unsigned int)screen->pixels;
		screen_pitch=screen->pitch;
		screen_width=320;
		screen_height=240;
		for(i=0;i<240;i++)
			memcpy( (void *)((unsigned)screen->pixels+(screen->pitch*i)),
			       	(void *)((unsigned)buff+(320*2*i)),
				320*2);
		if (SDL_MUSTLOCK(screen))
			SDL_UnlockSurface(screen);
		free(buff);
		render_no_border();
		render_vkbd_background();
		Draw_border(0);
	}
#ifndef ALWAYS_LOW
	else
	{
		Uint32 flags=SDL_HWSURFACE | SDL_HWPALETTE;
		void *buff=calloc(640,480*2);
		if (SDL_MUSTLOCK(screen))
			SDL_LockSurface(screen);
		memcpy(buff,screen->pixels,320*240);
		SDL_UnlockSurface(screen);
#ifdef USE_DOUBLE_BUFFER
		flags|=SDL_DOUBLEBUF;
#endif
		if (!full)
			full=SDL_FULLSCREEN;
		else
			full=0;
		screen=SDL_SetVideoMode(640, 480, 16, flags|full);
		show_icon();
		render_no_border();
		render_vkbd_background();
		Draw_border(0);
		if (SDL_MUSTLOCK(screen))
			SDL_LockSurface(screen);
		screen_buffer=(unsigned int)screen->pixels;
		screen_pitch=screen->pitch;
		screen_width=320;
		screen_height=240;
		memcpy(screen->pixels,buff,640*480);
		SDL_UnlockSurface(screen);
		free(buff);
		render_no_border();
		render_vkbd_background();
		Draw_border(0);
	}
#endif
#else
	if (!full)
		full=SDL_FULLSCREEN;
	else
		full=0;
	TV_SetFullScreen(screen,full==SDL_FULLSCREEN?SDL_TRUE:SDL_FALSE);
	show_icon();
#endif
	vid_flag=1;
}
#endif

void video_change_to_low(void)
{
#ifndef ALWAYS_LOW
	SDL_Surface *old=screen;
#ifdef WINDOWS
	Uint32 flags=SDL_SWSURFACE;
#else
	Uint32 flags=SDL_HWSURFACE | SDL_HWPALETTE;
#endif
#ifdef DREAMCAST
	flags|=SDL_FULLSCREEN;
#else
	if (screen && !full)
		full=screen->flags&SDL_FULLSCREEN;
	flags|=full;
#endif
#ifdef USE_DOUBLE_BUFFER
	flags|=SDL_DOUBLEBUF;
#endif

	if (!screen || actual_is_med)
	{
#ifdef NO_USE_TV_FILTER
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
#ifdef DREAMCAST
#ifdef USE_DOUBLE_BUFFER
		SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
#else
		SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
#endif
#endif
		SDL_InitSubSystem(SDL_INIT_VIDEO);
 	  //screen = SDL_SetVideoMode(320, 240, 16, flags);
		ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
		screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);
#else
		//screen = TV_Init(320, 240, 16, flags);
		ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
		screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);
#endif
	}
	actual_is_med=0;
/*
	if (old)
		SDL_FreeSurface(old);
*/
#else
// ALWAYS_LOW:
	if (actual_is_med)
		free((void *)screen_buffer);
	actual_is_med=0;
#endif
	SDL_FillRect(screen,NULL,0);
	show_icon();

	screen_buffer=(unsigned int)screen->pixels;
	screen_pitch=screen->pitch;
	screen_width=320;
	screen_height=240;
	render_no_border();
	render_vkbd_background();
	Draw_border(0);
	vid_flag=1;
}

void video_change_to_med(void)
{
#ifndef ALWAYS_LOW
	SDL_Surface *old=screen;
#ifdef WINDOWS
	Uint32 flags=SDL_SWSURFACE;
#else
	Uint32 flags=SDL_HWSURFACE | SDL_HWPALETTE;
#endif
//	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK);
#ifdef DREAMCAST
	flags|=SDL_FULLSCREEN;
#else
	if (screen && !full)
		full=screen->flags&SDL_FULLSCREEN;
	flags|=full;
#endif
#ifdef USE_DOUBLE_BUFFER
	flags|=SDL_DOUBLEBUF;
#endif

	if (!screen || !actual_is_med)
	{
#ifdef NO_USE_TV_FILTER
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
#ifdef DREAMCAST
		SDL_DC_SetVideoDriver(SDL_DC_TEXTURED_VIDEO);
		SDL_InitSubSystem(SDL_INIT_VIDEO);
 		screen = SDL_SetVideoMode(1024, 256, 16, SDL_HWSURFACE);
		SDL_DC_SetWindow(640,240);
#else
		SDL_InitSubSystem(SDL_INIT_VIDEO);
 		screen = SDL_SetVideoMode(640, 480, 16, flags);
#endif
#else
 		screen = TV_Init(640, 480, 16, flags);
#endif
	}
#else
//ALWAYS_LOW:
	if (!actual_is_med)
		screen_buffer=(unsigned int)calloc(640*480,2);
#endif
	actual_is_med=1;
	SDL_FillRect(screen,NULL,0);
	show_icon();
/*
	if (old)
		SDL_FreeSurface(old);
*/
#ifndef ALWAYS_LOW
	screen_buffer=(unsigned int)screen->pixels;
#endif
	screen_width=640;
#ifdef DREAMCAST
	screen_pitch=screen->pitch;
	screen_height=240;
#else
#ifndef ALWAYS_LOW
	screen_pitch=screen->pitch*2;
#else
	screen_pitch=640*2;
#endif
	screen_height=480;
#endif
	render_no_border();
	render_vkbd_background();
	Draw_border(0);
	vid_flag=1;
}

void video_change_to_menu(void)
{
#ifndef ALWAYS_LOW
#ifdef WINDOWS
	Uint32 flags=SDL_SWSURFACE;
#else
	Uint32 flags=SDL_HWSURFACE | SDL_HWPALETTE;
#endif
#ifdef DREAMCAST
	flags|=SDL_FULLSCREEN;
#else
	if (screen && !full)
		full=screen->flags&SDL_FULLSCREEN;
	flags|=full;
#endif
#ifdef USE_DOUBLE_BUFFER
	flags|=SDL_DOUBLEBUF;
#endif
	if (!screen || actual_is_med)
	{
#ifdef NO_USE_TV_FILTER
		SDL_Surface *old=screen;
		if (old)
			SDL_QuitSubSystem(SDL_INIT_VIDEO);
#ifdef DREAMCAST
#ifdef USE_DOUBLE_BUFFER
		SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
#else
		SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
#endif
#endif
		if (old)
			SDL_InitSubSystem(SDL_INIT_VIDEO);
		//screen = SDL_SetVideoMode(320, 240, 16, flags);
		ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
		screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);
#else
		//screen = TV_Init(320, 240, 16, flags);
		ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
		screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);
#endif
	}
	if (screen == NULL) {
		fprintf(stderr, "SDL_SetVideoMode(): %s\n", SDL_GetError());
		exit(1);
	}
#else
//ALWAYS_LOW:
	if (actual_is_med)
		free((void *)screen_buffer);
#endif
	actual_is_med=0;
	SDL_FillRect(screen,NULL,0);
	show_icon();
/*
	if (old)
		SDL_FreeSurface(old);
*/
	screen_buffer=(unsigned int)screen->pixels;
	screen_pitch=screen->pitch;
	vid_flag=1;
}

void render_init(void)
{
	int i;
	if (screen==NULL)
		video_change_to_menu();
	screen_add=20;
	for (i = 0; i < 256; i++) {
		vm2bm1[i] =
			((i & 0x10) ? 0x01000000L : 0)
			| ((i & 0x20) ? 0x00010000L : 0)
			| ((i & 0x40) ? 0x00000100L : 0)
			| ((i & 0x80) ? 0x00000001L : 0);
		vm2bm2[i] =
			((i & 0x01) ? 0x01000000L : 0)
			| ((i & 0x02) ? 0x00010000L : 0)
			| ((i & 0x04) ? 0x00000100L : 0)
			| ((i & 0x08) ? 0x00000001L : 0);
	}
	vid_flag=1;
}

void Draw_border(int e)
{
	if (screen_add)
	{
// #ifdef DREAMCAST
		unsigned _p0=pal16[0];
		if (render_pal16_copy0!=_p0)
// #endif
		{
			render_force_background();
			if (!e)
				render_down_screen();
		}
	}
}

void render_vkbd_background(void)
{
	if (!screen_add)
	{
		unsigned _p0=pal16[0];
		if (render_pal16_copy0!=_p0)
		{
			render_force_background();
			render_up_screen();
			vkbd_redraw();
		}
	}
}

void render_no_border(void)
{
	render_pal16_copy0=~pal16[0];
#ifdef DREAMCAST
	vid_border_color(0,0,0);
#endif
}


void Redraw ( int row, int vid_adr )
{
	if (vid_flag) {
		vid_flag = 0;
		register unsigned i;
		for (i = 0; i < 16; i++) {
#ifndef DREAMCAST
            		unsigned b = (unsigned)(36.428 * (double)((vid_col[i] & 0x7)));
            		unsigned g = (unsigned)(36.428 * (double)(((vid_col[i] >> 4) & 0x7)));
            		unsigned r = (unsigned)(36.428 * (double)(((vid_col[i] >> 8) & 0x7)));
	   		pal16[i]=(unsigned short)SDL_MapRGB(screen->format,r,g,b);
#else
            		register unsigned b = (unsigned)(4.4286 * (double)((vid_col[i] & 0x7)));
            		register unsigned g = 9 * (unsigned)(((vid_col[i] >> 4) & 0x7));
            		register unsigned r = (unsigned)(4.4286 * (double)(((vid_col[i] >> 8) & 0x7)));
			pal16[i]= (r<<11)|(g<<5)|(b);
#endif
		}
	}
	{
		register int real_row=(row+screen_add);
		if (real_row<0 || real_row>=240 || vid_adr>(MEMSIZE-160))
			return;
	}
#ifndef DREAMCAST
#ifndef DINGOO
	if (SDL_MUSTLOCK(screen))
        	if (SDL_LockSurface(screen)<0)
			return;
#endif
#ifndef USE_DOUBLE_BUFFER
	register unsigned long *line_o =
		(unsigned long *) (screen_buffer +  screen_pitch * (row+screen_add));
	if ((screen_buffer!=(unsigned int)screen->pixels)||(screen_pitch!=screen->pitch))
	{
		screen_buffer=(unsigned int)screen->pixels;
		screen_pitch=screen->pitch;
		line_o=(unsigned long *) (screen_buffer +  screen_pitch * (row+screen_add));
	}
#else
	screen_buffer=(unsigned int)screen->pixels;
	register unsigned long *line_o =
		(unsigned long *) (screen_buffer +  screen_pitch * (row+screen_add));
#endif
#else
	register unsigned long *line_o =
		(unsigned long *) (screen_buffer +  screen_pitch * (row+screen_add));
	{
	  unsigned long *cpy_o=(unsigned long *)(0xe0000000 | ((unsigned)line_o) & 0x03ffffe0);
	  QACR0 = ((((unsigned int)line_o)>>26)<<2)&0x1c;
	  QACR1 = ((((unsigned int)line_o)>>26)<<2)&0x1c;
	  line_o=cpy_o;
	}
#endif
	{
	register int col;
	register unsigned char *line_i =
		(unsigned char *)(((unsigned char *)membase)+vid_adr);
	for (col = 0; col < 20; col ++) {
		register unsigned long val;
#ifdef USE_BIG_ENDIAN
		unsigned char val6 = *line_i++;
		unsigned char val7 = *line_i++;
		unsigned char val4 = *line_i++;
		unsigned char val5 = *line_i++;
		unsigned char val2 = *line_i++;
		unsigned char val3 = *line_i++;
		unsigned char val0 = *line_i++;
		unsigned char val1 = *line_i++;
#else
		unsigned char val7 = *line_i++;
		unsigned char val6 = *line_i++;
		unsigned char val5 = *line_i++;
		unsigned char val4 = *line_i++;
		unsigned char val3 = *line_i++;
		unsigned char val2 = *line_i++;
		unsigned char val1 = *line_i++;
		unsigned char val0 = *line_i++;
#endif
#ifdef DREAMCAST
		asm("pref @%0" : : "r" (line_i));
#endif

		val =     (vm2bm1[val6])
			+ (vm2bm1[val4] << 1)
			+ (vm2bm1[val2] << 2)
			+ (vm2bm1[val0] << 3);
		line_o[0] = WRL(pal16[val&0x0f],pal16[(val>>8)&0x0f]);
		line_o[1] = WRL(pal16[(val>>16)&0x0f],pal16[(val>>24)&0x0f]);

		val =     (vm2bm2[val6])
			+ (vm2bm2[val4] << 1)
			+ (vm2bm2[val2] << 2)
			+ (vm2bm2[val0] << 3);
		line_o[2] = WRL(pal16[val&0x0f],pal16[(val>>8)&0x0f]);
		line_o[3] = WRL(pal16[(val>>16)&0x0f],pal16[(val>>24)&0x0f]);

		val =     (vm2bm1[val7])
			+ (vm2bm1[val5] << 1)
			+ (vm2bm1[val3] << 2)
			+ (vm2bm1[val1] << 3);
		line_o[4] = WRL(pal16[val&0x0f],pal16[(val>>8)&0x0f]);
		line_o[5] = WRL(pal16[(val>>16)&0x0f],pal16[(val>>24)&0x0f]);

		val =     (vm2bm2[val7])
			+ (vm2bm2[val5] << 1)
			+ (vm2bm2[val3] << 2)
			+ (vm2bm2[val1] << 3);
		line_o[6] = WRL(pal16[val&0x0f],pal16[(val>>8)&0x0f]);
		line_o[7] = WRL(pal16[(val>>16)&0x0f],pal16[(val>>24)&0x0f]);
#ifdef DREAMCAST
		asm("pref @%0" : : "r" (line_o));
#endif
		line_o+=8;
	}
	}
#ifndef DREAMCAST
#ifndef DINGOO
	if (SDL_MUSTLOCK(screen))
        	SDL_UnlockSurface(screen);
#endif
#else
	line_o=(unsigned long *)0xe0000000;
	line_o[0] = line_o[8] = 0;
#endif
#if !defined(USE_DOUBLE_BUFFER) && !defined(DINGOO)
	SDL_UpdateRect(screen, 0, row+screen_add, 320, 1);
#endif
}

void Redraw_med ( int row, int vid_adr )
{
	if (vid_flag) {
		unsigned char i,r, g, b;
		vid_flag = 0;
		for (i = 0; i < 4; i++) {
#ifndef DREAMCAST
            		unsigned b = (unsigned)(36.428 * (double)((vid_col[i] & 0x7)));
            		unsigned g = (unsigned)(36.428 * (double)(((vid_col[i] >> 4) & 0x7)));
            		unsigned r = (unsigned)(36.428 * (double)(((vid_col[i] >> 8) & 0x7)));
	   		pal16[i]=(unsigned short)SDL_MapRGB(screen->format,r,g,b);
#else
            		register unsigned b = (unsigned)(4.4286 * (double)((vid_col[i] & 0x7)));
            		register unsigned g = 9 * (unsigned)(((vid_col[i] >> 4) & 0x7));
            		register unsigned r = (unsigned)(4.4286 * (double)(((vid_col[i] >> 8) & 0x7)));
			pal16[i]= (r<<11)|(g<<5)|(b);
#endif
		}
	}
	{
		register int real_row=(row+screen_add);
		if (real_row<0 || real_row>=240 || vid_adr>(MEMSIZE-160))
			return;
	}
#ifndef DREAMCAST
#ifndef ALWAYS_LOW
#ifndef DINGOO
	if (SDL_MUSTLOCK(screen))
        	if (SDL_LockSurface(screen)<0)
			return;
#endif
#endif
#ifndef USE_DOUBLE_BUFFER
	register unsigned long *line_o =
		(unsigned long *) (screen_buffer + screen_pitch * (row+screen_add));
#ifndef ALWAYS_LOW
	if ((screen_buffer!=(unsigned int)screen->pixels)||(screen_pitch!=screen->pitch))
	{
		screen_buffer=(unsigned int)screen->pixels;
		line_o=(unsigned long *) (screen_buffer +  screen_pitch * (row+screen_add));
	}
#endif
#else
	screen_buffer=(unsigned int)screen->pixels;
	register unsigned long *line_o =
		(unsigned long *) (screen_buffer + screen_pitch * (row+screen_add));
#endif
#else
// DREAMCAST:
	register unsigned long *line_o =
		(unsigned long *) (screen_buffer + screen_pitch * (row+screen_add));
	{
	  unsigned long *cpy_o=(unsigned long *)(0xe0000000 | ((unsigned)line_o) & 0x03ffffe0);
	  QACR0 = ((((unsigned int)line_o)>>26)<<2)&0x1c;
	  QACR1 = ((((unsigned int)line_o)>>26)<<2)&0x1c;
	  line_o=cpy_o;
	}
#endif
	{
	register int col;
	register unsigned *line_i =
		(unsigned *)(((unsigned char *)membase)+vid_adr);

#ifndef NO_USE_TV_FILTER
	unsigned long *line_org=line_o;
#endif
	for ( col=0; col<40; col++)
	{
		register unsigned val=*line_i++;
#ifdef DREAMCAST
		asm("pref @%0" : : "r" (line_i));
#endif

		line_o[0]= WRL(pal16[((val>>15)&1)|((val>>30)&2)],pal16[((val>>14)&1)|((val>>29)&2)]);

		line_o[1]= WRL(pal16[((val>>13)&1)|((val>>28)&2)],pal16[((val>>12)&1)|((val>>27)&2)]);

		line_o[2]= WRL(pal16[((val>>11)&1)|((val>>26)&2)],pal16[((val>>10)&1)|((val>>25)&2)]);

		line_o[3]= WRL(pal16[((val>>9)&1)|((val>>24)&2)],pal16[((val>>8)&1)|((val>>23)&2)]);

		line_o[4]= WRL(pal16[((val>>7)&1)|((val>>22)&2)],pal16[((val>>6)&1)|((val>>21)&2)]);

		line_o[5]= WRL(pal16[((val>>5)&1)|((val>>20)&2)],pal16[((val>>4)&1)|((val>>19)&2)]);

		line_o[6]= WRL(pal16[((val>>3)&1)|((val>>18)&2)],pal16[((val>>2)&1)|((val>>17)&2)]);

		line_o[7]= WRL(pal16[((val>>1)&1)|((val>>16)&2)],pal16[((val>>0)&1)|((val>>15)&2)]);

#ifdef DREAMCAST
		asm("pref @%0" : : "r" (line_o));
#endif
		line_o+=8;
	}
#ifndef NO_USE_TV_FILTER
	{
	unsigned long *line_dest = (unsigned long *)(((unsigned)line_org)+(screen_pitch/2));
	memcpy(line_dest,line_org,screen_pitch/2);
	}
#endif

	}
#ifndef DREAMCAST
#ifndef ALWAYS_LOW
#ifndef DINGOO
	if (SDL_MUSTLOCK(screen))
        	SDL_UnlockSurface(screen);
#endif
#endif
#else
	line_o=(unsigned long *)0xe0000000;
	line_o[0] = line_o[8] = 0;
#endif
#if !defined(USE_DOUBLE_BUFFER) && !defined(ALWAYS_LOW)
	SDL_UpdateRect(screen, 0, row + screen_add, 320, 1);
#endif
}

extern unsigned int fdc_commands_executed;

void render_status(void)
{
	extern int emulated_mouse;
#ifdef ALWAYS_LOW
	int i,j;
	if (actual_is_med)
	{
#ifndef DINGOO
	    if (SDL_MUSTLOCK(screen))
        	if (SDL_LockSurface(screen)<0)
			return;
#endif
	    for(j=0;j<480;j+=2)
	    {
		unsigned short *s=(unsigned short *)(screen_buffer+j*640);
		unsigned short *d=(unsigned short *)(((unsigned)screen->pixels)+screen->pitch*j/2);
		for(i=0;i<640;i+=2)
		{
			*d++=*s++;
			s++;
		}
	    }
#ifndef DINGOO
	    if (SDL_MUSTLOCK(screen))
        	SDL_UnlockSurface(screen);
#endif
#ifndef NO_VKBD
	    if (vkbd_mode)
	    {
		static Uint32 ticks=0;
		Uint32 now=SDL_GetTicks();
		vkbd_redraw();
    		if (now-ticks>100)
		{
	    		vkbd_process();
			ticks=now;
		}
	    }
#endif
	}
#endif
	if (emulated_mouse)
		vkbd_mouse();
	if (fdc_commands_executed>1)
		drawDisk();
	if (show_message)
	{
		show_message--;
		if (show_message)
			_write_text_inv_n(screen,0,28,37,show_message_str);
		else
		if (screen_add)
			render_down_screen();
		else
		{
			render_up_screen();
			if (vkbd_mode)
				vkbd_redraw();
		}
	}
#if defined(USE_DOUBLE_BUFFER) || defined(ALWAYS_LOW) || defined(DINGOO)
	//TV_Flip(screen);
	quick_flip(screen);
#endif
}
