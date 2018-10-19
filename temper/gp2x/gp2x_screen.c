#include "gp2x_common.h"

void update_screen()
{
  gp2x_update_screen();
}

void set_screen_resolution(u32 width, u32 height)
{
  gp2x_initialize_screen();
}

void clear_screen()
{
  gp2x_clear_all_buffers();
}

void clear_line_edges(u32 line_number, u32 color, u32 edge, u32 middle)
{
  gp2x_clear_line_edges_all_buffers(line_number, color, edge, middle);
}

void *get_screen_ptr()
{
  return gp2x_get_screen_ptr();
}

u32 get_screen_pitch()
{
  return 320;
}

void set_single_buffer_mode()
{
  gp2x_set_single_buffer_mode();
}

void set_multi_buffer_mode()
{
  gp2x_set_multi_buffer_mode();
}

void clear_all_buffers()
{
  gp2x_clear_all_buffers();
}

