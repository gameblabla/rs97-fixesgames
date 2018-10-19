#ifndef _COLOURS_H_
#define _COLOURS_H_

extern int *colortable;

#define Palette_GetR(x) ((UBYTE) (colortable[x] >> 16))
#define Palette_GetG(x) ((UBYTE) (colortable[x] >> 8))
#define Palette_GetB(x) ((UBYTE) colortable[x])
#define Palette_GetY(x) (0.30 * Palette_GetR(x) + 0.59 * Palette_GetG(x) + 0.11 * Palette_GetB(x))
void Palette_SetRGB(int i, int r, int g, int b, int *colortable_ptr);

struct palette_settings {
	float brightness;
	float contrast;
	float saturation;
	float hue;
	float gamma;
	float saturation_ramp;
		// If > 0, higher broghtnesses have less saturation
	float colorburst_freq;
		// Frequency of the colorburst signal in MHz
	int color_delay;
		// Delay of the first color's phase in nanoseconds
	int color_diff;
		// Difference between phases of neighbour colors nanoseconds
};

extern struct palette_settings *color_settings;

int Palette_Read(const char *filename, int *colortable_ptr);
void Palette_Generate(int *colortable_ptr, struct palette_settings *settings);
void Palette_Regenerate();
void Palette_Adjust(int black, int white, int colintens, int *colortable_ptr);
void Palette_SetVideoSystem(int mode);
void Palette_InitialiseMachine(void);
void Palette_Initialise(int *argc, char *argv[]);

#endif /* _COLOURS_H_ */
