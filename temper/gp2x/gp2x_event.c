#include "gp2x_common.h"

// When you press a button on GP2X, which config option should it
// correspond to? That is, which in config.pad, the order that
// the GP2X buttons are given in the menu. This says what, for each
// bit.

u32 gp2x_to_config_map[] =
{
  GP2X_CONF_UP,           // 0
  GP2X_CONF_NONE,         // 1
  GP2X_CONF_LEFT,         // 2
  GP2X_CONF_NONE,         // 3
  GP2X_CONF_DOWN,         // 4
  GP2X_CONF_NONE,         // 5
  GP2X_CONF_RIGHT,        // 6
  GP2X_CONF_NONE,         // 7
  GP2X_CONF_START,        // 8
  GP2X_CONF_SELECT,       // 9
  GP2X_CONF_L,            // 10
  GP2X_CONF_R,            // 11
  GP2X_CONF_A,            // 12
  GP2X_CONF_B,            // 13
  GP2X_CONF_X,            // 14
  GP2X_CONF_Y,            // 15
  GP2X_CONF_NONE,         // 16
  GP2X_CONF_NONE,         // 17
  GP2X_CONF_NONE,         // 18
  GP2X_CONF_NONE,         // 19
  GP2X_CONF_NONE,         // 20
  GP2X_CONF_NONE,         // 21
  GP2X_CONF_VOL_DOWN,     // 22
  GP2X_CONF_VOL_UP,       // 23
  GP2X_CONF_VOL_MID,      // 24
  GP2X_CONF_NONE,         // 25
  GP2X_CONF_NONE,         // 26
  GP2X_CONF_STICK,        // 27
};

void update_input()
{
  u32 gp2x_buttons = gp2x_joystick_read();
  u32 button_status = 0;
  static u32 last_gp2x_buttons;

  u32 i;

  // vol up + vol down cancels both but presses the imaginary vol mid button
  if((gp2x_buttons & GP2X_VOL_UP) && (gp2x_buttons & GP2X_VOL_DOWN))
  {
    gp2x_buttons &= ~(GP2X_VOL_UP | GP2X_VOL_DOWN);
    gp2x_buttons |= GP2X_VOL_MID;
  }

  for(i = 0; i < 28; i++)
  {
    if(gp2x_buttons & (1 << i))
    {
      // Now perform the action and update the button status
      button_status |= button_action(gp2x_to_config_map[i],
       last_gp2x_buttons & (1 << i));
    }
  }

  last_gp2x_buttons = gp2x_buttons;

  if(button_status & IO_BUTTON_CLEAR)
    button_status = IO_BUTTON_NONE;

  io.button_status = (~button_status) & 0xFFF;
}

u32 platform_specific_button_action_direct(u32 button, u32 old_action)
{
  switch(button)
  {
    case CONFIG_BUTTON_VOLUME_DOWN:
      if(!old_action)
        gp2x_sound_volume(-4);

      break;

    case CONFIG_BUTTON_VOLUME_UP:
      if(!old_action)
        gp2x_sound_volume(4);

      break;
  }

  return 0;
}

u32 last_buttons = 0;
button_repeat_state_type button_repeat_state = BUTTON_NOT_HELD;

gui_action_type get_gui_input()
{
  gui_action_type new_button = CURSOR_NONE;
  u32 buttons = gp2x_joystick_read();
  u32 new_buttons;

  static u64 button_repeat_timestamp;
  static u32 button_repeat = 0;
  static gui_action_type cursor_repeat = CURSOR_NONE;

  delay_us(10000);

  new_buttons = (last_buttons ^ buttons) & buttons;
  last_buttons = buttons;

  if(new_buttons & GP2X_A)
    new_button = CURSOR_BACK;

  if(new_buttons & GP2X_X)
    new_button = CURSOR_EXIT;

  if(new_buttons & GP2X_B)
    new_button = CURSOR_SELECT;

  if(new_buttons & GP2X_UP)
    new_button = CURSOR_UP;

  if(new_buttons & GP2X_DOWN)
    new_button = CURSOR_DOWN;

  if(new_buttons & GP2X_LEFT)
    new_button = CURSOR_LEFT;

  if(new_buttons & GP2X_RIGHT)
    new_button = CURSOR_RIGHT;

  if(new_buttons & GP2X_L)
    new_button = CURSOR_PAGE_UP;

  if(new_buttons & GP2X_R)
    new_button = CURSOR_PAGE_DOWN;

  if(new_button != CURSOR_NONE)
  {
    get_ticks_us(&button_repeat_timestamp);
    button_repeat_state = BUTTON_HELD_INITIAL;
    button_repeat = new_buttons;
    cursor_repeat = new_button;
  }
  else
  {
    if(buttons & button_repeat)
    {
      u64 new_ticks;
      get_ticks_us(&new_ticks);

      if(button_repeat_state == BUTTON_HELD_INITIAL)
      {
        if((new_ticks - button_repeat_timestamp) >
         BUTTON_REPEAT_START)
        {
          new_button = cursor_repeat;
          button_repeat_timestamp = new_ticks;
          button_repeat_state = BUTTON_HELD_REPEAT;
        }
      }

      if(button_repeat_state == BUTTON_HELD_REPEAT)
      {
        if((new_ticks - button_repeat_timestamp) >
         BUTTON_REPEAT_CONTINUE)
        {
          new_button = cursor_repeat;
          button_repeat_timestamp = new_ticks;
        }
      }
    }
  }

  return new_button;
}

void clear_gui_actions()
{
  last_buttons = 0;
  button_repeat_state = BUTTON_NOT_HELD;
}

void initialize_event()
{
}
