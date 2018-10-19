#include "wiz_common.h"

void update_screen()
{
  wiz_update_screen();
}

void set_screen_resolution(u32 width, u32 height)
{
  wiz_initialize_screen();
}

void clear_screen()
{
  wiz_clear_all_buffers();
}

void clear_line_edges(u32 line_number, u32 color, u32 edge, u32 middle)
{
  wiz_clear_line_edges_all_buffers(line_number, color, edge, middle);
}

void *get_screen_ptr()
{
  return wiz_get_screen_ptr();
}

u32 get_screen_pitch()
{
#ifdef FRAMEBUFFER_PORTRAIT_ORIENTATION
  return 240;
#else
  return 320;
#endif
}

void set_single_buffer_mode()
{
  wiz_set_single_buffer_mode();
}

void set_multi_buffer_mode()
{
  wiz_set_multi_buffer_mode();
}

void clear_all_buffers()
{
  wiz_clear_all_buffers();
}


