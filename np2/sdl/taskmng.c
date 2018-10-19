#include	"compiler.h"
#include "np2.h"
#include	"inputmng.h"
#include	"taskmng.h"
#include	"vramhdl.h"
#include	"menubase.h"
#include	"sysmenu.h"
#include "pg.h"
#include "psplib.h"
#include "scrnmng.h"
#include "keyconf.h"
#include "mousemng.h"
#include "keystat.h"
#include "soundmng.h"

#include "keypad.h"

BOOL task_avail;

volatile static int onscrn_cnt = 0;
volatile static int kc_scrn_cnt = 0;
static char onscrn_buf[5][25];
static char kc_scrn_buf[96][27];
static int selected_keyset = 1;
static int keyset = 1;
static int mbutton_l, mbutton_r;

static void update_kc_scrn()
{
    char s[256];

    /* まだ設定ファイルを読んでいない、設定ファイルがない */
    if (kc_is_ready() == 0) {
        plPrint(0, 0, 0xffff, "no""psp_key.txt");
        return;
    }
	/* QVGA : w:0-64 h:0-24

    /* witdh:0-95 height:0-26 */
    sprintf(s, "setting-%d", keyset);
    plPrint(11, 2, 0xffff, s);
    sprintf(s, "(%s)", kc_get_comment(keyset));
    plPrint(11, 3, 0xffff, s);

    plPrint(8, 5, 0xffff, " (config key mode)");
    sprintf(s, "↑ : %s", kc_get_keystrs(keyset, kc_up));
    plPrint(8, 6, 0xffff, s);
    sprintf(s, "→ : %s", kc_get_keystrs(keyset, kc_right));
    plPrint(8, 7, 0xffff, s);
    sprintf(s, "← : %s", kc_get_keystrs(keyset, kc_left));
    plPrint(8, 8, 0xffff, s); 
    sprintf(s, "↓ : %s", kc_get_keystrs(keyset, kc_down));
    plPrint(8, 9, 0xffff, s);
    sprintf(s, "○ : %s", kc_get_keystrs(keyset, kc_circle));
    plPrint(8, 10, 0xffff, s);
    sprintf(s, "△ : %s", kc_get_keystrs(keyset, kc_triangle));
    plPrint(8, 11, 0xffff, s);
    sprintf(s, "× : %s", kc_get_keystrs(keyset, kc_cross));
    plPrint(8, 12, 0xffff, s);
    sprintf(s, "□ : %s", kc_get_keystrs(keyset, kc_square));
    plPrint(8, 13, 0xffff, s);

    plPrint(8, 15, 0xffff, " (pc98 mouse mode)");
    sprintf(s, "△ : %s", kc_get_keystrs(keyset, kc_mm_triangle));
    plPrint(8, 16, 0xffff, s);
    sprintf(s, "□ : %s", kc_get_keystrs(keyset, kc_mm_square));
    plPrint(8, 17, 0xffff, s);

    plPrint(8, 20, 0xffff, "←→ : キーコンフィグ変更   ○ : キーコンフィグ決定");
}

void taskmng_initialize(void)
{
    int i;

    task_avail = TRUE;

    onscrn_cnt = kc_scrn_cnt = 0;

    for (i = 0; i < 5; i++) {
        onscrn_buf[i][0] = '\0';
    }

    for (i = 0; i < 20; i++) {
        kc_scrn_buf[i][0] = '\0';
    }

    kc_loadfile();

    taskmng_set_psp_clock(np2oscfg.pspturbo);
    taskmng_set_psp_mbutton(np2oscfg.pspmbutton);
}

void taskmng_exit(void)
{
    task_avail = FALSE;
}

void taskmng_prt_onscrn(void)
{
    int i;

    if (kc_scrn_cnt > 0)
	{
        update_kc_scrn();
        return;
    }

    if (onscrn_cnt > 0)
	{
        for (i = 0; i < 5; i++)
		{
            plPrint(20, i + 7 , 0xffff, onscrn_buf[i]);
        }
    }
}

void static set_onscrn(int i, char *s)
{
    onscrn_cnt = 120;
    strcpy(&onscrn_buf[i][0], s);
}

void taskmng_set_psp_clock(BOOL turbo)
{
}

void taskmng_set_psp_mbutton(BOOL mbutton)
{
    if (mbutton) {
        mbutton_l = PAD_BUTTON2;
        mbutton_r = PAD_BUTTON1;
        set_onscrn(2, "98mouse button: swap");
    } else {
        mbutton_l = PAD_BUTTON1;
        mbutton_r = PAD_BUTTON2;
        set_onscrn(2, "98mouse button: normal");
    }
}

static void mouse_98mode(unsigned button)
{
    static int x = 0, y = 0;
//     int ax, ay;

    if (button & PAD_LEFT) {
        x--;
    }
    if (button & PAD_RIGHT) {
        x++;
    }
    if (button & PAD_UP) {
        y--;
    }
    if (button & PAD_DOWN) {
        y++;
    }

    // x, y方向それぞれニュートラルなら0に戻す
    if (!(button & (PAD_LEFT | PAD_RIGHT))) {
        x = 0;
    }
    if (!(button & (PAD_UP | PAD_DOWN))) {
        y = 0;
    }

    // ある程度加速がついたら一定速度にする
    if (x > 5) {
        x = 5;
    }
    if (x < -5) {
        x = -5;
    }
    if (y > 5) {
        y = 5;
    }
    if (y < -5) {
        y = -5;
    }

    mousemng_update(x, y, button & mbutton_l, button & mbutton_r);
}

typedef struct _psp_key {
    UINT32 button;
    int flag; // 1:repeat (現在未使用)
    int idx;
    UINT8 keycode[2];
} psp_key_t;

static psp_key_t psp_key[] = {
    {PAD_UP, 0, 0, {kc_up, kc_up}},
    {PAD_LEFT, 0, 0, {kc_left, kc_left}},
    {PAD_RIGHT, 0, 0, {kc_right, kc_right}},
    {PAD_DOWN, 0, 0, {kc_down, kc_down}},
    {PAD_BUTTON1, 0, 0, {kc_circle, kc_circle}},
    {PAD_BUTTON2, 0, 0, {kc_cross, kc_cross}},
    {PAD_BUTTON4, 0, 0, {kc_triangle, kc_mm_triangle}},
    {PAD_BUTTON3, 0, 0, {kc_square, kc_mm_square}},
    {0, 0, 0, {0, 0}}
};

static BOOL mouse_98key_mode(unsigned button,
                             UINT32 beingdown, UINT32 beingup)
{
    int i, l;
    UINT32 type;
    BOOL moving = FALSE;
    char *p;

    i = 0;
    while ((type = psp_key[i].button) != 0) { // typeがNULLでない間
        p = kc_get_keycodes(selected_keyset, psp_key[i].keycode[0]);
        if (p == NULL || *(unsigned char *)p == 0xff) {
            i++;
            continue;
        }

        for (l = 0; p[l] != (char)0xff; l++)
            ;

        if (l == 1) {
            /* flagが1ならボタンが押されている間はsenddata()する(repeat mode)。
               flagが0ならボタンが押された最初の一回のみsenddata()する。*/
            if ((psp_key[i].flag == 1)?
                button & type : beingdown & type) {
                keystat_senddata(*p);
            }
            if (beingup & type) {
                keystat_senddata(*p | 0x80);
            }
        } else { // 同時押し「SHIFT A」で「a」とか
            while (*p != (char)0xff) {
                if (beingdown & type) {
                    keystat_senddata(*p);
                }
                if (beingup & type) {
                    keystat_senddata(*p | 0x80);
                }
                p++;
            }
        }
        i++;
    }

    // PSP画面におさまらないPC98画面のスクロール処理
    // moving |= scrnmng_set_scrn_pos(pad->Lx, pad->Ly);

    return moving;
}

//menuはpspの実画面(480x272)に描画する
static BOOL mouse_digipad(short *x, short *y, unsigned button)
{
    BOOL moving = FALSE;

    if (button & PAD_LEFT) {
        if (*x > 0) {
            (*x)--;
            moving = TRUE;
        }
    }
    if (button & PAD_RIGHT) {
        if (*x < (MAINSCR_W-1)) {
            (*x)++;
            moving = TRUE;
        }
    }
    if (button & PAD_UP) {
        if (*y > 0) {
            (*y)--;
            moving = TRUE;
        }
    }
    if (button & PAD_DOWN) {
        if (*y < (MAINSCR_H-1)) {
            (*y)++;
            moving = TRUE;
        }
    }

    return moving;
}

BOOL taskmng_mouse_anapad(short *x, short *y, UINT8 ax, UINT8 ay,
                          short maxx, short maxy)
{
    short oldx, oldy;

    oldx = *x, oldy = *y;

    if (ax > 255 / 2 + 32) {
        (*x)++;
        if (ax > 255 - 5) {
            (*x)++;
            if (ax == 255) {
                *x += 2;
            }
        }
        if (*x > maxx) {
            *x = maxx;
        }
    }
    if (ax < 255 / 2 - 32) {
        (*x)--;
        if (ax < 5) {
            (*x)--;
            if (ax == 0) {
                *x -= 2;
            }
        }
        if (*x < 0) {
            *x = 0;
        }
    }
    if (ay > 255 / 2 + 32) {
        (*y)++;
        if (ay > 255 - 5) {
            (*y)++;
            if (ay == 255) {
                *y += 2;
            }
        }
        if (*y > maxy) {
            *y = maxy;
        }
    }
    if (ay < 255 / 2 - 32) {
        (*y)--;
        if (ay < 5) {
            (*y)--;
            if (ay == 0) {
                *y -= 2;
            }
        }
        if (*y < 0) {
            *y = 0;
        }
    }

    return (oldx != *x || oldy != *y);
}

#define SKBD_TBL_ROW 6
#define SKBD_TBL_COL 22

UINT8 skbd_tbl[SKBD_TBL_ROW][SKBD_TBL_COL] = {
     //          COPY   f.1         f.2   f.3   f.4   f.5                     f.8         f.9  f.10
    {0x60, 0x60, 0x61, 0x62, 0x62, 0x63, 0x64, 0x65, 0x66, 0x66, 0x67, 0x68, 0x69, 0x69, 0x6a, 0x6b, 0x52, 0x53, 0x54, 0x54, 0x55, 0x56},
     //             1                                                           -           \  BS
    {0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x38, 0x39, 0x3e, 0x3f, 0x40, 0x41},
     //             q                                                                 [   RET   RET
    {0x0f, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1c, 0x36, 0x37, 0x42, 0x43, 0x44, 0x45},
     //             a     s     d     f                                               ]   RET   RET    ↑
    {0x74, 0x71, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x1c, 0x3a, 0x3a, 0x3a, 0x46, 0x47, 0x48, 0x49},
     //            29                                                              RSFT  RSFT  RSFT   ←    →    [1]
    {0x70, 0x70, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x7d, 0x7d, 0x7d, 0x3b, 0x3c, 0x4a, 0x4b, 0x4c, 0x4d},
     //          KANA                                                                                  ↓         [0]
    {0xff, 0xff, 0x72, 0x73, 0x51, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x35, 0x35, 0xff, 0xff, 0xff, 0x3d, 0x3d, 0x4e, 0x4f, 0x50, 0x1c}
};

static void get_skbd_key_pos(int row, int col, int *x, int *y, int *w, int *h)
{
    UINT8 tmp_tbl[21];
    int i, j;

    // キーコードでない0xffを飛ばしながら、col方向のダブりをなくす
    for (i = 0, j = 0; i < 22; i++) {
        if (skbd_tbl[row][i] != 0xff) {
            if (j == 0) {
                tmp_tbl[j++] = skbd_tbl[row][i];
            } else if (tmp_tbl[j - 1] != skbd_tbl[row][i]) {
                tmp_tbl[j++] = skbd_tbl[row][i];
            }
        }
    }

    // 左から何番目のキーかを算出(0 origin)
    for (i = 0; tmp_tbl[i] != skbd_tbl[row][col]; i++)
        ;

    switch (row) {
    case 0:
        *y = 2, *h = 14;
        if (i == 0) {
            *x = 2, *w = 14;
        } else if (i == 1) {
            *x = 19, *w = 14;
        } else if (i >= 2 && i <= 6) {
            *x = 40 + (17 * (i - 2)), *w = 17;
        } else if (i >= 7 && i <= 11) {
            *x = 130 + (17 * (i - 7)), *w = 17; 
        } else if (i >= 12) {
            *x = 220 + (17 * (i - 12)), *w = 17;
        }
        break;
    case 1:
        *y = 18, *h = 14;
        if (i == 0) {
            *x = 2, *w = 17;
        } else if (i >= 1 && i <= 13) {
            *x = 19 + (14 * (i - 1)), *w = 14;
        } else if (i == 14) { // BS
            *x = 201, *w = 17;
        } else if (i == 15 || i == 16) { // INS, DEL
            *x = 218 + (14 * (i - 15)), *w = 14;
        } else if (i >= 17) {
            *x = 250 + (14 * (i - 17)), *w = 14;
        }
        break;
    case 2:
        *y = 32, *h = 14;
        if (i == 0) { // TAB
            *x = 2, *w = 24;
        } else if (i >= 1 && i <= 12) {
            *x = 26 + (14 * (i - 1)), *w = 14;
        } else if (i == 13) { // RET
            *x = 199, *w = 19, *h = 28;
        } else if (i == 14 || i == 15) {
            *x = 218 + (14 * (i - 14)), *w = 14;
        } else if (i >= 16) {
            *x = 250 + (14 * (i - 16)), *w = 14;
        }
        break;
    case 3:
        *y = 46, *h = 14;
        if (i >= 0 && i <= 13) {
            *x = 2 + (14 * i), *w = 14;
        } else if (i == 14) { // RET
            *x = 199, *y = 32, *w = 19, *h = 28;
        } else if (i == 15) { // ↑
            *x = 218, *w = 29;
        } else if (i >= 16) {
            *x = 250 + (14 * (i - 16)), *w = 14;
        }
        break;
    case 4:
        *y = 60, *h = 14;
        if (i == 0) {
            *x = 2, *w = 34;
        } else if (i >= 1 && i <= 11) {
            *x = 36 + (14 * (i - 1)), *w = 14;
        } else if (i == 12) {
            *x = 190, *w = 28;
        } else if (i == 13 || i == 14) {
            *x = 218 + (14 * (i - 13)), *w = 14;
        } else if (i >= 15) {
            *x = 250 + (14 * (i - 15)), *w = 14;
        }
        break;
    case 5:
        *y = 74, *h = 14;
        if (i == 0 || i == 1) {
            *x = 29 + (14 * i), *w = 14;
        } else if (i == 2) {
            *x = 57, *w = 21;
        } else if (i == 3) {
            *x = 78, *w = 90;
        } else if (i == 4) {
            *x = 169, *w = 22;
        } else if (i == 5) {
            *x = 218, *w = 29;
        } else if (i >= 6) {
            *x = 250 + (14 * (i - 6)), *w = 14;            
        }
        break;
    }
}

/* deltar, deltacは-1, 0, +1のいずれか。どちらか片方は0 */
static BOOL skbd_key_move(int *row, int *col, int deltar, int deltac)
{
    int r, c;
    int x, y, w, h;

    /* 両方0ならキーの移動なし */
    if (deltar == 0 && deltac == 0) {
        return FALSE;
    }

    r = *row, c = *col;

    while (r < SKBD_TBL_ROW && c < SKBD_TBL_COL && r >= 0 && c >= 0
           && skbd_tbl[r][c] != 0xff) {

        if (skbd_tbl[*row][*col] != skbd_tbl[r][c]) {
            /* 今までの反転を元に戻す */
            get_skbd_key_pos(*row, *col, &x, &y, &w, &h);
            scrnmng_skbd_key_reverse(x, y, w, h);

            /* 新しいキーを反転する */
            *row = r, *col = c;
            get_skbd_key_pos(*row, *col, &x, &y, &w, &h);
            scrnmng_skbd_key_reverse(x, y, w, h);

            return TRUE;
        }
        r += deltar, c += deltac;
    }

    return FALSE;
}

#define SKBD_ROW_INIT 3
#define SKBD_COL_INIT 5
static int skbd_mode(UINT32 beingdown, UINT32 beingup, UINT32 button_down,
					int button)
{
    static int key_repeat;
    static int skbdrow = SKBD_ROW_INIT, skbdcol = SKBD_COL_INIT;
    int deltar, deltac;
    int redraw = 0;

    // ソフトキーボードは画面の外に完全に出ないようにする
    /*if (taskmng_mouse_anapad(&skbdx, &skbdy, 0,0,
                             479 - 4, 271 - 4)) {
        redraw = TRUE;
    }*/
	

    if (beingdown & PAD_BUTTON1) {
        keystat_senddata(skbd_tbl[skbdrow][skbdcol]);
    }
    if (beingup & PAD_BUTTON1) {
        keystat_senddata(skbd_tbl[skbdrow][skbdcol] | 0x80);
    }

    deltar = deltac = 0;
    if (beingdown &
        (PAD_LEFT | PAD_RIGHT | PAD_UP | PAD_DOWN)) {
        key_repeat = 20;
    }
    deltac = (beingdown & PAD_LEFT)? -1 : deltac;
    deltac = (beingdown & PAD_RIGHT)? +1 : deltac;
    deltar = (beingdown & PAD_UP)? -1 : deltar;
    deltar = (beingdown & PAD_DOWN)? +1 : deltar;

    if (button_down &
        (PAD_LEFT | PAD_RIGHT | PAD_UP | PAD_DOWN)) {
        key_repeat--;
        if (key_repeat < 0) {
            deltac = (button_down & PAD_LEFT)? -1 : deltac;
            deltac = (button_down & PAD_RIGHT)? +1 : deltac;
            deltar = (button_down & PAD_UP)? -1 : deltar;
            deltar = (button_down & PAD_DOWN)? +1 : deltar;
            key_repeat = 5;
        }
    }

    redraw |= skbd_key_move(&skbdrow, &skbdcol, deltar, deltac);

    return redraw;
}

static int pspmenu_mode(UINT32 beingdown, UINT32 beingup, unsigned button)
{
    static short x = 0, y = 0;
    BOOL moving = FALSE;
    int redraw = 0;
	

    // デジタルパッド

	moving |= mouse_digipad(&x, &y, button);

// アナログパッド
/*    moving |= taskmng_mouse_anapad(&x, &y,
                                   0,0,
                                   479, 271);
*/
	
    // PC98仮想画面(640x400)からPSP実画面(480x272)へ座標変換
    scrnmng_set_pspmxy(x, y);

    if (moving) {
        if (menuvram) {
            menubase_moving(x, y, 0);
        }
        redraw = TRUE;
    }

    if (beingdown & PAD_BUTTON1) {
        if (menuvram) {
            menubase_moving(x, y, 1);
            redraw = TRUE;
        }
    }
    if (beingup & PAD_BUTTON1) {
        if (menuvram) {
            menubase_moving(x, y, 2);
            redraw = TRUE;
        }
    }

    return redraw;
}

static char mouse_mode_str[4][25]
    = {"pspmenu", "PC98Mode: mouse", "PC98Mode: config key", "pc98skbd"};

void taskmng_rol(void)
{
    int redraw = 0;
	UINT32 sdl_button;
    UINT32 beingdown = 0, beingup = 0, button_changing;
    static UINT32 button_down = 0;
    int keyx, keyy, keyw, keyh;
    static BOOL first_call = TRUE;
	static int oldtick = 0;
	int newtick;
	
    static enum {
        pspmenu,
        pc98mouse,
        pc98key,
        pc98skbd,
        keyconf
    } mouse_mode = pc98mouse, old_mouse_mode = pc98mouse;

    if (first_call == TRUE) {
        set_onscrn(1, mouse_mode_str[mouse_mode]);

        get_skbd_key_pos(SKBD_ROW_INIT, SKBD_COL_INIT,
                         &keyx, &keyy, &keyw, &keyh);
        scrnmng_skbd_key_reverse(keyx, keyy, keyw, keyh);
        first_call = FALSE;
    }

    // リセット後、しばらくHELPキーを押す
    if (help_key_sending > 0) {
        keystat_senddata(0x3f);
        if (--help_key_sending == 0) {
            keystat_senddata(0x3f | 0x80);
        }
    }

	newtick = GETTICK();
	if (newtick < oldtick + 10)
		return;
	
	oldtick = newtick;
	
	
    if (onscrn_cnt > 0) {
        onscrn_cnt--;
        redraw = TRUE;
    } 

    // LTRIGGERを使っていなくてメニューが閉じられた場合の対処。
    // ちょっとかっこ悪い。
    if (mouse_mode == pspmenu && menuvram == NULL) {
        soundmng_play();
        mouse_mode = old_mouse_mode;
        set_onscrn(1, mouse_mode_str[mouse_mode]);
		redraw = TRUE;
    }
	
	sdl_button = pad_poll();
	if (pad_is_quit())
		taskmng_exit();

    // 前回と変化のあったbitを立てる
    button_changing = sdl_button ^ button_down;
    // 今回初めて押された = 前回と変化がある & 今ボタンが押されている
    beingdown = button_changing & sdl_button;
    // 今回初めてはなされた = 前回と変化がある & 前回押されていた
    beingup = button_changing & button_down;
    // 変化のあったbitを反転させる
    button_down ^= button_changing;

    // キーコンフィグモード
    if (beingdown & PAD_SELECT) {
        if (mouse_mode != keyconf) {
            // pc98モードに戻す(keycon -> pspmenu or pc98skbdには戻さない)
            if (mouse_mode == pc98mouse || mouse_mode == pc98key) {
                old_mouse_mode = mouse_mode;
            }
            mouse_mode = keyconf;
            onscrn_cnt = 0;
            kc_scrn_cnt = 1;
            keyset = selected_keyset;

            if (menuvram) {
                menubase_close();
                soundmng_play();
            }
            if (skbdx < MAINSCR_W) {
                skbdx += MAINSCR_W;
            }
        } else {
            mouse_mode = old_mouse_mode;
            kc_scrn_cnt = 0;
            set_onscrn(1, mouse_mode_str[mouse_mode]);
	}
	redraw = TRUE;
    }

    if (beingdown & PAD_L) {
        if (mouse_mode != pspmenu) {
            // pc98モードに戻す(pspmenu->pc98skbdには戻さない)
            if (mouse_mode == pc98mouse || mouse_mode == pc98key) {
                old_mouse_mode = mouse_mode;
            }
            mouse_mode = pspmenu;

            if (!menuvram) {
                soundmng_stop();
                sysmenu_menuopen(0, 0, 0);
            }
            if (skbdx < MAINSCR_W) {
                skbdx += MAINSCR_W;
            }
            kc_scrn_cnt = 0;
        } else {
            menubase_close();
            soundmng_play();
            mouse_mode = old_mouse_mode;
            set_onscrn(1, mouse_mode_str[mouse_mode]);
        }
        redraw = TRUE;
    }

    if (beingdown & PAD_R) {
        if (mouse_mode != pc98skbd) {
            if (mouse_mode == pc98mouse || mouse_mode == pc98key) {
                old_mouse_mode = mouse_mode;
            }
            mouse_mode = pc98skbd;

            if (skbdx >= MAINSCR_W) {
                skbdx -= MAINSCR_W;
				skbdy = MAINSCR_H-100;
            }
            if (menuvram) {
                menubase_close();
                soundmng_play();
            }
            kc_scrn_cnt = 0;
        } else {
			if (skbdy > 0)
			{
				skbdy = 0;
			}
			else
			{
				if (skbdx < MAINSCR_W)
					skbdx += MAINSCR_W;
				mouse_mode = old_mouse_mode;
				set_onscrn(1, mouse_mode_str[mouse_mode]);
			}
        }
        redraw = TRUE;
    }

    if (mouse_mode == pc98skbd) {
        redraw |= skbd_mode(beingdown, beingup, button_down, sdl_button);
        goto done; //ソフトキーボードモードならここで終了
    }

    if (mouse_mode == pspmenu) {
        redraw |= pspmenu_mode(beingdown, beingup, sdl_button);
        goto done;
    }

    if (mouse_mode == keyconf) {
        if (beingdown & PAD_LEFT) {
            if (--keyset == 0) {
                keyset = kc_get_ks_num();
            }
            redraw = TRUE;
        }
        if (beingdown & PAD_RIGHT) {
            if (++keyset == kc_get_ks_num() + 1) {
                keyset = 1;
            }
            redraw = TRUE;
        }
        if (beingdown & PAD_BUTTON1) {
            selected_keyset = keyset;
            mouse_mode = old_mouse_mode;
            kc_scrn_cnt = 0;
            set_onscrn(1, mouse_mode_str[mouse_mode]);
            redraw = TRUE;
	}
        goto done;
    }

    // ここまでくるのはmouse_modeがpc98keyまたはpc98mouseのとき。

    if (beingdown & PAD_START) {
        if (mouse_mode == pc98key) {
            mouse_mode = pc98mouse;
        } else {
            mouse_mode = pc98key;
        }
        set_onscrn(1, mouse_mode_str[mouse_mode]);
        redraw = TRUE;
    }

    // PC98マウスモード
    if (mouse_mode == pc98mouse) {
        mouse_98mode(sdl_button);
        goto done;
    }

    // PC98コンフィグキーモード
    if (mouse_mode == pc98key) {
        redraw |= mouse_98key_mode(sdl_button, beingdown, beingup);
    }

 done:
	
	if (menuvram || skbdx < MAINSCR_W || (kc_scrn_cnt | onscrn_cnt))
		scrnmng_direct(0);
	else
		scrnmng_direct(1);
	
    if (redraw)
	{
		if (kc_scrn_cnt | onscrn_cnt)
		{
			plClear();
			taskmng_prt_onscrn();
		}
		
		scrnmng_set_osd(kc_scrn_cnt | onscrn_cnt);
        scrnmng_gu_update();
    }
}

BOOL taskmng_sleep(UINT32 tick) {

	UINT32	base;

	base = GETTICK();
	while((task_avail) && ((GETTICK() - base) < tick)) {
		taskmng_rol();
		SDL_Delay(1);
#ifndef PSP
#if !defined(TARGET_MONA)
		Sleep(1);
#endif
#endif
	}
	return(task_avail);
}
