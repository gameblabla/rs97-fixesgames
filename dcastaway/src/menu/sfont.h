
/*
 * Based on SFONT by Karl Bartel
 * Changed by Chui
*/

#ifndef SFONT_H
#define SFONT_H

#include "SDL.h"

#ifdef __cplusplus 
extern "C" {
#endif

// Delcare one variable of this type for each font you are using.
// To load the fonts, load the font image into YourFont->Surface
// and call InitFont( YourFont );
typedef struct {
	SDL_Surface *Surface;	
	int CharPos[512];
	int h;
} SFont_FontInfo;

extern SFont_FontInfo SFont_InternalFont;

// Initializes the font
// Font: this contains the suface with the font.
//       The font must be loaded before using this function.
void SFont_InitFontInfo(SFont_FontInfo *Font);

static inline void SFont_InitFont(SDL_Surface *Font)
{
	SFont_InternalFont.Surface=Font;
	SFont_InitFontInfo(&SFont_InternalFont);
}


// Blits a string to a surface
// Destination: the suface you want to blit to
// text: a string containing the text you want to blit.
void SFont_PutStringInfo(SDL_Surface *Surface, SFont_FontInfo *Font, int x, int y, char *text);

static inline void SFont_PutString(SDL_Surface *Surface, int x, int y, char *text)
{
	SFont_PutStringInfo( Surface,&SFont_InternalFont,x,y,text);
}

// Returns the width of "text" in pixels
int SFont_TextWidthInfo(SFont_FontInfo *Font, char *text);

static inline int SFont_TextWidth(char *text)
{
	return SFont_TextWidthInfo(&SFont_InternalFont,text);
}

// Blits a string to with centered x position
static inline void SFont_XCenteredStringInfo(SDL_Surface *Surface, SFont_FontInfo *Font, int y, char *text)
{
	SFont_PutStringInfo(Surface, Font, (Surface->w-SFont_TextWidthInfo(Font,text))>>1, y, text);
}

static inline void SFont_XCenteredString(SDL_Surface *Surface, int y, char *text)
{
	SFont_XCenteredStringInfo(Surface,&SFont_InternalFont,y,text);
}

// Allows the user to enter text
// Width: What is the maximum width of the text (in pixels)
// text: This string contains the text which was entered by the user
void SFont_InputInfo( SDL_Surface *Destination, SFont_FontInfo *Font, int x, int y, int Width, char *text);

static inline void SFont_Input( SDL_Surface *Destination, int x, int y, int Width, char *text)
{
	SFont_InputInfo(Destination,&SFont_InternalFont,x,y,Width,text);
}

#ifdef __cplusplus
}
#endif

#endif /* SFONT_H */
