// The way this works on PSP is a little different. There are 12
// PSP button config states (see psp_buttons_config_enum in event.h).
// This table converts these to a mask that shows which bit this
// corresponds with in the button status that the PSP system call
// returns. This way we can see if any of these are triggered.

u32 psp_mask_to_config[] =
{
  PSP_CTRL_UP,
  PSP_CTRL_DOWN,
  PSP_CTRL_LEFT,
  PSP_CTRL_RIGHT,
  PSP_CTRL_SQUARE,
  PSP_CTRL_CIRCLE,
  PSP_CTRL_CROSS,
  PSP_CTRL_TRIANGLE,
  PSP_CTRL_LTRIGGER,
  PSP_CTRL_RTRIGGER,
  PSP_CTRL_START,
  PSP_CTRL_SELECT,
};

void update_input()
{
  SceCtrlData ctrl_data;
  u32 psp_buttons;
  u32 button_status = 0;
  static u32 last_psp_buttons = 0;

  sceCtrlPeekBufferPositive(&ctrl_data, 1);
  psp_buttons = ctrl_data.Buttons;

  u32 i;

  for(i = 0; i < 12; i++)
  {
    // See if the particular action is being pressed
    if(psp_buttons & psp_mask_to_config[i])
    {
      // Now perform the action and update the button status
      button_status |= button_action(i, last_psp_buttons & (1 << i));
    }
  }

  last_psp_buttons = psp_buttons;

  if(button_status & IO_BUTTON_CLEAR)
    button_status = IO_BUTTON_NONE;

  io.button_status = (~button_status) & 0xFFF;
}

u32 platform_specific_button_action_direct(u32 button, u32 old_action)
{
  return 0;
}

gui_action_type get_gui_input()
{
  SceCtrlData ctrl_data;
  gui_action_type new_button = CURSOR_NONE;
  u32 buttons;
  u32 new_buttons;

  static u32 last_buttons = 0;
  static button_repeat_state_type button_repeat_state = BUTTON_NOT_HELD;

  static u64 button_repeat_timestamp;
  static u32 button_repeat = 0;
  static gui_action_type cursor_repeat = CURSOR_NONE;

  sceCtrlPeekBufferPositive(&ctrl_data, 1);
  buttons = ctrl_data.Buttons;

  delay_us(10000);

  new_buttons = (last_buttons ^ buttons) & buttons;
  last_buttons = buttons;

  if(new_buttons & PSP_CTRL_SQUARE)
    new_button = CURSOR_BACK;

  if(new_buttons & PSP_CTRL_CROSS)
    new_button = CURSOR_EXIT;

  if(new_buttons & PSP_CTRL_CIRCLE)
    new_button = CURSOR_SELECT;

  if(new_buttons & PSP_CTRL_UP)
    new_button = CURSOR_UP;

  if(new_buttons & PSP_CTRL_DOWN)
    new_button = CURSOR_DOWN;

  if(new_buttons & PSP_CTRL_LEFT)
    new_button = CURSOR_LEFT;

  if(new_buttons & PSP_CTRL_RIGHT)
    new_button = CURSOR_RIGHT;

  if(new_buttons & PSP_CTRL_LTRIGGER)
    new_button = CURSOR_PAGE_UP;

  if(new_buttons & PSP_CTRL_RTRIGGER)
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

void initialize_event()
{
}

