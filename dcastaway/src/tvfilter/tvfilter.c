#ifndef NO_USE_TV_FILTER

#include "internal.h"

SDL_Surface *_tvfilter_real_screen=NULL, *_tvfilter_surface_screen=NULL;
int _tvfilter_opened_w=0;
int _tvfilter_opened_h=0;
int _tvfilter_opened_bpp=0;
int _tvfilter_opened_flags=0;
int _tvfilter_size_w=0;
int _tvfilter_size_h=0;
int _tvfilter_real_w=0;
int _tvfilter_real_h=0;
int _tvfilter_real_bpp=0;
int _tvfilter_ini_x=0;
int _tvfilter_ini_y=0;
float _tvfilter_zval=0.001f;

static int to_resize=0;

#ifdef DEBUG_TVFILTER
void _tvfilter_show_glerror(char *m, GLenum e)
{
	if (e==GL_NO_ERROR)
		fprintf(stderr,"!!-- OpenGL ERROR GL_NO_ERROR (#%i): '%s'\n",e,m);
	else if (e==GL_INVALID_ENUM)
		fprintf(stderr,"!!-- OpenGL ERROR GL_INVALID_ENUM (#%i): '%s'\n",e,m);
	else if (e==GL_INVALID_VALUE)
		fprintf(stderr,"!!-- OpenGL ERROR GL_INVALID_VALUE (#%i): '%s'\n",e,m);
	else if (e==GL_INVALID_OPERATION)
		fprintf(stderr,"!!-- OpenGL ERROR GL_INVALID_OPERATION (#%i): '%s'\n",e,m);
	else if (e==GL_STACK_OVERFLOW)
		fprintf(stderr,"!!-- OpenGL ERROR GL_STACK_OVERFLOW (#%i): '%s'\n",e,m);
	else if (e==GL_STACK_UNDERFLOW)
		fprintf(stderr,"!!-- OpenGL ERROR GL_STACK_UNDERFLOW (#%i): '%s'\n",e,m);
	else if (e==GL_OUT_OF_MEMORY)
		fprintf(stderr,"!!-- OpenGL ERROR GL_OUT_OF_MEMORY (#%i): '%s'\n",e,m);
	else
		fprintf(stderr,"!!-- OpenGL UNKNOWN ERROR (#%i): '%s'\n",e,m);
	fflush(stderr);
}
#endif


void TV_Quit(void)
{
	_tvfilter_quit_texture();
	_tvfilter_quit_scanline();
	_tvfilter_quit_tvzw5();
	_tvfilter_real_screen=NULL;
	if (SDL_WasInit(SDL_INIT_VIDEO))
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void TV_ToggleFullScreen(SDL_Surface *screen)
{
	if (_tvfilter_real_screen)
		TV_Init(_tvfilter_opened_w,_tvfilter_opened_h,_tvfilter_opened_bpp,_tvfilter_opened_flags^SDL_FULLSCREEN);
	else
		SDL_WM_ToggleFullScreen(_tvfilter_surface_screen);
}

void TV_SetFullScreen(SDL_Surface *screen, SDL_bool b)
{
	if (_tvfilter_real_screen)
	{
		if (b)
		{
			if (!(_tvfilter_opened_flags&SDL_FULLSCREEN))
					TV_ToggleFullScreen(screen);
		}
		else
		{
			if (_tvfilter_opened_flags&SDL_FULLSCREEN)
					TV_ToggleFullScreen(screen);
		}
	}
}

static void real_resize(void)
{
	if (_tvfilter_real_screen && !(_tvfilter_opened_flags&SDL_FULLSCREEN))
	{
		TV_Init(_tvfilter_opened_w,_tvfilter_opened_h,_tvfilter_opened_bpp,_tvfilter_opened_flags);
	}
}

void TV_ResizeWindow(int w, int h)
{
	to_resize=8;
	_tvfilter_size_w=w;
	_tvfilter_size_h=h;
}

int TV_ConvertMousePosX(int x)
{
	if (!_tvfilter_opened_w)
		return 0;
//	return ((x*_tvfilter_real_w)/_tvfilter_opened_w);
	return ((x*_tvfilter_opened_w)/_tvfilter_real_w);
}

int TV_ConvertMousePosY(int y)
{
	if (!_tvfilter_opened_h)
		return 0;
//	return ((y*_tvfilter_real_h)/_tvfilter_opened_h);
	return ((y*_tvfilter_opened_h)/_tvfilter_real_h);
}


static __inline__ void real_flip(void)
{
	if (!_tvfilter_real_screen)
		return;

	if (_tvfilter_surface_screen==_tvfilter_real_screen)
	{
		SDL_Flip(_tvfilter_real_screen);
		return;
	}

	_tvfilter_zval=0.001f;

	glClear(GL_COLOR_BUFFER_BIT);
	_TVFILTER_CHECKERROR("_tvfilter_flip glClear");

	glLoadIdentity();
	_TVFILTER_CHECKERROR("_tvfilter_flip glLoadIdentity");

	_tvfilter_flip_texture();
	_tvfilter_flip_scanline();
	_tvfilter_flip_tvzw5();

	SDL_GL_SwapBuffers();
	_TVFILTER_CHECKERROR("_tvfilter_flip SDL_GL_SwapBuffers");
}


static void convert_to_texture(void *src_ptr)
{
	unsigned char *dst=(unsigned char *)_tvfilter_surface_texture;
	unsigned short *src=(unsigned short *)src_ptr;
	int i,max=_tvfilter_txtr_h*_tvfilter_txtr_w;
	for(i=0;i<max;i++)
	{
		unsigned short d=*src++;
		*dst++=(d&0xF800)>>(11-3);
		*dst++=(d&0xFE0)>>(5-2);
		*dst++=(d&0x1F)<<3;
		dst++;
	}
}

void TV_Flip(SDL_Surface *screen)
{
	if (!screen)
		return;

	if (to_resize)
	{
		to_resize--;
		if (!to_resize)
			real_resize();
	}
	if (_tvfilter_surface_screen==_tvfilter_real_screen)
		SDL_Flip(screen);
	else
	{
		glBindTexture(GL_TEXTURE_2D,_tvfilter_txtr);
		_TVFILTER_CHECKERROR("TV_Flip glBindTexture");
		switch(screen->format->BitsPerPixel)
		{
			case 16:
				if (!_tvfilter_surface_texture)
					glTexSubImage2D(GL_TEXTURE_2D,0,0,0,_tvfilter_txtr_w,_tvfilter_txtr_h,GL_RGB, GL_UNSIGNED_SHORT_5_6_5, screen->pixels);
				else
				{
					convert_to_texture(screen->pixels);
					glTexSubImage2D(GL_TEXTURE_2D,0,0,0,_tvfilter_txtr_w,_tvfilter_txtr_h,GL_RGBA, GL_UNSIGNED_BYTE, _tvfilter_surface_texture);
				}
				_TVFILTER_CHECKERROR("TV_Flip glTexSubImage2D 16");
				break;
			case 32:
				glTexSubImage2D(GL_TEXTURE_2D,0,0,0,_tvfilter_txtr_w,_tvfilter_txtr_h,GL_RGBA, GL_UNSIGNED_BYTE, screen->pixels);
				_TVFILTER_CHECKERROR("TV_Flip glTexSubImage2D 32");
		}
		real_flip();
	}
}

SDL_Surface *TV_Init(int w, int h, int bpp, int flags)
{
	int i;
	const SDL_VideoInfo *info;
	SDL_Surface *back_surface=_tvfilter_surface_screen;
	if (bpp!=32)
		bpp=16;
#ifdef DEBUG_TVFILTER
	printf("TV_Init(w=%i, h=%i, bpp=%i, flags=0x%X)\n",w,h,bpp,flags);fflush(stdout);
	{
	 SDL_version ver;
	 SDL_VERSION(&ver);
	 printf(" SDL compile-time version %u.%u.%u\n", ver.major, ver.minor, ver.patch);
	 ver = *SDL_Linked_Version();
	 printf(" SDL runtime version %u.%u.%u\n", ver.major, ver.minor, ver.patch);
	}
	fflush(stdout);
#endif
	TV_Quit();
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	SDL_putenv("SDL_VIDEO_CENTERED=center");
#ifdef WIN32
//	SDL_putenv("SDL_VIDEODRIVER=directx");
#endif
	info = SDL_GetVideoInfo();
	if (info)
	{
		if (info->hw_available)
			flags|=SDL_HWSURFACE;
		else
			flags|=SDL_SWSURFACE;
		if (info->blit_hw)
			flags|=SDL_HWACCEL;
#ifdef DEBUG_TVFILTER
		printf(" Hardware Available = %i\n",info->hw_available);
		printf(" Hardware Blit = %i\n",info->blit_hw);
		fflush(stdout);
#endif
	}

	_tvfilter_real_w=0; _tvfilter_real_h=0;
	if (info)
	{
		SDL_Rect **modes=SDL_ListModes(info->vfmt,SDL_FULLSCREEN);
		if (modes && modes != (SDL_Rect **)-1)
		{
			for(i=0;modes[i];i++)
			{
				if (modes[i]->w>_tvfilter_real_w && modes[i]->w>w && modes[i]->h>_tvfilter_real_h)
				{
					_tvfilter_real_w=modes[i]->w;
					_tvfilter_real_h=modes[i]->h;
				}
			}
//			free(modes);
		}
	}
	if (flags&SDL_DOUBLEBUF || flags&SDL_GL_DOUBLEBUFFER)
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		flags|=SDL_DOUBLEBUF|SDL_GL_DOUBLEBUFFER;
	}
#if 0
	if (bpp==16)
	{
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 4 );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 4 );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 4 );
		SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 4 );
	}
	else
	{
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	}
#endif
	flags|=SDL_OPENGL|SDL_RESIZABLE;

	if (flags&SDL_FULLSCREEN)
	{
#ifdef DEBUG_TVFILTER
		puts(" Fullscreen");
#endif
		_tvfilter_real_screen=SDL_SetVideoMode(0,0,0,flags);
		if (!_tvfilter_real_screen)
			_tvfilter_real_screen=SDL_SetVideoMode(0,0,bpp,flags);
		if (!_tvfilter_real_screen)
			_tvfilter_real_screen=SDL_SetVideoMode(_tvfilter_real_w,_tvfilter_real_h,bpp,flags);
		if (!_tvfilter_real_screen)
			_tvfilter_real_screen=SDL_SetVideoMode(_tvfilter_real_w,_tvfilter_real_h,0,flags);
	}
	else
	{
		if (!_tvfilter_size_w && !_tvfilter_size_h)
		{
			if ((2*_tvfilter_real_w)/3>=w*2 && (2*_tvfilter_real_h)/3>=h*2)
			{
				_tvfilter_size_w=w*2;
				_tvfilter_size_h=h*2;
			}
			else
			if (_tvfilter_real_w>=w && _tvfilter_real_h>=h)
			{
				_tvfilter_size_w=w;
				_tvfilter_size_h=h;
			}
			else
			{
				_tvfilter_size_w=_tvfilter_real_w;
				_tvfilter_size_h=_tvfilter_real_h;
			}
		}
#ifdef DEBUG_TVFILTER
		printf(" Windowed %i x %i\n",_tvfilter_size_w,_tvfilter_size_h);
#endif
		_tvfilter_real_screen=SDL_SetVideoMode(_tvfilter_size_w,_tvfilter_size_h,0,flags);
		if (!_tvfilter_real_screen)
			_tvfilter_real_screen=SDL_SetVideoMode(w,h,0,flags);
		if (!_tvfilter_real_screen)
			_tvfilter_real_screen=SDL_SetVideoMode(_tvfilter_size_w,_tvfilter_size_h,bpp,flags);
		if (!_tvfilter_real_screen)
			_tvfilter_real_screen=SDL_SetVideoMode(w,h,bpp,flags);
	}
	if (_tvfilter_real_screen)
	{
		_tvfilter_opened_w=w;
		_tvfilter_opened_h=h;
		_tvfilter_opened_bpp=bpp;
		_tvfilter_opened_flags=flags;
		_tvfilter_real_w=_tvfilter_real_screen->w;
		_tvfilter_real_h=_tvfilter_real_screen->h;
		_tvfilter_real_bpp=_tvfilter_real_screen->format->BitsPerPixel;
#ifdef DEBUG_TVFILTER
		{
			char buf[1024];
			printf(" Video Driver '%s'\n",SDL_VideoDriverName(buf,1024));
		}
		i=0; SDL_GL_GetAttribute(SDL_GL_RED_SIZE,&i);
		printf(" SDL_GL_RED_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE,&i);
		printf(" SDL_GL_GREEN_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE,&i);
		printf(" SDL_GL_BLUE_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE,&i);
		printf(" SDL_GL_ALPHA_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE,&i);
		printf(" SDL_GL_BUFFER_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER,&i);
		printf(" SDL_GL_DOUBLEBUFFER = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE,&i);
		printf(" SDL_GL_DEPTH_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE,&i);
		printf(" SDL_GL_STENCIL_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_ACCUM_RED_SIZE,&i);
		printf(" SDL_GL_ACCUM_RED_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_ACCUM_GREEN_SIZE,&i);
		printf(" SDL_GL_ACCUM_GREEN_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_ACCUM_BLUE_SIZE,&i);
		printf(" SDL_GL_ACCUM_BLUE_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,&i);
		printf(" SDL_GL_ACCUM_ALPHA_SIZE = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_STEREO,&i);
		printf(" SDL_GL_STEREO = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS,&i);
		printf(" SDL_GL_MULTISAMPLEBUFFERS = %i\n",i);
		i=0; SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES,&i);
		printf(" SDL_GL_MULTISAMPLESAMPLES = %i\n",i);
		printf(" OpenGL Vendor '%s'\n",glGetString(GL_VENDOR));
		printf(" OpenGL Renderer '%s'\n",glGetString(GL_RENDERER));
		printf(" OpenGL Version '%s'\n",glGetString(GL_VERSION));
		printf(" OpenGL Extensions '%s'\n",glGetString(GL_EXTENSIONS));
		i=0; glGetIntegerv(GL_MAX_TEXTURE_SIZE,&i);
		printf(" GL_MAX_TEXTURE_SIZE = %i\n",i);
		i=0; glGetIntegerv(GL_RGBA_MODE,&i);
		printf(" GL_RGBA_MODE = %i\n",i);
		printf(" Screen Width=%i, Height=%i, BPP=%i\n",_tvfilter_real_w,_tvfilter_real_h,_tvfilter_real_bpp);
		printf(" Screen BytesPerPixel=%i, Rmask=%X, Gmask=%X, Bmask=%X, Amask=%X (%i)\n",_tvfilter_real_screen->format->BytesPerPixel, _tvfilter_real_screen->format->Rmask, _tvfilter_real_screen->format->Gmask, _tvfilter_real_screen->format->Bmask, _tvfilter_real_screen->format->Amask,_tvfilter_real_screen->format->alpha);
		fflush(stdout);
#endif
		_tvfilter_alloc_texture(bpp,w,h,_tvfilter_real_w,_tvfilter_real_h);
		glDisable(GL_DEPTH_TEST);
		_TVFILTER_CHECKERROR("TV_Init glDisable(GL_DEPTH_TEST)");
		glClearColor(0.0, 0.0, 0.0, 0.0);
		_TVFILTER_CHECKERROR("TV_Init glClearColor");
		glViewport(0, 0, _tvfilter_real_w, _tvfilter_real_h);
		_TVFILTER_CHECKERROR("TV_Init glViewport");
		glMatrixMode( GL_PROJECTION );
		_TVFILTER_CHECKERROR("TV_Init glMatrixMode(GL_PROJECTION)");
		if (_tvfilter_real_w>=_tvfilter_real_h)
		{
			double _tvfilter_real_ratio=(double)_tvfilter_real_w / (double)_tvfilter_real_h;
			double nh=(double)w / _tvfilter_real_ratio;
			if (nh>=h)
			{
				_tvfilter_ini_x=0;
				_tvfilter_ini_y=(((int)nh-h)/2) / _tvfilter_real_ratio;
				glOrtho(0.0, w, nh, 0.0, -50.0, 50.0 );
			}
			else
			{
				double nw=(double)h * _tvfilter_real_ratio;
				_tvfilter_ini_x=(((int)nw-w)/2);
				_tvfilter_ini_y=0;
				glOrtho(0.0, nw, h, 0.0, -50.0, 50.0 );
			}
		}
		else
		{
			double _tvfilter_real_ratio=(double)_tvfilter_real_h / (double)_tvfilter_real_w;
			double nw=(double)h / _tvfilter_real_ratio;
			if (nw>=w)
			{
				_tvfilter_ini_x=(((int)nw-w)/2) / _tvfilter_real_ratio;
				_tvfilter_ini_y=0;
				glOrtho(0.0, nw, h, 0.0, -50.0, 50.0 );
			}
			else
			{
				double nh=(double)w * _tvfilter_real_ratio;
				_tvfilter_ini_x=0;
				_tvfilter_ini_y=(((int)nh-h)/2);
				glOrtho(0.0, w, nh, 0.0, -50.0, 50.0 );
			}
		}
		_TVFILTER_CHECKERROR("TV_Init glOrtho");
		glMatrixMode( GL_MODELVIEW );
		_TVFILTER_CHECKERROR("TV_Init glMatrixMode(GL_MODELVIEW)");
		glLoadIdentity();
		_TVFILTER_CHECKERROR("TV_Init glLoadIdentity");

		glEnable(GL_TEXTURE_2D);
		_TVFILTER_CHECKERROR("TV_Init glEnable(GL_TEXTURE_2D)");
		glEnable(GL_BLEND);
		_TVFILTER_CHECKERROR("TV_Init glEnable(GL_BLEND)");
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		_TVFILTER_CHECKERROR("TV_Init glBlendFunc");

		_tvfilter_init_scanline();
		_tvfilter_init_tvzw5();
		_tvfilter_init_texture(bpp,w,h,back_surface);

		if (flags&SDL_FULLSCREEN)
			_tvfilter_surface_screen->flags|=SDL_FULLSCREEN;
		return _tvfilter_surface_screen;
	}
#ifdef DEBUG_TVFILTER
	puts(" NOT OPENGL!!"); fflush(stdout);
#endif
	flags&=~(SDL_OPENGL|SDL_RESIZABLE);

	if (info)
	{
		SDL_Rect **modes=SDL_ListModes(info->vfmt,SDL_FULLSCREEN);
		if (modes && modes != (SDL_Rect **)-1)
		{
			for(i=0;modes[i];i++)
			{
				if (modes[i]->w<_tvfilter_real_w && modes[i]->w<w && modes[i]->h>_tvfilter_real_h && modes[i]->w<=w && modes[i]->h<=h)
				{
					_tvfilter_real_w=modes[i]->w;
					_tvfilter_real_h=modes[i]->h;
				}
			}
//			free(modes);
		}
	}

	if (!_tvfilter_real_screen)
		_tvfilter_real_screen=SDL_SetVideoMode(w, h, bpp,flags);

	if (!_tvfilter_real_screen)
		_tvfilter_real_screen=SDL_SetVideoMode(_tvfilter_real_w,_tvfilter_real_h, bpp,flags);

	if (!_tvfilter_real_screen)
		_tvfilter_real_screen=SDL_SetVideoMode(0,0, bpp,flags);


	if (_tvfilter_real_screen)
	{
		_tvfilter_opened_w=w;
		_tvfilter_opened_h=h;
		_tvfilter_opened_bpp=bpp;
		_tvfilter_opened_flags=flags;
	}
	_tvfilter_surface_screen=_tvfilter_real_screen;
	return _tvfilter_real_screen;
}

#endif

