#ifndef IPHONE_H
#define IPHONE_H

#define printf(format, ...)                                                 \
  fprintf(stderr, format, ##__VA_ARGS__)                                    \

#define vprintf(format, ap)                                                 \
  vfprintf(stderr, format, ap)                                              \

#define mkdir(dir)                                                          \
  mkdir(dir, 0755)                                                          \

void iphone_initialize();
u32 iphone_joystick_read();
void iphone_sound_volume(s32 volume_change);

void iphone_initialize_screen();
void iphone_clear_all_buffers();
void iphone_clear_line_edges(u32 line_number, u32 color,
  u32 edge, u32 middle);

u16 *iphone_get_screen_ptr();
u32 iphone_get_screen_pitch();

void iphone_update_screen();
void iphone_set_single_buffer_mode();
void iphone_set_multi_buffer_mode();

void iphone_quit();

void updateScreen();
extern float __audioVolume;
extern unsigned long gp2x_pad_status;
extern unsigned short BaseAddress[320*240];
extern int soundBufferSize;
extern int soundInit;
extern unsigned long gp2x_pad_status;
extern int iphone_soundon;
extern int tArgc;
extern char** tArgv;
extern int __autosave;
extern unsigned long gp2x_fps_debug;
extern int iphone_soundon;
extern int __emulation_run;
extern int __emulation_saving;
extern int iphone_touches;
extern int iphone_layout;

#endif
