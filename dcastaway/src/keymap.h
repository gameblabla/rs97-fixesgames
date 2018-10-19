/* SDL symbolic key to ST scan code mapping table */
static const char keysymToAtari[SDLK_LAST] =
{
/* ST Code,  PC Code */
  0x39,    /* 0 */
  0x39,    /* 1 */
  0x39,    /* 2 */
  0x39,    /* 3 */
  0x39,    /* 4 */
  0x39,    /* 5 */
  0x39,    /* 6 */
  0x39,    /* 7 */
  0x0E,  /* SDLK_BACKSPACE=8 */
  0x0F,  /* SDLK_TAB=9 */
  0x39,    /* 10 */
  0x39,    /* 11 */
  0x47,  /* SDLK_CLEAR = 12 */
  0x1C,  /* SDLK_RETURN = 13 */
  0x39,    /* 14 */
  0x39,    /* 15 */
  0x39,    /* 16 */
  0x39,    /* 17 */
  0x39,    /* 18 */
  0x39,    /* SDLK_PAUSE = 19 */
  0x39,    /* 20 */
  0x39,    /* 21 */
  0x39,    /* 22 */
  0x39,    /* 23 */
  0x39,    /* 24 */
  0x39,    /* 25 */
  0x39,    /* 26 */
  0x01,  /* SDLK_ESCAPE = 27 */
  0x39,    /* 28 */
  0x39,    /* 29 */
  0x39,    /* 30 */
  0x39,    /* 31 */
  0x39,  /* SDLK_SPACE = 32 */
  0x39,    /* SDLK_EXCLAIM = 33 */
  0x39,    /* SDLK_QUOTEDBL = 34 */
  0x29,  /* SDLK_HASH = 35 */
  0x39,    /* SDLK_DOLLAR = 36 */
  0x39,    /* 37 */
  0x39,    /* SDLK_AMPERSAND = 38 */
  0x39,    /* SDLK_QUOTE = 39 */
  0x63,  /* SDLK_LEFTPAREN = 40 */
  0x64,  /* SDLK_RIGHTPAREN = 41 */
  0x39,    /* SDLK_ASTERISK = 42 */
  0x1B,  /* SDLK_PLUS = 43 */
  0x33,  /* SDLK_COMMA = 44 */
  0x35,  /* SDLK_MINUS = 45 */
  0x34,  /* SDLK_PERIOD = 46 */
  0x39,    /* SDLK_SLASH = 47 */
  0x0B,  /* SDLK_0 = 48 */
  0x02,  /* SDLK_1 = 49 */
  0x03,  /* SDLK_2 = 50 */
  0x04,  /* SDLK_3 = 51 */
  0x05,  /* SDLK_4 = 52 */
  0x06,  /* SDLK_5 = 53 */
  0x07,  /* SDLK_6 = 54 */
  0x08,  /* SDLK_7 = 55 */
  0x09,  /* SDLK_8 = 56 */
  0x0A,  /* SDLK_9 = 57 */
  0x39,    /* SDLK_COLON = 58 */
  0x39,    /* SDLK_SEMICOLON = 59 */
  0x60,  /* SDLK_LESS = 60 */
  0x39,    /* SDLK_EQUALS = 61 */
  0x39,    /* SDLK_GREATER  = 62 */
  0x39,    /* SDLK_QUESTION = 63 */
  0x39,    /* SDLK_AT = 64 */
  0x39,    /* 65 */  /* Skip uppercase letters */
  0x39,    /* 66 */
  0x39,    /* 67 */
  0x39,    /* 68 */
  0x39,    /* 69 */
  0x39,    /* 70 */
  0x39,    /* 71 */
  0x39,    /* 72 */
  0x39,    /* 73 */
  0x39,    /* 74 */
  0x39,    /* 75 */
  0x39,    /* 76 */
  0x39,    /* 77 */
  0x39,    /* 78 */
  0x39,    /* 79 */
  0x39,    /* 80 */
  0x39,    /* 81 */
  0x39,    /* 82 */
  0x39,    /* 83 */
  0x39,    /* 84 */
  0x39,    /* 85 */
  0x39,    /* 86 */
  0x39,    /* 87 */
  0x39,    /* 88 */
  0x39,    /* 89 */
  0x39,    /* 90 */
  0x63,  /* SDLK_LEFTBRACKET = 91 */
  0x39,    /* SDLK_BACKSLASH = 92 */
  0x64,  /* SDLK_RIGHTBRACKET = 93 */
  0x2B,  /* SDLK_CARET = 94 */
  0x39,    /* SDLK_UNDERSCORE = 95 */
  0x39,    /* SDLK_BACKQUOTE = 96 */
  0x1E,  /* SDLK_a = 97 */
  0x30,  /* SDLK_b = 98 */
  0x2E,  /* SDLK_c = 99 */
  0x20,  /* SDLK_d = 100 */
  0x12,  /* SDLK_e = 101 */
  0x21,  /* SDLK_f = 102 */
  0x22,  /* SDLK_g = 103 */
  0x23,  /* SDLK_h = 104 */
  0x17,  /* SDLK_i = 105 */
  0x24,  /* SDLK_j = 106 */
  0x25,  /* SDLK_k = 107 */
  0x26,  /* SDLK_l = 108 */
  0x32,  /* SDLK_m = 109 */
  0x31,  /* SDLK_n = 110 */
  0x18,  /* SDLK_o = 111 */
  0x19,  /* SDLK_p = 112 */
  0x10,  /* SDLK_q = 113 */
  0x13,  /* SDLK_r = 114 */
  0x1F,  /* SDLK_s = 115 */
  0x14,  /* SDLK_t = 116 */
  0x16,  /* SDLK_u = 117 */
  0x2F,  /* SDLK_v = 118 */
  0x11,  /* SDLK_w = 119 */
  0x2D,  /* SDLK_x = 120 */
  0x15,  /* SDLK_y = 121 */
  0x2C,  /* SDLK_z = 122 */
  0x39,    /* 123 */
  0x39,    /* 124 */
  0x39,    /* 125 */
  0x39,    /* 126 */
  0x53,  /* SDLK_DELETE = 127 */
  /* End of ASCII mapped keysyms */
  0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, /* 1280x3943*/
  0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, /* 1440x3959*/
  0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, /* 1600x3975*/
  0x39, 0x39, 0x39, 0x39, 0x0d, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, /* 1760x3991*/
  0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, /* 192-207*/
  0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, /* 208-223*/
  0x39, 0x39, 0x39, 0x39, 0x28, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, /* 224-239*/
  0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x27, 0x39, 0x39, 0x39, 0x39, 0x39, 0x1A, 0x39, 0x39, 0x39, /* 240-255*/
  /* Numeric keypad: */
  0x70,    /* SDLK_KP0 = 256 */
  0x6D,    /* SDLK_KP1 = 257 */
  0x6E,    /* SDLK_KP2 = 258 */
  0x6F,    /* SDLK_KP3 = 259 */
  0x6A,    /* SDLK_KP4 = 260 */
  0x6B,    /* SDLK_KP5 = 261 */
  0x6C,    /* SDLK_KP6 = 262 */
  0x67,    /* SDLK_KP7 = 263 */
  0x68,    /* SDLK_KP8 = 264 */
  0x69,    /* SDLK_KP9 = 265 */
  0x71,    /* SDLK_KP_PERIOD = 266 */
  0x65,    /* SDLK_KP_DIVIDE = 267 */
  0x66,    /* SDLK_KP_MULTIPLY = 268 */
  0x4A,    /* SDLK_KP_MINUS = 269 */
  0x4E,    /* SDLK_KP_PLUS = 270 */
  0x72,    /* SDLK_KP_ENTER = 271 */
  0x39,      /* SDLK_KP_EQUALS = 272 */
  /* Arrows + Home/End pad */
  0x48,    /* SDLK_UP = 273 */
  0x50,    /* SDLK_DOWN = 274 */
  0x4D,    /* SDLK_RIGHT = 275 */
  0x4B,    /* SDLK_LEFT = 276 */
  0x52,    /* SDLK_INSERT = 277 */
  0x47,    /* SDLK_HOME = 278 */
  0x61,    /* SDLK_END = 279 */
  0x63,    /* SDLK_PAGEUP = 280 */
  0x64,    /* SDLK_PAGEDOWN = 281 */
  /* Function keys */
  0x3B,    /* SDLK_F1 = 282 */
  0x3C,    /* SDLK_F2 = 283 */
  0x3D,    /* SDLK_F3 = 284 */
  0x3E,    /* SDLK_F4 = 285 */
  0x3F,    /* SDLK_F5 = 286 */
  0x40,    /* SDLK_F6 = 287 */
  0x41,    /* SDLK_F7 = 288 */
  0x42,    /* SDLK_F8 = 289 */
  0x43,    /* SDLK_F9 = 290 */
  0x44,    /* SDLK_F10 = 291 */
  0x39,      /* SDLK_F11 = 292 */
  0x39,      /* SDLK_F12 = 293 */
  0x39,      /* SDLK_F13 = 294 */
  0x39,      /* SDLK_F14 = 295 */
  0x39,      /* SDLK_F15 = 296 */
  0x39,      /* 297 */
  0x39,      /* 298 */
  0x39,      /* 299 */
  /* Key state modifier keys */
  0x39,      /* SDLK_NUMLOCK = 300 */
  0x3A,    /* SDLK_CAPSLOCK = 301 */
  0x61,    /* SDLK_SCROLLOCK = 302 */
  0x36,    /* SDLK_RSHIFT = 303 */
  0x2A,    /* SDLK_LSHIFT = 304 */
  0x1D,    /* SDLK_RCTRL = 305 */
  0x1D,    /* SDLK_LCTRL = 306 */
  0x38,    /* SDLK_RALT = 307 */
  0x38,    /* SDLK_LALT = 308 */
  0x39,      /* SDLK_RMETA = 309 */
  0x39,      /* SDLK_LMETA = 310 */
  0x39,      /* SDLK_LSUPER = 311 */
  0x39,      /* SDLK_RSUPER = 312 */
  0x39,      /* SDLK_MODE = 313 */     /* "Alt Gr" key */
  0x39,      /* SDLK_COMPOSE = 314 */
  /* Miscellaneous function keys */
  0x62,    /* SDLK_HELP = 315 */
  0x62,    /* SDLK_PRINT = 316 */
  0x39,      /* SDLK_SYSREQ = 317 */
  0x39,      /* SDLK_BREAK = 318 */
  0x39,      /* SDLK_MENU = 319 */
  0x39,      /* SDLK_POWER = 320 */
  0x39,      /* SDLK_EURO = 321 */
  0x61     /* SDLK_UNDO = 322 */
};
