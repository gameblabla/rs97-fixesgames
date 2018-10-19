#include "../common.h"
#include "pandora_event.h"

#include <linux/input.h>
#include <fcntl.h>

u32 device_handle_gpio_keys;
u32 device_handle_keypad;
u32 shift_mod = 0;

u32 gpio_keys_map(u32 key)
{
  switch(key)
  {
    case KEY_UP:
      return config.pad[PANDORA_BUTTON_UP];
    
    case KEY_DOWN:
      return config.pad[PANDORA_BUTTON_DOWN];

    case KEY_LEFT:
      return config.pad[PANDORA_BUTTON_LEFT];
 
    case KEY_RIGHT:
      return config.pad[PANDORA_BUTTON_RIGHT];

    case KEY_LEFTALT:
    case KEY_ENTER:
      return config.pad[PANDORA_BUTTON_START];

    case KEY_LEFTCTRL:
      return config.pad[PANDORA_BUTTON_SELECT];

    case KEY_PAGEUP:
      return config.pad[PANDORA_BUTTON_Y];

    case KEY_HOME:
      return config.pad[PANDORA_BUTTON_A];

    case KEY_PAGEDOWN:
      return config.pad[PANDORA_BUTTON_X];

    case KEY_END:
      return config.pad[PANDORA_BUTTON_B];

    case KEY_RIGHTSHIFT:
      return config.pad[PANDORA_BUTTON_L];

    case KEY_RIGHTCTRL:
      return config.pad[PANDORA_BUTTON_R];

    case KEY_KPPLUS:
      return config.pad[PANDORA_BUTTON_L2];

    case KEY_KPMINUS:
      return config.pad[PANDORA_BUTTON_R2];

    default:
      break;
  }

  return 0;
}

gui_action_type gpio_keys_gui_action_map(u32 key)
{
  switch(key)
  {
    case KEY_UP:
      return CURSOR_UP;
    
    case KEY_DOWN:
      return CURSOR_DOWN;

    case KEY_LEFT:
      return CURSOR_LEFT;
 
    case KEY_RIGHT:
      return CURSOR_RIGHT;

    case KEY_LEFTALT:
    case KEY_END:
      return CURSOR_SELECT;

    case KEY_HOME:
      return CURSOR_BACK;

    case KEY_PAGEDOWN:
      return CURSOR_EXIT;

    case KEY_RIGHTSHIFT:
      return CURSOR_PAGE_UP;

    case KEY_RIGHTCTRL:
      return CURSOR_PAGE_DOWN;
  }
  return CURSOR_NONE;
}

#define update_key_map(base_char, shift_char)                                  \
  if(shift_mod)                                                                \
    return shift_char;                                                         \
  else                                                                         \
    return base_char                                                           \

#define key_map_codes_case(name, base_char, shift_char)                        \
  case KEY_##name:                                                             \
    update_key_map(base_char, shift_char)                                      \

#define key_map_codes_case_fn(name, base_char)                                 \
  case KEY_##name:                                                             \
    return base_char                                                           \

u32 update_key_letter(u32 key_code, u32 shift_mod)
{
  switch(key_code)
  {
    key_map_codes_case(0, '0', ']');
    key_map_codes_case(1, '1', 0);
    key_map_codes_case(2, '2', '{');
    key_map_codes_case(3, '3', '}');
    key_map_codes_case(4, '4', '-');
    key_map_codes_case(5, '5', '%');
    key_map_codes_case(6, '6', '^');
    key_map_codes_case(7, '7', '&');
    key_map_codes_case(8, '8', '*');
    key_map_codes_case(9, '9', '[');
  
    key_map_codes_case(A, 'a', 'A');
    key_map_codes_case(B, 'b', 'B');
    key_map_codes_case(C, 'c', 'C');
    key_map_codes_case(D, 'd', 'D');
    key_map_codes_case(E, 'e', 'E');
    key_map_codes_case(F, 'f', 'F');
    key_map_codes_case(G, 'g', 'G');
    key_map_codes_case(H, 'h', 'H');
    key_map_codes_case(I, 'i', 'I');
    key_map_codes_case(J, 'j', 'J');
    key_map_codes_case(K, 'k', 'K');
    key_map_codes_case(L, 'l', 'L');
    key_map_codes_case(M, 'm', 'M');
    key_map_codes_case(N, 'n', 'N');
    key_map_codes_case(O, 'o', 'O');
    key_map_codes_case(P, 'p', 'P');
    key_map_codes_case(Q, 'q', 'Q');
    key_map_codes_case(R, 'r', 'R');
    key_map_codes_case(S, 's', 'S');
    key_map_codes_case(T, 't', 'T');
    key_map_codes_case(U, 'u', 'U');
    key_map_codes_case(V, 'v', 'V');
    key_map_codes_case(W, 'w', 'W');
    key_map_codes_case(X, 'x', 'X');
    key_map_codes_case(Y, 'y', 'Y');
    key_map_codes_case(Z, 'z', 'Z');
  
    key_map_codes_case(DOT, '.', '>');
    key_map_codes_case(COMMA, ',', '<');
    key_map_codes_case(SPACE, ' ', 0);
  
    key_map_codes_case_fn(KPMINUS, '-');
    key_map_codes_case_fn(EQUAL, '=');
    key_map_codes_case_fn(LEFTBRACE, '(');
    key_map_codes_case_fn(RIGHTBRACE, ')');
    key_map_codes_case_fn(SEMICOLON, ';');
    key_map_codes_case_fn(APOSTROPHE, '\'');
    key_map_codes_case_fn(BACKSLASH, '\\');
    key_map_codes_case_fn(SLASH, '/');
    key_map_codes_case_fn(QUESTION, '?');
    key_map_codes_case_fn(KPPLUS, '+');
    key_map_codes_case_fn(GRAVE, '`');
  
    key_map_codes_case_fn(F14, '|');
    key_map_codes_case_fn(F15, '_');
    key_map_codes_case_fn(F16, '#');
    key_map_codes_case_fn(F17, '!');
    key_map_codes_case_fn(F19, '"');
    key_map_codes_case_fn(F20, '@');
    key_map_codes_case_fn(F21, ':');

    default:
      return 0;
  }

  return 0;
}

#define update_key_map_extra(base_char, shift_char)                            \
  if(shift_mod)                                                                \
    event_input->key_letter = shift_char;                                      \
  else                                                                         \
    event_input->key_letter = base_char                                        \

u32 update_input(event_input_struct *event_input)
{
  s32 read_bytes;
  u32 read_data = 0;
  struct input_event event;

  event_input->config_button_action = CONFIG_BUTTON_NONE;
  event_input->key_action = KEY_ACTION_NONE;
  event_input->hat_status = HAT_STATUS_NONE;
  event_input->key_letter = 0;

  read_bytes = read(device_handle_gpio_keys, &event, sizeof(event));

  if((read_bytes > 0) && (event.type == EV_KEY))
  {
    read_data++;
    if(event.value == 0)
      event_input->action_type = INPUT_ACTION_TYPE_RELEASE;
    else
      event_input->action_type = INPUT_ACTION_TYPE_PRESS;

    event_input->config_button_action = gpio_keys_map(event.code);
  }

  read_bytes = read(device_handle_keypad, &event, sizeof(event));

  if((read_bytes > 0) && (event.type == EV_KEY) && (event.value == 1))
  {
    event_input->action_type = INPUT_ACTION_TYPE_PRESS;

    read_data++;
    switch(event.code)
    {
      case KEY_LEFTSHIFT:
        shift_mod = 2;
        break;

      case KEY_ENTER:
        event_input->key_action = KEY_ACTION_NETPLAY_TALK_CURSOR_ENTER;
        event_input->config_button_action = CONFIG_BUTTON_RUN;
        break;

      case KEY_T:
        update_key_map_extra('t', 'T');
        event_input->key_action = KEY_ACTION_NETPLAY_TALK;
        break;

      case KEY_BACKSPACE:
        event_input->key_action = KEY_ACTION_NETPLAY_TALK_CURSOR_BACKSPACE;
        break;

      case KEY_F:
        update_key_map_extra('f', 'F');
        event_input->config_button_action = CONFIG_BUTTON_FAST_FORWARD;
        break;

      case KEY_S:
        update_key_map_extra('s', 'S');
        event_input->config_button_action = CONFIG_BUTTON_SAVE_STATE;
        break;

      case KEY_L:
        update_key_map_extra('l', 'L');
        event_input->config_button_action = CONFIG_BUTTON_LOAD_STATE;
        break;

      case KEY_M:
        update_key_map_extra('m', 'M');
        event_input->config_button_action = CONFIG_BUTTON_MENU;
        break;

      default:
        event_input->key_letter = update_key_letter(event.code, shift_mod);
        break;
    }

    if(shift_mod)
      shift_mod--;
  }

  return read_data;
}

u8 gui_actions[16];

void clear_gui_actions()
{
  memset(gui_actions, 0, 16);
}

void get_gui_input(gui_input_struct *gui_input)
{
  struct input_event event;
  gui_action_type gui_action = CURSOR_NONE;

  static u64 button_repeat_timestamp;
  static button_repeat_state_type button_repeat_state = BUTTON_NOT_HELD;
  static gui_action_type cursor_repeat = CURSOR_NONE;
  s32 read_bytes;

  delay_us(10000);

  do
  {
    read_bytes = read(device_handle_gpio_keys, &event, sizeof(event));

    if((read_bytes > 0) && (event.type == EV_KEY))
    {
      if(event.value == 1)
      {
        gui_action = gpio_keys_gui_action_map(event.code);
        gui_actions[gui_action] = 1;
      }
      else

      if(event.value == 0)
      {
        gui_actions[gpio_keys_gui_action_map(event.code)] = 0;
      }
    }

    read_bytes = read(device_handle_keypad, &event, sizeof(event));

    if((read_bytes > 0) && (event.type == EV_KEY) && (event.value == 1))
    {
      if(event.value == 1)
      {
        if(event.code == KEY_LEFTSHIFT)
          shift_mod = 2;

        gui_input->key_letter = update_key_letter(event.code, shift_mod);

        if(gui_input->key_letter != 0)
          gui_action = CURSOR_LETTER;

        if(shift_mod)
          shift_mod--;
      }
    }
  } while(read_bytes > 0);

  if((gui_action != CURSOR_NONE) && (gui_action != CURSOR_LETTER))
  {
    get_ticks_us(&button_repeat_timestamp);
    button_repeat_state = BUTTON_HELD_INITIAL;
    cursor_repeat = gui_action;
  }
  else
  {
    if(gui_actions[cursor_repeat])
    {
      u64 new_ticks;
      get_ticks_us(&new_ticks);

      if(button_repeat_state == BUTTON_HELD_INITIAL)
      {
        if((new_ticks - button_repeat_timestamp) >
         BUTTON_REPEAT_START)
        {
          gui_action = cursor_repeat;
          button_repeat_timestamp = new_ticks;
          button_repeat_state = BUTTON_HELD_REPEAT;
        }
      }

      if(button_repeat_state == BUTTON_HELD_REPEAT)
      {
        if((new_ticks - button_repeat_timestamp) >
         BUTTON_REPEAT_CONTINUE)
        {
          gui_action = cursor_repeat;
          button_repeat_timestamp = new_ticks;
        }
      }
    }
  }

  gui_input->action_type = gui_action;
}

void initialize_event()
{
  char device_probe_name[128];
  u32 devices_to_probe = 2;
  u32 device_number = 0;
  s32 device_handle;
  u32 fcntl_flags;

  while(devices_to_probe)
  {
    sprintf(device_probe_name, "/dev/input/event%i", device_number);
    device_handle = open(device_probe_name, O_RDONLY);

    if(device_handle < 0)
      break;

    ioctl(device_handle, EVIOCGNAME(sizeof(device_probe_name)),
     device_probe_name);

    if(!strcmp(device_probe_name, "gpio-keys"))
    {
      printf("got gpio-keys at device %s\n", device_probe_name);
      fcntl_flags = fcntl(device_handle, F_GETFL);
      fcntl(device_handle, F_SETFL, fcntl_flags | O_NONBLOCK);
      device_handle_gpio_keys = device_handle;
      devices_to_probe--;
    }
    else

    if(!strcmp(device_probe_name, "keypad"))
    {
      printf("got keypad at device %s\n", device_probe_name);
      fcntl_flags = fcntl(device_handle, F_GETFL);
      fcntl(device_handle, F_SETFL, fcntl_flags | O_NONBLOCK);
      device_handle_keypad = device_handle;
      devices_to_probe--;
    }
    else
    {
      close(device_handle);
    }
    device_number++;
  }
}

