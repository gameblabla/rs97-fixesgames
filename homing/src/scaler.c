#include "scaler.h"

#include <stdint.h>
#include "video.h"

/* Upscale 2x from 16-bit resolution */
void upscale2(uint32_t *to, uint32_t *from)
{
	uint32_t tmp, tmp2;
	unsigned int i, j;
	int scaleWidth = SCREEN_W * 2;

	for (j = 0; j < SCREEN_H; ++j)
	{
		for (i = 0; i < SCREEN_W/2; ++i)
		{
			tmp = *from++;

			tmp2 = (tmp & 0xffff) | (tmp << 16);
			*(to + scaleWidth/2) = tmp2;
			*to++ = tmp2;

			tmp2 = (tmp & 0xffff0000) | (tmp >> 16);
			*(to + scaleWidth/2) = tmp2;
			*to++ = tmp2;
		}
		to += scaleWidth/2;
	}
}

