
typedef struct {
	UINT8	NOWAIT;
	UINT8	DRAW_SKIP;
	UINT8	F12KEY;
	UINT8	resume;
	UINT8	jastsnd;
    UINT8   pspturbo;
    UINT8   pspmbutton;
    UINT8   pspscrn;
} NP2OSCFG;


#if defined(SIZE_QVGA)
enum {
	FULLSCREEN_WIDTH	= 320,
	FULLSCREEN_HEIGHT	= 240
};
#else
enum {
	FULLSCREEN_WIDTH	= 640,
	FULLSCREEN_HEIGHT	= 400
};
#endif

typedef struct {
       void (*signal_func)(int id, void *arg);
       void *signal_arg;
       void (*finish_func)(int id, void *arg);
       void *finish_arg;
} GeCB;

extern	NP2OSCFG	np2oscfg;
extern int gecbid;
extern char clock_buf[64];
