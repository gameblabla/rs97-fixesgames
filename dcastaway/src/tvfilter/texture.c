#ifndef NO_USE_TV_FILTER

#include "internal.h"

static void *txtr_pix=NULL;

int _tvfilter_txtr_w=0;
int _tvfilter_txtr_h=0;
unsigned *_tvfilter_surface_texture=NULL;
GLint _tvfilter_txtr=0;

static SDL_bool filter_enable=SDL_FALSE;
static SDL_bool distorsion_enable=SDL_TRUE;


void _tvfilter_quit_texture(void)
{
	if (txtr_pix)
		free(txtr_pix);
//	if (_tvfilter_surface_screen && _tvfilter_surface_screen!=_tvfilter_real_screen)
//		SDL_FreeSurface(_tvfilter_surface_screen);
	if (_tvfilter_txtr)
		glDeleteTextures(1,(GLuint *)&_tvfilter_txtr);
	_tvfilter_txtr=0;
	txtr_pix=NULL;
}


void TV_SetFilter(SDL_bool b)
{
	if (_tvfilter_real_screen)
	{
		glBindTexture(GL_TEXTURE_2D,_tvfilter_txtr);
		if (b)
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		}
		else
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//		glTexEnvi(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE, GL_MODULATE);

	}
}

void TV_ToggleFilter(void)
{
	filter_enable=!filter_enable;
	TV_SetFilter(filter_enable);
}

void TV_SetDistorsion(SDL_bool b)
{
	if (b)
		distorsion_enable=SDL_TRUE;
	else
		distorsion_enable=SDL_FALSE;
}

void TV_ToggleDistorsion(void)
{
	distorsion_enable=!distorsion_enable;
}


static void blit_txtr(int x, int y)
{
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,0.0f); glVertex3f(_tvfilter_ini_x+x, _tvfilter_ini_y+y, _tvfilter_zval);
	glTexCoord2f(1.0f,0.0f); glVertex3f(_tvfilter_txtr_w+_tvfilter_ini_x+x, _tvfilter_ini_y+y, _tvfilter_zval);
	glTexCoord2f(1.0f,1.0f); glVertex3f(_tvfilter_txtr_w+_tvfilter_ini_x+x, _tvfilter_txtr_h+_tvfilter_ini_y+y, _tvfilter_zval);
	glTexCoord2f(0.0f,1.0f); glVertex3f(_tvfilter_ini_x+x, _tvfilter_txtr_h+_tvfilter_ini_y+y, _tvfilter_zval);
	glEnd();
	_tvfilter_zval+=0.001f;
}

void _tvfilter_flip_texture(void)
{
	glBindTexture(GL_TEXTURE_2D,_tvfilter_txtr);
	_TVFILTER_CHECKERROR("_tvfilter_flip glBindTexture");
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	_TVFILTER_CHECKERROR("_tvfilter_flip glColor4f");
	blit_txtr(0,0);
	_TVFILTER_CHECKERROR("_tvfilter_flip _tvfilter_blit_txtr");

	if (distorsion_enable)
	{
		glColor4f(1.0f,1.0f,1.0f,0.03f);
		blit_txtr(-1,-1);
//		blit_txtr(-1, 0);
		blit_txtr(-1, 1);
//		blit_txtr( 0,-1);
//		blit_txtr( 0, 1);
		blit_txtr( 1,-1);
//		blit_txtr( 1, 0);
		blit_txtr( 1, 1);
	}
}

void _tvfilter_alloc_texture(unsigned bpp,unsigned w, unsigned h, unsigned real_w, unsigned real_h)
{
	int i;
	for(i=8;i<real_w*8;i*=2)
		if (w<=i)
		{
			_tvfilter_txtr_w=i;
			break;
		}
	for(i=8;i<_tvfilter_real_h*8;i*=2)
		if (h<=i)
		{
			_tvfilter_txtr_h=i;
			break;
		}
	txtr_pix=calloc(_tvfilter_txtr_w*_tvfilter_txtr_h,(bpp/8));
#ifdef DEBUG_TVFILTER
	printf(" Texture Width=%i, Height=%i, BPP=%i\n",_tvfilter_txtr_w,_tvfilter_txtr_h,bpp);fflush(stdout);
#endif
}



void _tvfilter_init_texture(unsigned bpp, unsigned w, unsigned h, SDL_Surface *back_surface)
{
	glGenTextures(1,(GLuint *)&_tvfilter_txtr);
	_TVFILTER_CHECKERROR("TV_Init glGenTextures _tvfilter_txtr");
	glBindTexture(GL_TEXTURE_2D,_tvfilter_txtr);
	_TVFILTER_CHECKERROR("TV_Init glBindTexture _tvfilter_txtr");

	if (_tvfilter_surface_texture)
	{
		free(_tvfilter_surface_texture);
		_tvfilter_surface_texture=NULL;
	}

	switch(bpp)
	{
		case 16:
			_tvfilter_surface_screen = SDL_CreateRGBSurfaceFrom(txtr_pix, w, h, 16, _tvfilter_txtr_w*2 , 0xF800, 0x7E0, 0x1F, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, _tvfilter_txtr_w, _tvfilter_txtr_h, 0,GL_RGB, GL_UNSIGNED_SHORT_5_6_5, txtr_pix);
				if (glGetError()==GL_NO_ERROR)
				break;
			_tvfilter_surface_texture=calloc(_tvfilter_txtr_w*_tvfilter_txtr_h,(32/8));
			glTexImage2D(GL_TEXTURE_2D, 0, 3, _tvfilter_txtr_w, _tvfilter_txtr_h, 0,GL_RGBA, GL_UNSIGNED_BYTE, _tvfilter_surface_texture);
			break;
		case 32:
			_tvfilter_surface_screen = SDL_CreateRGBSurfaceFrom(txtr_pix, w, h, 32, _tvfilter_txtr_w*4 , 0xFF, 0xFF00, 0xFF0000, 0x0);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, _tvfilter_txtr_w, _tvfilter_txtr_h, 0,GL_RGBA, GL_UNSIGNED_BYTE, txtr_pix);
			break;
		default:
			_tvfilter_surface_screen=NULL;		
	}
	_TVFILTER_CHECKERROR("TV_Init glTexImage2D _tvfilter_txtr");
	if (_tvfilter_surface_screen && back_surface)
	{
		SDL_Surface tmp;
		//memcpy((void *)&tmp,back_surface,sizeof(SDL_Surface));
		//memcpy(back_surface,_tvfilter_surface_screen,sizeof(SDL_Surface));
		//memcpy(_tvfilter_surface_screen,(void *)&tmp,sizeof(SDL_Surface));
		tmp=*back_surface;
		*back_surface=*_tvfilter_surface_screen;
		*_tvfilter_surface_screen=tmp;
		SDL_FreeSurface(_tvfilter_surface_screen);
		_tvfilter_surface_screen=back_surface;
	}
	if (filter_enable)
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) _tvfilter_txtr");
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	else
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) _tvfilter_txtr");
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}
	_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) _tvfilter_txtr");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP) _tvfilter_txtr");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_TRAP_S, GL_CLAMP) _tvfilter_txtr");
//	glTexEnvi(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE, GL_MODULATE);
//	_TVFILTER_CHECKERROR("TV_Init glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE) _tvfilter_txtr");
}

#endif

