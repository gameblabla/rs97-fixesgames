#ifndef WIZ_EVENT_H
#define WIZ_EVENT_H

typedef enum
{
  WIZ_UP       = (1 << 18),
  WIZ_LEFT     = (1 << 16),
  WIZ_DOWN     = (1 << 19),
  WIZ_RIGHT    = (1 << 17),
  WIZ_MENU     = (1 << 9),
  WIZ_SELECT   = (1 << 8),
  WIZ_L        = (1 << 7),
  WIZ_R        = (1 << 6),
  WIZ_A        = (1 << 20),
  WIZ_B        = (1 << 21),
  WIZ_X        = (1 << 22),
  WIZ_Y        = (1 << 23),
  WIZ_VOL_DOWN = (1 << 11),
  WIZ_VOL_UP   = (1 << 10),
  WIZ_VOL_MID  = (1 << 24),
} wiz_buttons_enum;

typedef enum
{
  WIZ_CONF_UP,
  WIZ_CONF_DOWN,
  WIZ_CONF_LEFT,
  WIZ_CONF_RIGHT,
  WIZ_CONF_A,
  WIZ_CONF_B,
  WIZ_CONF_X,
  WIZ_CONF_Y,
  WIZ_CONF_L,
  WIZ_CONF_R,
  WIZ_CONF_MENU,
  WIZ_CONF_SELECT,
  WIZ_CONF_VOL_DOWN,
  WIZ_CONF_VOL_UP,
  WIZ_CONF_VOL_MID,
  WIZ_CONF_NONE
} wiz_buttons_config_enum;


#endif

