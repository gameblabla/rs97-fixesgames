// gcc pbo_test.c `sdl-config --cflags --libs` -o pbo_test

#include "SDL.h"
#include "SDL_opengl.h"

typedef unsigned int u32;
typedef unsigned short int u16;

const u32 iterations = 500;

void fill_screen(void *_pixels, u32 pitch, u32 flip)
{
  u32 test = test;
  u32 x, y;
  u16 *pixels = _pixels;

  for(y = 0; y < 240; y++)
  {
    for(x = 0; x < 320; x++)
    {
      if((((x + flip) / 4) ^ ((y + flip) / 4)) & 0x1)
        pixels[x] = 0xFFFF;
      else
        pixels[x] = 0;
    }
    pixels += pitch;
  }
}

int main(int argc, char *argv[])
{
  SDL_Surface *screen;
  u32 use_opengl = 0;
  u32 ticks;
  u32 i, x, y;

  SDL_Init(SDL_INIT_VIDEO);

  if((argc > 1) && !strcmp(argv[1], "-ogl"))
    use_opengl = 1;

  if(use_opengl)
  {
   	GLuint pixel_buffer_objects[2];
    GLuint texture_handle;
    GLubyte shared_buffer[320 * 240 * 4];
    GLubyte *screen_pixels;

    printf("Using OpenGL pbuffers test.\n");

    screen = SDL_SetVideoMode(320, 240, 16, SDL_OPENGL);

    glShadeModel(GL_FLAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); 

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);

    glDisable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);

    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 240, 0,
     GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, (GLvoid *)shared_buffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenBuffers(2, pixel_buffer_objects);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pixel_buffer_objects[0]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, 320 * 240 * 2, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pixel_buffer_objects[1]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, 320 * 240 * 2, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    glOrtho(0, 1, 0, 1, -1, 1);

    u32 index = 0;
    u32 next_index = 0;
  
    for(i = 0; i < iterations; i++)
    { 
      index = (index + 1) % 2;
      next_index = (next_index + 1) % 2;

      glBindTexture(GL_TEXTURE_2D, texture_handle);
      glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pixel_buffer_objects[index]);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 320, 240,
       GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, 0);

      glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
       pixel_buffer_objects[next_index]);
      glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, 320 * 240 * 2, 0,
       GL_STREAM_DRAW_ARB);
      GLubyte* ptr =
       (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
  
      fill_screen(ptr, 320, index % 4);

      glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
      glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

      glBindTexture(GL_TEXTURE_2D, texture_handle);

      glBegin(GL_QUADS);
      glTexCoord2f(0, 0);
      glVertex3f(0, 0, 0);
      glTexCoord2f(1, 0.0);
      glVertex3f(1, 0, 0);
      glTexCoord2f(1, 1);
      glVertex3f(1, 1, 0);
      glTexCoord2f(0, 1);
      glVertex3f(0, 1, 0);
      glEnd();

      glBindTexture(GL_TEXTURE_2D, 0);

      SDL_GL_SwapBuffers();
    }
  }
  else
  {
    printf("Using pure SDL test.\n");

    screen = SDL_SetVideoMode(256, 384, 16, 0);
    printf("Got screen %p, pixels %p, pitch %d\n", screen, screen->pixels,
     screen->pitch);

    ticks = SDL_GetTicks();
    for(i = 0; i < iterations; i++)
    {
      fill_screen(screen->pixels, screen->pitch / 2, 0);
      SDL_Flip(screen);
    }
  }

  ticks = SDL_GetTicks() - ticks;

  printf("Took %d ms (%lf ms per frame, %lf fps)\n", ticks,
   (double)ticks / iterations, (1000.0 * iterations) / ticks);
}

