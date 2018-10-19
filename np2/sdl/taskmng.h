
#ifdef __cplusplus
extern "C" {
#endif

extern	BOOL	task_avail;
extern short skbdx, skbdy;
extern short pspmx, pspmy;

void taskmng_initialize(void);
void taskmng_exit(void);
void taskmng_prt_onscrn(void);
void taskmng_rol(void);
#define	taskmng_isavail()		(task_avail)
BOOL taskmng_sleep(UINT32 tick);
void taskmng_set_psp_clock(BOOL turbo);
void taskmng_set_psp_mbutton(BOOL mbutton);
BOOL taskmng_mouse_anapad(short *x, short *y, UINT8 ax, UINT8 ay,
                          short maxx, short maxy);

#ifdef __cplusplus
}
#endif

