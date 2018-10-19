#include        "compiler.h"
#include     <sys/time.h>
#include     <signal.h>
#include     <unistd.h>
#include        "scrnmng.h"
#include        "scrndraw.h"
#include        "vramhdl.h"
#include        "menubase.h"
#include        "softkbd.h"
#include "np2.h"
#include "pg.h"
#include "psplib.h"
#include "taskmng.h"

#include <SDL.h>

#define DRAWSURF g98VRAM

typedef struct {
    BOOL enable;
    int width;
    int height;
    int bpp;
    VRAMHDL vram;
} SCRNMNG;

typedef struct {
    int width;
    int height;
} SCRNSTAT;

static const char app_name[] = "Neko Project II";

static SCRNMNG scrnmng;
static SCRNSTAT scrnstat;
static SCRNSURF scrnsurf;

static CMNVRAM kbdvram;

typedef struct {
    int xalign;
    int yalign;
    int width;
    int height;
    int srcpos;
    int dstpos;
} DRAWRECT;


/*
  640*400をtextureに描画するが、texture bufferを1024*512にすると
  うまくいかないので512*512を2枚使う)

                           <--------- 512dot---------->
      vramtop   0x04000000 +--------------------------+
                           |                          |
                           |         実画面           |
                           |512*272*2*3               |
                           | 16bps*3画面 24pbs*2画面? |
      texture1  0x040cc000 +--------------------------+ A
                           |                          | |
                           |     512*400              |512dot
                           |- - - - - - - - - - - - - | |
                           |                          | V
      texture2  0x0414c000 +--------------------------+ A
                           |       :  308+4*90+4  :   | |
                           |128*400:- - - - - - - -   |512dot
                           | - - - -                  | |
                           |                          | V
                0x041cc000 +--------------------------+
*/

SDL_Surface *gMain;
SDL_Surface *gText;
SDL_Surface *g98VRAM;
SDL_Surface *gKeyboard;
SDL_Surface *ScreenSurface;

static int scrnmng_direct_flag = 0;
static int scrnmng_darty_flag = 0;
static int osd_time = 0;

int gecbid = -1;
static unsigned int *last_gc;
short skbdx = MAINSCR_W, skbdy = 0; //初期状態は画面外に置いておく
static int psp_scrn_mode = 0;
static short tx = 0, ty = 0;


static unsigned short mouse_cursor[] = {
    0xffff, 0xffff, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xffff, 0xffff, 0xffff, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0001, 0x0001, 0x0000,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0001, 0x0000,
    0xffff, 0xffff, 0xffff, 0xffff, 0x0001, 0x0001, 0x0001, 0x0000,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0001, 0x0000, 0x0000,
    0xffff, 0x0001, 0xffff, 0xffff, 0xffff, 0x0001, 0x0000, 0x0000,
    0x0001, 0x0001, 0xffff, 0xffff, 0xffff, 0x0001, 0x0000, 0x0000,
    0x0000, 0x0001, 0xffff, 0xffff, 0xffff, 0x0001, 0x0001, 0x0000,
    0x0000, 0x0001, 0x0001, 0xffff, 0xffff, 0xffff, 0x0001, 0x0000,
    0x0000, 0x0000, 0x0001, 0xffff, 0xffff, 0xffff, 0x0001, 0x0000,
    0x0000, 0x0000, 0x0001, 0xffff, 0xffff, 0xffff, 0x0001, 0x0001,
    0x0000, 0x0000, 0x0001, 0x0001, 0xffff, 0xffff, 0xffff, 0x0001,
    0x0000, 0x0000, 0x0000, 0x0001, 0xffff, 0xffff, 0xffff, 0x0001,
    0x0000, 0x0000, 0x0000, 0x0001, 0xffff, 0xffff, 0xffff, 0x0001,
    0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001
};

short pspmx = 0, pspmy = 0;
static int scr0o_f = 0;

static inline void PutPixel(SDL_Surface *ps,int x,int y,unsigned short color)
{
	if (x < 0 || x >= ps->w || y < 0 || y >= ps->h) 
		return;
	
	*(unsigned short *)(ps->pixels 
					   + (y * ps->pitch)
					   + (x * ps->format->BytesPerPixel)) = color;
}


// カーソル描画
static void draw_cursor(short x, short y)
{
    int i, j, idx;
	
	
	if (SDL_MUSTLOCK(gMain))
		SDL_LockSurface(gMain);
	//    vp = (unsigned short *)(pgGetVramAddr(x, y));

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 8; j++) {
            idx = i * 8 + j;
            /* 0なら描かない(透過させる) */
            if (mouse_cursor[idx] != 0x0) {
                PutPixel(gMain,x+j,y+i,mouse_cursor[idx]);
            }
        }
    }
	if (SDL_MUSTLOCK(gMain))
		SDL_UnlockSurface(gMain);

}


void scrnmng_draw_cursor()
{
    draw_cursor(pspmx, pspmy);
}


void scrnmng_change_scrn(UINT8 scrn_mode)
{
    psp_scrn_mode = (int)scrn_mode;

    if (psp_scrn_mode == 0) {
        // 外枠を表・裏面ともにクリアする
        scr0o_f = 2;
    }

    if (psp_scrn_mode == 2) {
        // (400 - 2y) * 0.75 = 272 ⇒ y = 18
        tx = 0, ty = 18; // 上下を18ドットずつはみ出るようにする
    } else {
        tx = 0, ty = 0;
    }
}

BOOL scrnmng_set_scrn_pos(short ax, short ay)
{
    short tx0, ty0, tmpx;

    tx0 = tx, ty0 = ty;

    // 画面からはみ出ない画面モードなら終了
    if (psp_scrn_mode == 0 || psp_scrn_mode == 1) {
        return FALSE;
    }

    if (psp_scrn_mode == 2) {
        // (400 - y) * 0.75 = 272 ⇒ y=38
        (void)taskmng_mouse_anapad(&tmpx, &ty, ax, ay, 0, 38);
    } else if (psp_scrn_mode == 3) {
        (void)taskmng_mouse_anapad(&tx, &ty, ax, ay, 160, 128);
    }

    return (tx != tx0 || ty != ty0);
}


void scrnmng_set_osd(int osd)
{
	osd_time = osd;
}


void scrnmng_gu_update(void)
{
	SDL_Rect rct;
	rct.w = rct.h = 0;
	
	if (scrnmng_direct_flag)
	{
		//SDL_UpdateRect(gMain,0,0,0,0);
		if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
		int x, y;
		uint32_t *s = (uint32_t*)gMain->pixels;
		uint32_t *d = (uint32_t*)ScreenSurface->pixels;
		/*for(y=0; y<240; y++){
			for(x=0; x<160; x++){
				*d++ = *s++;
			}
			d+= 160;
		}*/
				for(uint8_t y2 = 0; y2 < 240; y2++, s += 160, d += 320) 
			memmove(d, s, 1280); // double-line fix by pingflood, 2018
		if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
		SDL_Flip(ScreenSurface);
		return;
	}
	
	SDL_BlitSurface(g98VRAM,NULL,gMain,NULL);


	if (skbdx < MAINSCR_W)
	{
		rct.x = skbdx;
		rct.y = skbdy;

		SDL_BlitSurface(gKeyboard,NULL,gMain,&rct);
	}
	
	if (osd_time)
		SDL_BlitSurface(gText,NULL,gMain,NULL);

	
	if (menuvram)
		scrnmng_draw_cursor();
	
	//SDL_UpdateRect(gMain,0,0,0,0);
	{
		if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
		int x, y;
		uint32_t *s = (uint32_t*)gMain->pixels;
		uint32_t *d = (uint32_t*)ScreenSurface->pixels;
		for(y=0; y<240; y++){
			for(x=0; x<160; x++){
				*d++ = *s++;
			}
			d+= 160;
		}
		if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
		SDL_Flip(ScreenSurface);
	}

}

void scrnmng_gu_init()
{	
}

void scrnmng_set_pspmxy(short x, short y)
{
    pspmx = x, pspmy = y;
}

static BOOL calcdrawrect(DRAWRECT *dr, VRAMHDL s, const RECT_T *rt) {

	int		pos;

	dr->xalign = 2; // depth(16bpp) / 8
	dr->yalign = MAINSCR_W * 2; //次の行までのサイズ？横は実画面？
	dr->srcpos = 0;
	dr->dstpos = 0;
	dr->width = min(scrnmng.width, s->width);
	dr->height = min(scrnmng.height, s->height);
	if (rt) {
		pos = max(rt->left, 0);
		dr->srcpos += pos;
		dr->dstpos += pos * dr->xalign;
		dr->width = min(rt->right, dr->width) - pos;

		pos = max(rt->top, 0);
		dr->srcpos += pos * s->width;
		dr->dstpos += pos * dr->yalign;
		dr->height = min(rt->bottom, dr->height) - pos;
	}
	if ((dr->width <= 0) || (dr->height <= 0)) {
		return(FAILURE);
	}
	return(SUCCESS);
}


void scrnmng_initialize(void)
{
	// not frame
    scrnstat.width = 640;
    scrnstat.height = 400;
	
	// TODO : 失敗したらどうする？
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		printf("Failed to init SDL!＼n");
		return;
	}
	SDL_ShowCursor(SDL_DISABLE);

	//gMain = SDL_SetVideoMode(MAINSCR_W,MAINSCR_H,16,0); // for test
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_SWSURFACE);
	gMain = SDL_CreateRGBSurface(SDL_SWSURFACE, MAINSCR_W, MAINSCR_H, 16, 0, 0, 0, 0); // for test
	if (!gMain)
	{
		printf("can't alloc gMain＼n");
		return;
	}
	g98VRAM  = SDL_CreateRGBSurface(SDL_SWSURFACE,VRAM_W,VRAM_H,16,
									gMain->format->Rmask,
									gMain->format->Gmask,
									gMain->format->Bmask,
									gMain->format->Amask);
	
	if (!g98VRAM)
	{
		printf("can't alloc g98VRAM＼n");
		return;
	}
	
	gKeyboard = SDL_CreateRGBSurface(SDL_SWSURFACE,320,240,16,
									 gMain->format->Rmask,
									 gMain->format->Gmask,
									 gMain->format->Bmask,
									 gMain->format->Amask);

	if (!gKeyboard)
	{
		printf("can't alloc gKeyboard＼n");
		return;
	}
	
	
	gText = SDL_CreateRGBSurface(SDL_SWSURFACE,320,240,16,
									 gMain->format->Rmask,
									 gMain->format->Gmask,
									 gMain->format->Bmask,
									 gMain->format->Amask);
	
	if (!gText)
	{
		printf("can't alloc gText＼n");
		return;
	}
		
	Uint32 keycolor = 0x0001;
	
	SDL_FillRect(gKeyboard,NULL,keycolor);
	SDL_SetColorKey(gKeyboard,SDL_SRCCOLORKEY,keycolor);

	SDL_FillRect(gText,NULL,keycolor);
	SDL_SetColorKey(gText,SDL_SRCCOLORKEY,keycolor);
	
	plSetSurf(gText);
}

void scrnmng_skbd_key_reverse(int x, int y, int w, int h)
{
    short *dst;
    UINT8 *tmp;
    int i, j;

    dst = (short *)((UINT8 *)kbdvram.ptr +
                    x * kbdvram.xalign + y * kbdvram.yalign);

    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            *dst = ~*dst;
            dst++;
        }

#if 0
        (UINT8 *)dst -= (w * kbdvram.xalign);
        (UINT8 *)dst += kbdvram.yalign;
#else
        tmp = (UINT8 *)dst;
        tmp -= w * kbdvram.xalign;
        tmp += kbdvram.yalign;
        dst = (short *)tmp;
#endif
    }
}

static void palcnv(CMNPAL *dst, const RGB32 *src, UINT pals, UINT bpp)
{
    UINT i;

    if (bpp == 16) {
        for (i = 0; i < pals; i++) 
		{
			dst[i].pal16 = SDL_MapRGB(gMain->format,src[i].p.r,src[i].p.g,src[i].p.b);
//            dst[i].pal16 = ((src[i].p.r & 0xf8) << 8) |
//                ((src[i].p.g & 0xfc) << 3) | (src[i].p.b >> 3);
        }
    }
}

BOOL scrnmng_create(int width, int height)
{
    scrnmng.enable = TRUE;
    scrnmng.width = width;
    scrnmng.height = height;
    scrnmng.bpp = 16; // 16bpp for psp

    softkbd_initialize();
    kbdvram.width = 320;
    kbdvram.height = 240; 
    kbdvram.xalign = 2; // 2 = 16bpp
    kbdvram.yalign = 320 * 2; // W * 2
	
	kbdvram.ptr = (UINT8 *)gKeyboard->pixels;
    kbdvram.bpp = 16;
	
	if (SDL_MUSTLOCK(gKeyboard))
		SDL_LockSurface(gKeyboard);

    softkbd_paint(&kbdvram, palcnv, TRUE);
	
	if (SDL_MUSTLOCK(gKeyboard))
		SDL_UnlockSurface(gKeyboard);

    return(SUCCESS);

}

void scrnmng_destroy(void) 
{
	if (g98VRAM)
	{
		SDL_FreeSurface(g98VRAM);
		g98VRAM = NULL;
	}
	
	if (gKeyboard)
	{
		SDL_FreeSurface(gKeyboard);
		gKeyboard = NULL;
	}
	
	if (gText)
	{
		SDL_FreeSurface(gText);
		gText = NULL;
		plSetSurf(gText);
	}
	
	SDL_Quit();
	
	scrnmng.enable = FALSE;
}

RGB16 scrnmng_getpalfilter(void)
{
	RGB16 tmp = 0;
	if (gMain)
	{
		tmp = (gMain->format->Rmask<<1)  & gMain->format->Rmask;
		tmp |= (gMain->format->Gmask<<1) & gMain->format->Gmask;
		tmp |= (gMain->format->Bmask<<1) & gMain->format->Bmask;
	}
	return tmp;
}

RGB16 scrnmng_makepal16(RGB32 pal32) {
	RGB16	ret;
	
	if (gMain)
	{
		ret = SDL_MapRGB(gMain->format,pal32.p.r,pal32.p.g,pal32.p.b);
		return ret;
	}
	
	// GGGGGG000 RRRRR000 GGGGGG000
#ifdef PSP
	ret = (pal32.p.b & 0xf8) << 7;
	ret += (pal32.p.g & 0xf8) << 2;
	ret += pal32.p.r >> 3;
#else
	ret = (pal32.p.r & 0xf8) << 8;
#if defined(SIZE_QVGA)
	ret += (pal32.p.g & 0xfc) << (3 + 16);
#else
	ret += (pal32.p.g & 0xfc) << 3;
#endif
	ret += pal32.p.b >> 3;

#endif
	return(ret);
}

void scrnmng_setwidth(int posx, int width)
{
    scrnstat.width = width;
}

void scrnmng_setheight(int posy, int height)
{
    scrnstat.height = height;
}

const SCRNSURF *scrnmng_surflock(void)
{
	if (scrnmng.vram == NULL) 
	{
		SDL_Surface *ps;
		if (scrnmng_direct_flag)
		{
			ps = gMain;
			if (scrnmng_darty_flag)
			{
				SDL_BlitSurface(g98VRAM,NULL,gMain,NULL);
				scrnmng_darty_flag = 0;
			}
		}
		else
		{
			ps = g98VRAM;
			scrnmng_darty_flag = 1;
		}
		
		
		if (SDL_MUSTLOCK(ps))
			SDL_LockSurface(ps);
			
        scrnsurf.ptr = (UINT8 *)(ps->pixels);
        scrnsurf.xalign = 2; // depth(16bpp) / 8
        scrnsurf.yalign = VRAM_W * 2;
        scrnsurf.bpp = 16; // depth(16bpp)
   }
	else 
	{
        scrnsurf.ptr = scrnmng.vram->ptr;
        scrnsurf.xalign = scrnmng.vram->xalign;
        scrnsurf.yalign = scrnmng.vram->yalign;
        scrnsurf.bpp = scrnmng.vram->bpp;
    }
    scrnsurf.width = min(scrnstat.width, 640);
    scrnsurf.height = min(scrnstat.height, 400);
    scrnsurf.extend = 0;
    return(&scrnsurf);
}

void scrnmng_surfunlock(const SCRNSURF *surf)
{
	if (scrnmng.vram == NULL) 
	{
		if (scrnmng_direct_flag)
		{
			if (SDL_MUSTLOCK(gMain))
				SDL_UnlockSurface(gMain);
		}
		else 
		{
			if (SDL_MUSTLOCK(g98VRAM))
			SDL_UnlockSurface(g98VRAM);
		}
	}
	scrnmng_gu_update();
}

void scrnmng_direct(int flag)
{
	if (scrnmng_direct_flag == flag)
		return;

	if (scrnmng_direct_flag)
		SDL_BlitSurface(gMain,NULL,g98VRAM,NULL);
	else 
		SDL_BlitSurface(g98VRAM,NULL,gMain,NULL);

	
	scrnmng_direct_flag = flag;
}


// ----

BOOL scrnmng_entermenu(SCRNMENU *smenu)
{
    if (smenu == NULL) {
        goto smem_err;
    }
    vram_destroy(scrnmng.vram);
    scrnmng.vram
        = vram_create(scrnmng.width, scrnmng.height, FALSE, scrnmng.bpp);

    if (scrnmng.vram == NULL) {
        goto smem_err;
    }

    scrndraw_redraw();
#if 0
    smenu->width = scrnmng.width;
    smenu->height = scrnmng.height;
#else
    // メニュー画面はPSPサイズにする
    smenu->width = MAINSCR_W;
    smenu->height = MAINSCR_H;
#endif
    smenu->bpp = (scrnmng.bpp == 32)?24:scrnmng.bpp;
    return(SUCCESS);

 smem_err:
    return(FAILURE);
}

void scrnmng_leavemenu(void)
{
	VRAM_RELEASE(scrnmng.vram);
}

void scrnmng_menudraw(const RECT_T *rct) {

    DRAWRECT dr;
    const UINT8 *p;
    const UINT8 *q;
//    UINT8 *r;
    UINT8 *a;
    int salign;
    int dalign;
    int x;
	
	SDL_Surface *ps;
	
	if (scrnmng_direct_flag)
		ps = gMain;
	else 
		ps = g98VRAM;

	
    if ((!scrnmng.enable) && (menuvram == NULL)) {
        return;
    }
	if (!ps)
		return;
	
    if (calcdrawrect(&dr, menuvram, rct) == SUCCESS) {
		UINT8 *r;

		if (SDL_MUSTLOCK(ps))
		{
			SDL_LockSurface(ps);
		}
		
		
        p = scrnmng.vram->ptr + (dr.srcpos * 2);
        q = menuvram->ptr + (dr.srcpos * 2);
		r = ps->pixels + dr.dstpos;
        a = menuvram->alpha + dr.srcpos;
        salign = menuvram->width;
        dalign = dr.yalign - (dr.width * dr.xalign);
        do {
            x = 0;
            do {
                if (a[x]) {
                    if (!(a[x] & 2)) {
                        a[x] = 0;
                    }
                    if (a[x] & 2) {
                        // ダイアログ描画
                        *(UINT16 *)r = *(UINT16 *)(q + (x * 2));
                    } else {
                        // ダイアログ消去・移動時の背景(PC-98画面)描画
                        a[x] = 0;
                        *(UINT16 *)r = *(UINT16 *)(p + (x * 2));
                    }
                }
				r += 2; // x pitch
                // r += dr.xalign;
            } while(++x < dr.width);
            p += salign * 2;
            q += salign * 2;
			r += dalign;
            a += salign;
        } while(--dr.height);
		
		
		if (SDL_MUSTLOCK(ps))
			SDL_UnlockSurface(ps);
		
    }
}

void scrnmng_menudraw2(const RECT_T *rct) 
{
	scrnmng_menudraw(rct);

}
