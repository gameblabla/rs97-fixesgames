#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <GL/gl.h>

#include "tvfilter.h"

#ifndef NO_USE_TV_FILTER

#ifndef GL_UNSIGNED_SHORT_5_6_5
#define GL_UNSIGNED_SHORT_5_6_5                 0x8363
#endif

#ifndef GL_UNSIGNED_SHORT_4_4_4_4
#define GL_UNSIGNED_SHORT_4_4_4_4          	0x8033
#endif

#define _TVFILTER_SCANLINE_X 8
#define _TVFILTER_SCANLINE_Y 512
#define _TVFILTER_SCANLINE_BLEND   0x30000000

#define _TVFILTER_TVZW5_X 1024
#define _TVFILTER_TVZW5_Y 512
#define _TVFILTER_TVZW5_RED	 0x12202080
#define _TVFILTER_TVZW5_GREEN	 0x12208020
#define _TVFILTER_TVZW5_BLUE	 0x12802020

extern SDL_Surface *_tvfilter_real_screen;
extern SDL_Surface *_tvfilter_surface_screen;
extern int _tvfilter_opened_w;
extern int _tvfilter_opened_h;
extern int _tvfilter_opened_bpp;
extern int _tvfilter_opened_flags;
extern int _tvfilter_size_w;
extern int _tvfilter_size_h;
extern int _tvfilter_real_w;
extern int _tvfilter_real_h;
extern int _tvfilter_real_bpp;
extern int _tvfilter_ini_x;
extern int _tvfilter_ini_y;
extern float _tvfilter_zval;

extern unsigned *_tvfilter_surface_texture;
extern int _tvfilter_txtr_w;
extern int _tvfilter_txtr_h;
extern GLint _tvfilter_txtr;



#ifdef DEBUG_TVFILTER
void _tvfilter_show_glerror(char *m, GLenum e);
#define _TVFILTER_CHECKERROR(MSG) \
{ \
	static int yet_showed=0; \
	if (!yet_showed) \
	{ \
		GLenum chkerr=glGetError(); \
		if (chkerr!=GL_NO_ERROR) \
		{ \
			yet_showed=1;  \
			_tvfilter_show_glerror((MSG),chkerr); \
		} \
	} \
}
#else
#define _TVFILTER_CHECKERROR(MSG)
#endif


void _tvfilter_quit_scanline(void);
void _tvfilter_flip_scanline(void);
void _tvfilter_init_scanline(void);
void _tvfilter_quit_tvzw5(void);
void _tvfilter_flip_tvzw5(void);
void _tvfilter_init_tvzw5(void);
void _tvfilter_quit_texture(void);
void _tvfilter_flip_texture(void);
void _tvfilter_init_texture(unsigned bpp, unsigned w, unsigned h, SDL_Surface *back_surface);
void _tvfilter_alloc_texture(unsigned bpp,unsigned w, unsigned h, unsigned real_w, unsigned real_h);

#endif

