#ifndef GP2X_H
#define GP2X_H

#define printf(format, ...)                                                 \
  fprintf(stderr, format, ##__VA_ARGS__)                                    \

#define vprintf(format, ap)                                                 \
  vfprintf(stderr, format, ap)                                              \

#define mkdir(dir)                                                          \
  mkdir(dir, 0755)                                                          \

void gp2x_initialize();
u32 gp2x_joystick_read();
void gp2x_sound_volume(s32 volume_change);

void gp2x_initialize_screen();
void gp2x_clear_all_buffers();
void gp2x_clear_line_edges_all_buffers(u32 line_number,
 u32 color, u32 edge, u32 middle);

u16 *gp2x_get_screen_ptr();
u32 gp2x_get_screen_pitch();

void gp2x_update_screen();
void gp2x_set_single_buffer_mode();
void gp2x_set_multi_buffer_mode();

void gp2x_set_flck(u32 mhz);
void gp2x_set_ram_timings(u32 tRC, u32 tRAS, u32 tWR, u32 tMRD, u32 tRFC,
 u32 tRP, u32 tRCD);
void gp2x_default_ram_timings();
void gp2x_set_gamma(u32 gamma_value_percent);
void gp2x_quit();

#endif
