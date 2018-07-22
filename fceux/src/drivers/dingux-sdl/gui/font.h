#ifndef FONT_H
#define FONT_H

enum {
    SP_SELECTOR = 95,
    SP_ROM,
    SP_SETTINGS,
    SP_NOPREVIEW,
    SP_LEFTARROW,
    SP_RIGHTARROW,
    SP_UPARROW,
    SP_DOWNARROW,
    SP_MAIN_SETTINGS,
    SP_VIDEO_SETTINGS,
    SP_SOUND_SETTINGS,
    SP_BROWSER,
	SP_PREVIEWBLOCK,
	SP_LOGO
};


int InitFont();
void KillFont();
int DrawChar(SDL_Surface *dest, uint8 c, int x, int y);
void DrawText(SDL_Surface *dest, const char *textmsg, int x, int y);

#endif
