#include "iphone_common.h"

void update_screen()
{
  iphone_update_screen();
}

void set_screen_resolution(u32 width, u32 height)
{
  iphone_initialize_screen();
}

void clear_screen()
{
}

void clear_line_edges(u32 line_number, u32 color, u32 edge, u32 middle)
{
  iphone_clear_line_edges(line_number, color, edge, middle);
}

void *get_screen_ptr()
{
  return iphone_get_screen_ptr();
}

u32 get_screen_pitch()
{
  return 320;
}

void set_single_buffer_mode()
{
}

void set_multi_buffer_mode()
{
}

void clear_all_buffers()
{
}

