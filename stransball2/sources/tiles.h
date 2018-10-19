#ifndef __TILES_HEADER
#define __TILES_HEADER

class TILE {
public:
	TILE(SDL_Surface *src,int x,int y,int dx,int dy);
	~TILE();

	void draw(int x,int y,SDL_Surface *surface);
	void draw_with_offset(int x,int y,SDL_Surface *surface,int offset);

	int get_sx(void) {return r.w;};
	int get_sy(void) {return r.h;};

private:
	SDL_Surface *orig;
	SDL_Rect r;
};

#endif
