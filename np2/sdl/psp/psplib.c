// 自前のlibc関係他
#include "compiler.h"
#include "pg.h"
#include <stdarg.h>
#include <SDL.h>

static SDL_Surface *pTextSurf = NULL;

typedef struct _knj10_blank_id {
    int start;
    int end;
} knj10_bid_t;

typedef struct _ank10 {
    unsigned char bmp[10];
} ank10_t;

typedef struct _knj10 {
    unsigned short bmp[10];
} knj10_t;

#include "naga10font/n10font.c"

#ifdef PSP_TIME

// for localtime()
struct tm {
  int   tm_sec;
  int   tm_min;
  int   tm_hour;
  int   tm_mday;
  int   tm_mon;
  int   tm_year;
  int   tm_wday;
  int   tm_yday;
  int   tm_isdst;
};

long time(long *timer)
{
    long t;

	t = SDL_GetTicks() * 1000;

    if (timer != NULL) {
        *timer = t;
    }
    return t;
}

static int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 超適当localtime()
struct tm *localtime(const long *timer)
{
    struct tm *tobj;
    int i;
    int day, year, tmp_year, leapy_cnt, tmp_day;
    long t;

    //1970以前の場合はエラー
    if (*timer < 0) {
        return NULL;
    }

    tobj = malloc(sizeof(struct tm));
    if (tobj == NULL) {
        return NULL;
    }

    t = *timer + 9 * 60 * 60; // GMT->JST変換

    day = (int)(t / (60 * 60 * 24));
    tmp_year = day / 365;
    leapy_cnt = (tmp_year + 2) / 4;
    year = (day + leapy_cnt) / 365;

    // 1970 1971 1972 1973 1974 1975 1976
    //  365  365  366  365  365  365  366

    //  1  2  3  4  5  6  7  8  9 10 11 12
    // 31 28 31 30 31 30 31 31 30 31 30 31

    tobj->tm_sec = t % 60;
    tobj->tm_min = (t / 60) % 60;
    tobj->tm_hour = (t / (60 * 60)) % 24;
    tobj->tm_year = year + 70;
    tobj->tm_wday = (day + 4) % 7; // 曜日(日:0, 月:1, ...) 1970/1/1は木
    tobj->tm_yday = day - (365 * year) - leapy_cnt;
    tmp_day = tobj->tm_yday + 1;
    for (i = 0; i < 12; i++) {
        tmp_day -= days[i];
        if (tmp_day <= 0) {
            break;
        }
    }
    tobj->tm_mon = i;
    tobj->tm_mday = tmp_day + days[i];
    tobj->tm_isdst = 0;

    return tobj;
}
#endif

static char dbgBuf[80];

void dbgPrint(char *s)
{
    strcpy(dbgBuf, s);
}
char *pgGetDbgBuf(void)
{
    return dbgBuf;
}

/***** ナガ10フォント表示系 *****/

#define N10F_SWIDTH 5  //半角文字幅
#define N10F_WWIDTH 10 //全角文字幅
#define N10F_HEIGHT 10 //文字の高さ

#define PL_SCHAR 1
#define PL_WCHAR 2

static void *getAddress(int x,int y)
{
	return (pTextSurf->pixels + (y * pTextSurf->pitch) + (x * 2));
}

/* sjis→jisコード変換 */
static unsigned int sjis2jis(unsigned int w)
{
    unsigned char wh, wl;

    wh = w / 256, wl = w % 256;

    wh <<= 1;
    if (wl < 0x9f) {
        wh += (wh < 0x3f)? 0x1f : -0x61;
        wl -= (wl > 0x7e)? 0x20 : 0x1f;
    } else {
        wh += (wh < 0x3f)? 0x20 : -0x60;
        wl -= 0x7e;
    }

    return (wh * 256 + wl);
}

/* JISコードから0 originのindexに変換する */
static unsigned int jis2idx(unsigned int jc) {
    jc -= 0x2121;
    jc = jc % 256 + (jc / 256) * 0x5e;

    return jc;
}

/* 文字のbitデータを返却し、呼び元の文字列のポインタもずらす。
   戻り値: PL_SCHAR(半角文字), PL_WCHAR(全角文字) */
static int get_bmp(unsigned char **s, unsigned short *b)
{
    int i;
    unsigned int idx, blank;
    unsigned short bmp;

    // ank文字
    if ((**s & 0x80) == 0) {
        for (i = 0; i < N10F_HEIGHT; i++) {
            bmp = ank10[**s - 0x20].bmp[i];
            b[i] = bmp << 8;
        }
        (*s)++;

        return PL_SCHAR;
    }

    // 全角文字
    idx = jis2idx(sjis2jis(**s * 256 + *(*s + 1)));

    /* 空きコードを飛ばす */
    blank = 0;
    for (i = 0; knj10_bid[i].start != -1; i++) {
        if (knj10_bid[i].end > idx) {
            break;
        }
        blank += knj10_bid[i].end - knj10_bid[i].start - 1;
    }
    idx -= blank;

    for (i = 0; i < N10F_HEIGHT; i++) {
        b[i] = knj10[idx].bmp[i];
    }
    *s += 2; //全角なので次の文字は2byte先になる

    return PL_WCHAR;
}

/* 文字位置pに幅w, 文字データbmp, 色colorの一文字を表示 */
static void plPutChar(int x,int y, int w, unsigned short *bmp, int color,int shift)
{
    unsigned short l;
	unsigned short *p;
	
    int i, j;
	int h;
		
	x *= N10F_SWIDTH;
	y *= N10F_HEIGHT;
	
	x+= shift;
	y+= shift;
	
	p = (unsigned short *)getAddress(x, y);

	w = (x + w < pTextSurf->w ? w : pTextSurf->w - x);
	h = (y + N10F_HEIGHT < pTextSurf->h ? N10F_HEIGHT : pTextSurf->h - y);
	
    for (j = 0; j < h; j++) {
        l = bmp[j];
        for (i = 0; i < w; i++) {
            if (l & 0x8000 && x+i >= 0 && y+j >= 0)
                *p = color;
            p++;
            l <<= 1;
        }
        p += MAINSCR_W - w; //次のラインにずらす
    }
}

/* ナガ10フォント(半角5*10, 全角10*10)を表示する */
void plPrint(int x, int y, int color, char *s)
{
    unsigned short bmp[10], *p;
    unsigned char *us;
    int w;
	
	if (!pTextSurf)
		return;

    us = (unsigned char *)s;

    while (*us != '\0') {
        w = (get_bmp(&us, bmp) == PL_WCHAR)? N10F_WWIDTH : N10F_SWIDTH;
        plPutChar(x,y, w, bmp, 0,1);       
		plPutChar(x,y, w, bmp, 0,-1);

        plPutChar(x,y, w, bmp, color,0);
		if (w == N10F_SWIDTH)
			x++;
		else 
			x+=2;
		
    }
}

/* ナガ10フォント座標(x, y)からw文字分をクリアする */
void plClrChars(int x, int y, int w)
{
    unsigned short bmp[10], *p;
    int i;

	if (!pTextSurf)
		return;

    memset(bmp, 0xff, sizeof(bmp));

    for (i = 0; i < w; i++) {
        plPutChar(x,y, N10F_SWIDTH, bmp, 0x0001,0); // color=0で塗りつぶす
		x++;
    }
}

void plPrintErr(char *s)
{
    plClrChars(40, 15, 20);
    plPrint(40, 15, 0xffff, s);
}

void plClear(void)
{
	SDL_FillRect(pTextSurf,NULL,0x0001);
}

void plSetSurf(SDL_Surface *ps)
{
	pTextSurf = ps;
}

/***** ナガ10フォント表示系終わり *****/
