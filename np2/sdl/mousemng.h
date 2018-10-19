
#ifdef __cplusplus
extern "C" {
#endif

enum {
        uPD8255A_LEFTBIT        = 0x80,
        uPD8255A_RIGHTBIT       = 0x20
};

UINT8 mousemng_getstat(SINT16 *x, SINT16 *y, int clear);
void mousemng_initialize(void);
void mousemng_update(int x, int y, int lb, int rb);

#ifdef __cplusplus
}
#endif

