#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<SDL.h>

#ifdef DREAMCAST
#include<kos.h>
#include<SDL_dreamcast.h>
#endif

#include "tvfilter/tvfilter.h"

#include "config.h"

extern SDL_Surface *screen;

extern int emulating;
extern int nScreenRefreshRate;
extern int draw_border, maybe_border;
extern unsigned screen_pitch, screen_width, screen_height;
extern unsigned cyclenext;
extern unsigned vid_adr_cycleyet;
extern unsigned char *vid_cycle;

void dcastaway(void);
void emergency_reset(void);
void events_init(void);
void do_events(void);
int savestate(int numstate);
int loadstate(int numstate);
int loadstate_vmu(int numstate);
int savestate_vmu(int numstate);
void video_fullscreen(void);
void video_change_to_low(void);
void video_change_to_med(void);
void video_change_to_menu(void);
void change_redraw_to_menu(void);
void savestate_init(void);
void show_icon(void);
void reset_frameskip(void);
void print_frameskip(void);

#ifndef NO_RENDER
void Redraw(int row, int vid_adr);
void Redraw_med(int row, int vid_adr);
void Draw_border(int e);
void drawDisk(void);
void drawDiskEmpty(void);
void render_init(void);
void render_status(void);
void set_message(char *msg, int t);
void render_blank_screen(void);
void render_up_screen(void);
void render_down_screen(void);
void render_force_background(void);
void render_no_border(void);
void render_vkbd_background(void);
#else
#define Redraw(row, vid_adr)
#define Redraw_med(row, vid_adr)
#define Draw_border(e)
#define drawDisk()
#define drawDiskEmpty()
#define render_init()
#define render_status()
#define set_message(MSG,T)
#define render_blank_screen()
#define render_up_screen()
#define render_down_screen()
#define render_force_background()
#define render_no_border()
#define render_vkbd_background()
#endif
