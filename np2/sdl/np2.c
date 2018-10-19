#include "compiler.h"
// #include <sys/time.h>
// #include <signal.h>
// #include <unistd.h>
#include "strres.h"
#include "np2.h"
#include "dosio.h"
#include "commng.h"
#include "fontmng.h"
#include "inputmng.h"
#include "scrnmng.h"
#include "soundmng.h"
#include "sysmng.h"
#include "taskmng.h"
#include "ini.h"
#include "pccore.h"
#include "iocore.h"
#include "scrndraw.h"
#include "s98.h"
#include "diskdrv.h"
#include "timing.h"
#include "keystat.h"
#include "vramhdl.h"
#include "menubase.h"
#include "mousemng.h"
#include "sysmenu.h"
#include "pg.h"

#ifdef PSPDBG
#include "psplib.h"
#endif


NP2OSCFG np2oscfg = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT framecnt;
static UINT waitcnt;
static UINT framemax = 1;
static char datadir[MAX_PATH];
static int exit_sem;
char clock_buf[64];

// ---- resume

static void getstatfilename(char *path, const char *ext, int size) {

	file_cpyname(path, datadir, size);
	file_cutext(path);
	file_catname(path, "np2sdl.", size);
	file_catname(path, ext, size);
}

static int flagsave(const char *ext) {

	int		ret;
	char	path[MAX_PATH];

	getstatfilename(path, ext, sizeof(path));
	ret = statsave_save(path);
	if (ret) {
		file_delete(path);
	}
	return(ret);
}

static void flagdelete(const char *ext) {

	char	path[MAX_PATH];

	getstatfilename(path, ext, sizeof(path));
	file_delete(path);
}

static int flagload(const char *ext, const char *title, BOOL force) {

	int		ret;
	int		id;
	char	path[MAX_PATH];
	char	buf[1024];
	char	buf2[1024 + 256];

	getstatfilename(path, ext, sizeof(path));
	id = DID_YES;
	ret = statsave_check(path, buf, sizeof(buf));
	if (ret & (~STATFLAG_DISKCHG)) {
		menumbox("Couldn't restart", title, MBOX_OK | MBOX_ICONSTOP);
		id = DID_NO;
	}
	else if ((!force) && (ret & STATFLAG_DISKCHG)) {
		SPRINTF(buf2, "Conflict!\n\n%s\nContinue?", buf);
		id = menumbox(buf2, title, MBOX_YESNOCAN | MBOX_ICONQUESTION);
	}
	if (id == DID_YES) {
		statsave_load(path);
	}
	return(id);
}


// ---- proc

static void framereset(UINT cnt) {
    framecnt = 0;
#ifdef PSPDBG
    if (sysmng_workclockrenewal()) {
        sysmng_updatecaption();
    }
#endif
}

static void processwait(UINT cnt) {

	if (timing_getcount() >= cnt) {
		timing_setcount(0);
		framereset(cnt);
	}
	else {
		taskmng_sleep(1);
	}
}

int pspeDebugWrite(const char* str,size_t length);

static void exit_proc(void)
{
    //終了処理

    taskmng_exit();

    pccore_cfgupdate();
    if (np2oscfg.resume) {
        flagsave(str_sav);
    }
    else {
        flagdelete(str_sav);
    }

    pccore_term();
    S98_trash();
    soundmng_deinitialize();

    if (sys_updates & (SYS_UPDATECFG | SYS_UPDATEOSCFG)) {
        initsave();
    }
	scrnmng_destroy();
}


// ms0:/psp/game/hoge/eboot.pbp → ms0:/psp/game/hoge/ に変換
static void set_datadir(char *s)
{
    char *p = '\0', *d;

    for (d = datadir; *s != '\0'; s++) {
        *d++ = *s;
        if (*s == '/') {
            p = d;
        }
    }
    *p = '\0';
	
	if (datadir[0]!='/')
	{
		file_get_execdir(datadir);
	}
}

int main(int argc, char *argv[])
{
    int id;

    set_datadir(argv[0]);
	

    pgGeInit();
    //    scrnmng_gu_init0();
    pgScreenFrame(2, 0);

    /* カレントファイル操作 */
    file_setcd(datadir);
    /* np2.cfgファイルの読み込み? */
    initload();

    scrnmng_change_scrn(np2oscfg.pspscrn);

    TRACEINIT();

    if (fontmng_init() != SUCCESS) {
        goto np2main_err2;
    }
    keystat_initialize();

    if (sysmenu_create() != SUCCESS) {
        goto np2main_err3;
    }

    scrnmng_gu_init();

    scrnmng_initialize();
    if (scrnmng_create(FULLSCREEN_WIDTH, FULLSCREEN_HEIGHT) != SUCCESS) {
        goto np2main_err4;
    }

    soundmng_initialize();
    commng_initialize();
    sysmng_initialize();
    taskmng_initialize();
    mousemng_initialize();

    pccore_init();
    S98_init();

    scrndraw_redraw();

    pccore_reset();

    if (np2oscfg.resume) {
        id = flagload(str_sav, str_resume, FALSE);
        if (id == DID_CANCEL) {
            goto np2main_err5;
        }
    }

#ifdef PSPDBG
    sysmng_workclockreset();
#endif

    while(taskmng_isavail()) {
        taskmng_rol();
        if (menuvram) {
            // menuを出している時は、emulationを停止する
            continue;
        }
        if (np2oscfg.NOWAIT) {
            pccore_exec(framecnt == 0);
            if (np2oscfg.DRAW_SKIP) { // nowait frame skip
                framecnt++;
                if (framecnt >= np2oscfg.DRAW_SKIP) {
                    processwait(0);
                }
            }
            else { // nowait auto skip
                framecnt = 1;
                if (timing_getcount()) {
                    processwait(0);
                }
            }
        }
        else if (np2oscfg.DRAW_SKIP) { // frame skip
            if (framecnt < np2oscfg.DRAW_SKIP) {
                pccore_exec(framecnt == 0);
                framecnt++;
            }
            else {
                processwait(np2oscfg.DRAW_SKIP);
            }
        }
        else { // auto skip
            if (!waitcnt) {
                UINT cnt;
                pccore_exec(framecnt == 0);
                framecnt++;
                cnt = timing_getcount();
                if (framecnt > cnt) {
                    waitcnt = framecnt;
                    if (framemax > 1) {
                        framemax--;
                    }
                }
                else if (framecnt >= framemax) {
                    if (framemax < 12) {
                        framemax++;
                    }
                    if (cnt >= 12) {
                        timing_reset();
                    }
                    else {
                        timing_setcount(cnt - framecnt);
                    }
                    framereset(0);
                }
            }
            else {
                processwait(waitcnt);
                waitcnt = framecnt;
            }
        }
    }
	exit_proc();
	return (SUCCESS);



 np2main_err5:
    pccore_term();
    S98_trash();
    soundmng_deinitialize();

 np2main_err4:
    scrnmng_destroy();

 np2main_err3:
    sysmenu_destroy();

 np2main_err2:
    TRACETERM();

    // np2main_err1:
    return(FAILURE);
}

