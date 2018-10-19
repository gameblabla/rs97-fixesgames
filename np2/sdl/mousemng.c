#include	"compiler.h"
#include	"mousemng.h"

typedef struct {
    SINT16  x;
    SINT16  y;
    UINT8   btn;
//  UINT    flag;
} MOUSEMNG;

static MOUSEMNG mousemng;

UINT8 mousemng_getstat(SINT16 *x, SINT16 *y, int clear)
{
    *x = mousemng.x;
    *y = mousemng.y;
    if (clear) {
        mousemng.x = 0;
        mousemng.y = 0;
    }
    return mousemng.btn;
}

void mousemng_initialize(void)
{
    ZeroMemory(&mousemng, sizeof(mousemng));
    mousemng.btn = uPD8255A_LEFTBIT | uPD8255A_RIGHTBIT;
}

void mousemng_update(int x, int y, int lb, int rb)
{
    mousemng.x = x;
    mousemng.y = y;

    if (lb) {
        mousemng.btn &= ~(uPD8255A_LEFTBIT);
    } else {
        mousemng.btn |= uPD8255A_LEFTBIT;
    }
    if (rb) {
        mousemng.btn &= ~(uPD8255A_RIGHTBIT);
    } else {
        mousemng.btn |= uPD8255A_RIGHTBIT;
    }
}
