#ifndef NO_USE_TV_FILTER

#include "internal.h"

static unsigned tvzw5[_TVFILTER_TVZW5_Y][_TVFILTER_TVZW5_X];
static GLint tvzw5_txtr=0;
static SDL_bool tvzw5_enable=SDL_TRUE;
static float scale_w=1.0f;
static float scale_h=1.0f;


void _tvfilter_quit_tvzw5(void)
{
	if (tvzw5_txtr)
		glDeleteTextures(1,(GLuint *)&tvzw5_txtr);
	tvzw5_txtr=0;
}


void TV_SetTV(SDL_bool b)
{
	if (b)
		tvzw5_enable=SDL_TRUE;
	else
		tvzw5_enable=SDL_FALSE;
}

void TV_ToggleTV(void)
{
	tvzw5_enable=!tvzw5_enable;
}


void _tvfilter_flip_tvzw5(void)
{
	if (tvzw5_enable)
	{
		glBindTexture(GL_TEXTURE_2D,tvzw5_txtr);
		_TVFILTER_CHECKERROR("_tvfilter_flip tvzw5");
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		_TVFILTER_CHECKERROR("_tvfilter_flip glColor4f(1.0f,1.0f,1.0f,1.0f)");
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f); glVertex3f(_tvfilter_ini_x, _tvfilter_ini_y, _tvfilter_zval);
		glTexCoord2f(scale_w,0.0f); glVertex3f(_tvfilter_ini_x+_tvfilter_opened_w, _tvfilter_ini_y, _tvfilter_zval);
		glTexCoord2f(scale_w,scale_h); glVertex3f(_tvfilter_ini_x+_tvfilter_opened_w, _tvfilter_ini_y+_tvfilter_opened_h, _tvfilter_zval);
		glTexCoord2f(0.0f,scale_h); glVertex3f(_tvfilter_ini_x, _tvfilter_ini_y+_tvfilter_opened_h, _tvfilter_zval);
#if 0
		glColor4f(1.0f,1.0f,1.0f,0.4f);
		glTexCoord2f(0.0f,0.0f); glVertex3f(_tvfilter_ini_x+2, _tvfilter_ini_y+2, _tvfilter_zval);
		glTexCoord2f(scale_w,0.0f); glVertex3f(_tvfilter_ini_x+_tvfilter_opened_w+2, _tvfilter_ini_y+2, _tvfilter_zval);
		glTexCoord2f(scale_w,scale_h); glVertex3f(_tvfilter_ini_x+_tvfilter_opened_w+2, _tvfilter_ini_y+_tvfilter_opened_h+2, _tvfilter_zval);
		glTexCoord2f(0.0f,scale_h); glVertex3f(_tvfilter_ini_x+2, _tvfilter_ini_y+_tvfilter_opened_h+2, _tvfilter_zval);
		glColor4f(1.0f,1.0f,1.0f,0.4f);
		glTexCoord2f(0.0f,0.0f); glVertex3f(_tvfilter_ini_x-2, _tvfilter_ini_y-2, _tvfilter_zval);
		glTexCoord2f(scale_w,0.0f); glVertex3f(_tvfilter_ini_x+_tvfilter_opened_w-2, _tvfilter_ini_y-2, _tvfilter_zval);
		glTexCoord2f(scale_w,scale_h); glVertex3f(_tvfilter_ini_x+_tvfilter_opened_w-2, _tvfilter_ini_y+_tvfilter_opened_h-2, _tvfilter_zval);
		glTexCoord2f(0.0f,scale_h); glVertex3f(_tvfilter_ini_x-2, _tvfilter_ini_y+_tvfilter_opened_h-2, _tvfilter_zval);
#endif
		glEnd();
		_TVFILTER_CHECKERROR("_tvfilter_flip glEnd");
	}
}


void _tvfilter_init_tvzw5(void)
{
	int i;
	double w,h;
	glGenTextures(1,(GLuint *)&tvzw5_txtr);
	_TVFILTER_CHECKERROR("TV_Init glGenTextures tvzw5");
	glBindTexture(GL_TEXTURE_2D,tvzw5_txtr);
	_TVFILTER_CHECKERROR("TV_Init glBindTexture tvzw5");
	for(i=0;i<_TVFILTER_TVZW5_X;i+=6)
	{
#if 1
		tvzw5[0][i+0]=_TVFILTER_TVZW5_GREEN;
		tvzw5[0][i+1]=_TVFILTER_TVZW5_RED;
		tvzw5[0][i+2]=_TVFILTER_TVZW5_RED;
		tvzw5[0][i+3]=_TVFILTER_TVZW5_BLUE;
		tvzw5[0][i+4]=_TVFILTER_TVZW5_BLUE;
		tvzw5[0][i+5]=_TVFILTER_TVZW5_GREEN;
#else
		tvzw5[0][i+0]=_TVFILTER_TVZW5_GREEN;
		tvzw5[0][i+1]=_TVFILTER_TVZW5_RED;
		tvzw5[0][i+2]=_TVFILTER_TVZW5_BLUE;
		tvzw5[0][i+3]=_TVFILTER_TVZW5_GREEN;
		tvzw5[0][i+4]=_TVFILTER_TVZW5_RED;
		tvzw5[0][i+5]=_TVFILTER_TVZW5_BLUE;
#endif
	}
	for(i=0;i<_TVFILTER_TVZW5_X;i+=6)
	{
#if 1
		tvzw5[1][i+0]=_TVFILTER_TVZW5_BLUE;
		tvzw5[1][i+1]=_TVFILTER_TVZW5_BLUE;
		tvzw5[1][i+2]=_TVFILTER_TVZW5_GREEN;
		tvzw5[1][i+3]=_TVFILTER_TVZW5_GREEN;
		tvzw5[1][i+4]=_TVFILTER_TVZW5_RED;
		tvzw5[1][i+5]=_TVFILTER_TVZW5_RED;
#else
		tvzw5[1][i+0]=_TVFILTER_TVZW5_BLUE;
		tvzw5[1][i+1]=_TVFILTER_TVZW5_GREEN;
		tvzw5[1][i+2]=_TVFILTER_TVZW5_RED;
		tvzw5[1][i+3]=_TVFILTER_TVZW5_BLUE;
		tvzw5[1][i+4]=_TVFILTER_TVZW5_GREEN;
		tvzw5[1][i+5]=_TVFILTER_TVZW5_RED;
#endif
	}
	for(i=2;i<_TVFILTER_TVZW5_Y;i+=2)
		memcpy(&tvzw5[i][0],&tvzw5[0][0],_TVFILTER_TVZW5_X*2*sizeof(unsigned));
	glTexImage2D(GL_TEXTURE_2D, 0, 4, _TVFILTER_TVZW5_X, _TVFILTER_TVZW5_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tvzw5);
	_TVFILTER_CHECKERROR("TV_Init glTexImage2D tvzw5");
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) tvzw5");
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) tvzw5");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP) tvzw5");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	_TVFILTER_CHECKERROR("TV_Init glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_TRAP_S, GL_CLAMP) tvzw5");
//	glTexEnvi(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE, GL_BLEND);
//	_TVFILTER_CHECKERROR("TV_Init glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_BLEND) tvzw5");
	if (_tvfilter_real_w)
	{
		w=_tvfilter_real_w;
		h=_tvfilter_real_h;
	}
	else
	{
		w=_tvfilter_size_w;
		h=_tvfilter_size_h;
	}
	scale_w=w/((double)_TVFILTER_TVZW5_X);
	scale_h=h/((double)_TVFILTER_TVZW5_Y);
	if (scale_w>1.0f)
		scale_w=1.0f;
	if (scale_h>1.0f)
		scale_h=1.0f;
}

#endif

