

#include "../../types.h"

void upscale_320x240(uint32 *dst, uint8 *src); 
void upscale_384x240(uint32 *dst, uint8 *src);
void upscale_384x272(uint32 *dst, uint8 *src);
void upscale_480x272(uint32 *dst, uint8 *src);
void upscale_320x240_bilinearish_clip(uint32_t* dst, uint8 *src, int width);
void upscale_320x240_bilinearish_noclip(uint32_t* dst, uint8 *src, int width);