#ifndef IPHONE_EVENT_H
#define IPHONE_EVENT_H

typedef enum
{
  GP2X_UP       = 1 << 0,
  GP2X_LEFT     = 1 << 2,
  GP2X_DOWN     = 1 << 4,
  GP2X_RIGHT    = 1 << 6,
  GP2X_START    = 1 << 8,
  GP2X_SELECT   = 1 << 9,
  GP2X_L        = 1 << 10,
  GP2X_R        = 1 << 11,
  GP2X_A        = 1 << 12,
  GP2X_B        = 1 << 13,
  GP2X_X        = 1 << 14,
  GP2X_Y        = 1 << 15,
  GP2X_VOL_DOWN = 1 << 22,
  GP2X_VOL_UP   = 1 << 23,
  GP2X_VOL_MID  = 1 << 24,
  GP2X_PUSH     = 1 << 27
} iphone_buttons_enum;

typedef enum
{
  GP2X_CONF_UP,
  GP2X_CONF_DOWN,
  GP2X_CONF_LEFT,
  GP2X_CONF_RIGHT,
  GP2X_CONF_A,
  GP2X_CONF_B,
  GP2X_CONF_X,
  GP2X_CONF_Y,
  GP2X_CONF_L,
  GP2X_CONF_R,
  GP2X_CONF_START,
  GP2X_CONF_SELECT,
  GP2X_CONF_VOL_DOWN,
  GP2X_CONF_VOL_UP,
  GP2X_CONF_VOL_MID,
  GP2X_CONF_STICK,
  GP2X_CONF_NONE
} iphone_buttons_config_enum;


#endif

