#ifndef WIZ_COMMON_H
#define WIZ_COMMON_H

#define printf(format, ...)                                                 \
  fprintf(stderr, format, ##__VA_ARGS__)                                    \

#define vprintf(format, ap)                                                 \
  vfprintf(stderr, format, ap)                                              \

void wiz_initialize();
u32 wiz_joystick_read();
void wiz_sound_volume(s32 volume_change);

void wiz_initialize_screen();
void wiz_clear_all_buffers();
void wiz_clear_line_edges_all_buffers(u32 line_number,
 u32 color, u32 edge, u32 middle);

u16 *wiz_get_screen_ptr();
u32 wiz_get_screen_pitch();

void wiz_update_screen();
void wiz_set_single_buffer_mode();
void wiz_set_multi_buffer_mode();
void wiz_set_clock_speed(u32 clock_speed);

void wiz_quit();

#endif
