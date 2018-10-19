#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <stdint.h>

void upscale2(uint32_t *to, uint32_t *from);
void upscale3(uint32_t *to, uint32_t *from);
void upscale4(uint32_t *to, uint32_t *from);

#endif /* _PLATFORM_H */
