#ifndef MOG_AUXILIAR
#define MOG_AUXILIAR


#ifndef _WIN32
char *strupr(char *ptr);
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
// PPC values:
#define AMASK  0xff000000
#define BMASK  0x000000ff
#define GMASK  0x0000ff00
#define RMASK  0x00ff0000
#define AOFFSET 0
#define BOFFSET 3
#define GOFFSET 2
#define ROFFSET 1

#else
// Intel values:
#define AMASK  0xff000000
#define BMASK  0x000000ff
#define GMASK  0x0000ff00
#define RMASK  0x00ff0000
#define AOFFSET 3
#define BOFFSET 0
#define GOFFSET 1
#define ROFFSET 2

#endif

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
Uint32 getpixel(SDL_Surface *surface, int x, int y);
void rectangle(SDL_Surface *surface, int x, int y, int w, int h, Uint32 pixel);

void surface_fader(SDL_Surface *surface,float r_factor,float g_factor,float b_factor,float a_factor,SDL_Rect *r);

SDL_Surface *load_maskedimage(char *image,char *mask,char *path);


#endif


