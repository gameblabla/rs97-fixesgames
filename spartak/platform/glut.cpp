/*
Spartak Chess based on stockfish engine.
Copyright (C) 2010 djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_GLUT

#include <GL/glut.h>
#include <ctype.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include "../ui/dialog.h"
#include "../game.h"
#include "../io.h"

static eGame* game = NULL;

static dword tex[512*256];

static const GLushort vertices[4 * 2] =
{
	0, 0,
	0, 1,
	1, 1,
	1, 0,
};
static const GLubyte triangles[2 * 3] =
{
	0, 1, 2,
	0, 2, 3,
};

static void DrawGL(int _w, int _h)
{
	if(!game->Desktop().Update())
		return;
	dword* p = tex;
	eRGBA* data = game->Desktop().Buffer();
	for(int y = 0; y < 240; ++y)
	{
		for(int x = 0; x < 320; ++x)
		{
			*p++ = *data++;
		}
		p += 512 - 320;
	}

	int w = _w;
	int h = _h;
	if(float(w)/h > 4.0f/3.0f)
		w = float(_h)*4/3;
	else
		h = float(_w)*3/4;

	GLint filter = w % 320 ? GL_LINEAR : GL_NEAREST;

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(320.0f/512.0f, 240.0f/256.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
	glViewport((_w - w)/2, (_h - h)/2, w, h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_SHORT, 0, vertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_SHORT, 0, vertices);
	glDrawElements(GL_TRIANGLES, 2*3, GL_UNSIGNED_BYTE, triangles);

	glFlush();
	glutSwapBuffers();
}

static int window = -1;
static int w = 1, h = 1;

static void OnResizeWindow(int Width, int Height)
{
	if(Height == 0)
		Height = 1;
	w = Width;
	h = Height;
}

static void OnDraw()
{
	if(game->Update())
	{
		DrawGL(w, h);
	}
	else
		exit(0);
}
static void OnIdle(int timer)
{
	glutPostRedisplay();
	glutTimerFunc(15, OnIdle, 0);
}
static void OnKeyDown(byte _key, int x, int y)
{
	switch(_key)
	{
	case '`':	game->Command('n'); 	break;
	case '\r':	game->Command('a'); 	break;
	case 27:	game->Command('b'); 	break;//esc
	case '\\':	game->Command('g'); 	break;
	case ' ':	game->Command('f'); 	break;
	}
}
static void OnKeyUp(byte _key, int x, int y)
{
}
static void OnKeySpecialDown(int _key, int x, int y)
{
	switch(_key)
	{
	case GLUT_KEY_LEFT:		game->Command('l');		break;
	case GLUT_KEY_RIGHT:	game->Command('r');		break;
	case GLUT_KEY_UP:		game->Command('u');		break;
	case GLUT_KEY_DOWN:		game->Command('d');		break;
	}
}
static void OnKeySpecialUp(int _key, int x, int y)
{
}

static void Done()
{
	delete game;
}

int main(int argc, char* argv[])
{
//	xIo::ResourcePath(argv[0]);
//	xLog::Open();
	game = new eGame;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(320, 240);
	glutInitWindowPosition(100, 100);
	window = glutCreateWindow("Spartak Chess (Stockfish)");
	glutDisplayFunc(OnDraw);
//	glutFullScreen();
	glutTimerFunc(15, OnIdle, 0);
	glutReshapeFunc(OnResizeWindow);
	glutIgnoreKeyRepeat(true);
	glutKeyboardFunc(OnKeyDown);
	glutKeyboardUpFunc(OnKeyUp);
	glutSpecialFunc(OnKeySpecialDown);
	glutSpecialUpFunc(OnKeySpecialUp);
	atexit(Done);
	glutMainLoop();
}

#endif//USE_GLUT
