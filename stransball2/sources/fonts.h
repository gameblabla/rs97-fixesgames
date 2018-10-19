#ifndef MOG_INIT
#define MOG_INIT

bool fonts_initialization(void);
void fonts_termination(void);

void font_print(int x,int y,char *text,SDL_Surface *surface);
void font_print_right(int x,int y,char *text,SDL_Surface *surface);
void font_print_centered(int x,int y,char *text,SDL_Surface *surface);

#endif

