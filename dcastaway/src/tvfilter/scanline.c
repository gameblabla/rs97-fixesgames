#ifndef NO_USE_TV_FILTER

#include "internal.h"

static unsigned scanline[_TVFILTER_SCANLINE_Y][_TVFILTER_SCANLINE_X];
static GLint scanline_txtr=0;
static SDL_bool scanline_enable=SDL_TRUE;
static double scale_h=1.0f;


void _tvfilter_quit_scanline(void)
{
	if (scanline_txtr)
		glDeleteTextures(1,(GLuint *)&scanline_txtr);
	scanline_txtr=0;
}

void TV_SetScanlines(SDL_bool b)
{
	if (b)
		scanline_enable=SDL_TRUE;
	else
		scanline_enable=SDL_FALSE;
}

void TV_ToggleScanlines(void)
{
	scanline_enable=!scanline_enable;
}




void _tvfilter_flip_scanline(void)
{
	if (scanline_enable)
	{
		glBindTexture(GL_TEXTURE_2D,scanline_txtr);
		_TVFILTER_CHECKERROR("_tvfilter_flip scanline_enable");
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		_TVFILTER_CHECKERROR("_tvfilter_flip glColor4f(1.0f,1.0f,1.0f,1.0f)");
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f); glVertex3f(_tvfilter_ini_x, _tvfilter_ini_y, _tvfilter_zval);
		glTexCoord2f(1.0f,0.0f); glVertex3f(_tvfilter_ini_x+_tvfilter_opened_w, _tvfilter_ini_y, _tvfilter_zval);
		glTexCoord2f(1.0f,scale_h); glVertex3f(_tvfilter_ini_x+_tvfilter_opened_w, _tvfilter_ini_y+_tvfilter_opened_h, _tvfilter_zval);
		glTexCoord2f(0.0f,scale_h); glVertex3f(_tvfilter_ini_x, _tvfilter_ini_y+_tvfilter_opened_h, _tvfilter_zval);
		glEnd();
		_TVFILTER_CHECKERROR("_tvfilter_flip glEnd");
	}
}

void _tvfilter_init_scanline(void)
{
	int i;
	double h;
	glGenTextures(1,(GLuint *)&scanline_txtr);
	_TVFILTER_CHECKERROR("TV_Init glGenTextures scanline");
	glBindTexture(GL_TEXTURE_2D,scanline_txtr);
	_TVFILTER_CHECKERROR("TV_Init glBindTexture scanline");
	for(i=0;i<_TVFILTER_SCANLINE_Y;i+=2)
	{
		int j;
		for(j=0;j<_TVFILTER_SCANLINE_X;j++)
			scanline[i][j]=_TVFILTER_SCANLINE_BLEND;
		memset(&scanline[i+1][0],0,sizeof(unsigned)*_TVFILTER_SCANLINE_X);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, 4, _TVFILTER_SCANLINE_X, _TVFILTER_SCANLINE_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, scanline);
	_TVFILTER_CHECKERROR("TV_Init glTexImage2D scanline");
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) scanline");
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) scanline");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP) scanline");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_TRAP_S, GL_CLAMP) scanline");
//	glTexEnvi(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE, GL_BLEND);
//	_TVFILTER_CHECKERROR("TV_Init glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_BLEND) scanline");
	if (_tvfilter_real_h)
		h=_tvfilter_real_h;
	else
		h=_tvfilter_size_h;
	scale_h=h/((double)_TVFILTER_TVZW5_Y);
	if (scale_h>1.0f)
		scale_h=1.0f;
}

#endif

