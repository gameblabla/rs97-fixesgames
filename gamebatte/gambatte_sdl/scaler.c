
#include "scaler.h"


/* Ayla's fullscreen upscaler */
/* Upscale from 160x144 to 320x240 */
void fullscreen_upscale(uint32_t *to, uint32_t *from) {
    uint32_t reg1, reg2, reg3, reg4;
    unsigned int x,y;

    /* Little explanation:
     * we transform three lines of pixels into five lines.
     * Each line has twice the number of pixels.
     * So each turn, we read two pixels (2 * 16-bit pixel) from the upper line,
     * two from the middle line, and two from the bottom line.
     * Each pixel from those lines will be doubled and added to the first, third or fifth
     * line on the output.
     * The pixels composing lines two and four will be calculated as the average between
     * the pixels above them and the pixels under them.
     * Easy isn't it?
     */

    for (y=0; y < 240/5; y++) {
        for(x=0; x < 320/4; x++) {
            __builtin_prefetch(to+4, 1);

            reg2 = *from;

            // first pixel, upper line => reg1
            reg1 = reg2 & 0xffff0000;
            reg1 |= reg1 >> 16;
            *(to+1) = reg1;
            reg1 = (reg1 & 0xf7def7de) >> 1;

            // second pixel, upper line => reg2
            reg2 = reg2 & 0xffff;
            reg2 |= reg2 << 16;
            *to = reg2;
            reg2 = (reg2 & 0xf7def7de) >> 1;

            reg4 = *(from + 160/2);

            // first pixel, middle line => reg3
            reg3 = reg4 & 0xffff0000;
            reg3 |= reg3 >> 16;
            *(to + 2*320/2 +1) = reg3;
            reg3 = (reg3 & 0xf7def7de) >> 1;

            // second pixel, middle line => reg4
            reg4 = reg4 & 0xffff;
            reg4 |= reg4 << 16;
            *(to + 2*320/2) = reg4;
            reg4 = (reg4 & 0xf7def7de) >> 1;

            // We calculate the first pixel of the 2nd output line.
            *(to + 320/2 +1) = reg1 + reg3;

            // We calculate the second pixel of the 2nd output line.
            *(to + 320/2) = reg2 + reg4;

            reg2 = *(from++ + 2*160/2);

            // first pixel, bottom line => reg1
            reg1 = reg2 & 0xffff0000;
            reg1 |= reg1 >> 16;
            *(to + 4*320/2 +1) = reg1;
            reg1 = (reg1 & 0xf7def7de) >> 1;

            // second pixel, bottom line => reg2
            reg2 = reg2 & 0xffff;
            reg2 |= reg2 << 16;
            *(to + 4*320/2) = reg2;
            reg2 = (reg2 & 0xf7def7de) >> 1;

            // We calculate the two pixels of the 4th line.
            *(to++ + 3*320/2) = reg2 + reg4;
            *(to++ + 3*320/2) = reg1 + reg3;
        }
        to += 4*320/2;
        from += 2*160/2;
    }
}

/* Ayla's 1.5x Upscaler - 160x144 to 240x216 */
	/* Before:
	 *    a b c d
	 *    e f g h
	 *
	 * After (parenthesis = average):
	 *    a      (a,b)      b      c      (c,d)      d
	 *    (a,e)  (a,b,e,f)  (b,f)  (c,g)  (c,d,g,h)  (d,h)
	 *    e      (e,f)      f      g      (g,h)      h
	 */
void scale15x(uint32_t *to, uint32_t *from) {
	uint32_t reg1, reg2, reg3, reg4, reg5;
	int x, y;

	for (y=0; y<216/3; y++) {
		for (x=0; x<240/6; x++) {
			__builtin_prefetch(to+4, 1);

			// Read b-a
			reg1 = *from;
			reg5 = reg1 >> 16;
			reg2 = (reg1 & 0xf7de0000) >> 1;
			reg1 &= 0xffff;
			reg1 |= reg2 + ((reg1 & 0xf7de) << 15);

			// Write (a,b)-a
			*to = reg1;
			reg1 = (reg1 & 0xf7def7de) >> 1;

			// Read f-e
			reg3 = *(from++ + 160/2);
			reg2 = reg3 >> 16;
			reg4 = (reg3 & 0xf7de0000) >> 1;
			reg3 &= 0xffff;
			reg3 |= reg4 + ((reg3 & 0xf7de) << 15);

			// Write (e,f)-e
			*(to + 2*320/2) = reg3;
			reg3 = (reg3 & 0xf7def7de) >> 1;

			// Write (a,b,e,f)-(a,e)
			*(to++ + 320/2) = reg1 + reg3;
			
			// Read d-c
			reg1 = *from;

			// Write c-b
			reg5 |= (reg1 << 16);
			*to = reg5;
			reg5 = (reg5 & 0xf7def7de) >> 1;

			// Read h-g
			reg3 = *(from++ + 160/2);

			// Write g-f
			reg2 |= (reg3 << 16);
			*(to + 2*320/2) = reg2;
			reg2 = (reg2 & 0xf7def7de) >> 1;

			// Write (c,g)-(b,f)
			*(to++ + 320/2) = reg2 + reg5;

			// Write d-(c,d)
			reg2 = (reg1 & 0xf7def7de) >> 1;
			reg1 = (reg1 & 0xffff0000) | ((reg2 + (reg2 >> 16)) & 0xffff);
			*to = reg1;
			reg1 = (reg1 & 0xf7def7de) >> 1;

			// Write h-(g,h)
			reg2 = (reg3 & 0xf7def7de) >> 1;
			reg3 = (reg3 & 0xffff0000) | ((reg2 + (reg2 >> 16)) & 0xffff);
			*(to + 2*320/2) = reg3;
			reg3 = ((reg3 & 0xf7def7de) >> 1);

			// Write (d,h)-(c,d,g,h)
			*(to++ + 320/2) = reg1 + reg3;
		}

		to += 2*360/2;
		from += 160/2;
	}
}
