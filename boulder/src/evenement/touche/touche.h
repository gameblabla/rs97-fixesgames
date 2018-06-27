#ifndef _TOUCHE_H
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

/**Topic: Documentation du module Touche
 * Ce module permet l'utilisation du claviers avec notamment la gestion
 * de l'appuie de touche simultané
 */

/*#define T_UP SDLK_UP
#define T_DOWN SDLK_DOWN
#define T_LEFT SDLK_LEFT
#define T_RIGHT SDLK_RIGHT
#define T_RETURN SDLK_RETURN
#define T_SPACE SDLK_SPACE
#define T_a     SDLK_a
*/

#define T_UNKNOWN		 0
#define T_FIRST		 0
#define T_BACKSPACE		 8
#define T_TAB		 9
#define T_CLEAR		 12
#define T_RETURN		 13
#define T_PAUSE		 19
#define T_ESCAPE		 27
#define T_SPACE		 32
#define T_EXCLAIM		 33
#define T_QUOTEDBL		 34
#define T_HASH		 35
#define T_DOLLAR		 36
#define T_AMPERSAND		 38
#define T_QUOTE		 39
#define T_LEFTPAREN		 40
#define T_RIGHTPAREN		 41
#define T_ASTERISK		 42
#define T_PLUS		 43
#define T_COMMA		 44
#define T_MINUS		 45
#define T_PERIOD		 46
#define T_SLASH		 47
#define T_0			 48
#define T_1			 49
#define T_2			 50
#define T_3			 51
#define T_4			 52
#define T_5			 53
#define T_6			 54
#define T_7			 55
#define T_8			 56
#define T_9			 57
#define T_COLON		 58
#define T_SEMICOLON		 59
#define T_LESS		 60
#define T_EQUALS		 61
#define T_GREATER		 62
#define T_QUESTION		 63
#define T_AT			 64
/* 
   Skip uppercase letters
 */
#define T_LEFTBRACKET	 91
#define T_BACKSLASH		 92
#define T_RIGHTBRACKET	 93
#define T_CARET		 94
#define T_UNDERSCORE		 95
#define T_BACKQUOTE		 96
#define T_a			 97
#define T_b			 98
#define T_c			 99
#define T_d			 100
#define T_e			 101
#define T_f			 102
#define T_g			 103
#define T_h			 104
#define T_i			 105
#define T_j			 106
#define T_k			 107
#define T_l			 108
#define T_m			 109
#define T_n			 110
#define T_o			 111
#define T_p			 112
#define T_q			 113
#define T_r			 114
#define T_s			 115
#define T_t			 116
#define T_u			 117
#define T_v			 118
#define T_w			 119
#define T_x			 120
#define T_y			 121
#define T_z			 122
#define T_DELETE		 127
/* End of ASCII mapped keysyms */
	/* International keyboard syms */
#define T_WORLD_0		 160		/* 0xA0 */
#define T_WORLD_1		 161
#define T_WORLD_2		 162
#define T_WORLD_3		 163
#define T_WORLD_4		 164
#define T_WORLD_5		 165
#define T_WORLD_6		 166
#define T_WORLD_7		 167
#define T_WORLD_8		 168
#define T_WORLD_9		 169
#define T_WORLD_10		 170
#define T_WORLD_11		 171
#define T_WORLD_12		 172
#define T_WORLD_13		 173
#define T_WORLD_14		 174
#define T_WORLD_15		 175
#define T_WORLD_16		 176
#define T_WORLD_17		 177
#define T_WORLD_18		 178
#define T_WORLD_19		 179
#define T_WORLD_20		 180
#define T_WORLD_21		 181
#define T_WORLD_22		 182
#define T_WORLD_23		 183
#define T_WORLD_24		 184
#define T_WORLD_25		 185
#define T_WORLD_26		 186
#define T_WORLD_27		 187
#define T_WORLD_28		 188
#define T_WORLD_29		 189
#define T_WORLD_30		 190
#define T_WORLD_31		 191
#define T_WORLD_32		 192
#define T_WORLD_33		 193
#define T_WORLD_34		 194
#define T_WORLD_35		 195
#define T_WORLD_36		 196
#define T_WORLD_37		 197
#define T_WORLD_38		 198
#define T_WORLD_39		 199
#define T_WORLD_40		 200
#define T_WORLD_41		 201
#define T_WORLD_42		 202
#define T_WORLD_43		 203
#define T_WORLD_44		 204
#define T_WORLD_45		 205
#define T_WORLD_46		 206
#define T_WORLD_47		 207
#define T_WORLD_48		 208
#define T_WORLD_49		 209
#define T_WORLD_50		 210
#define T_WORLD_51		 211
#define T_WORLD_52		 212
#define T_WORLD_53		 213
#define T_WORLD_54		 214
#define T_WORLD_55		 215
#define T_WORLD_56		 216
#define T_WORLD_57		 217
#define T_WORLD_58		 218
#define T_WORLD_59		 219
#define T_WORLD_60		 220
#define T_WORLD_61		 221
#define T_WORLD_62		 222
#define T_WORLD_63		 223
#define T_WORLD_64		 224
#define T_WORLD_65		 225
#define T_WORLD_66		 226
#define T_WORLD_67		 227
#define T_WORLD_68		 228
#define T_WORLD_69		 229
#define T_WORLD_70		 230
#define T_WORLD_71		 231
#define T_WORLD_72		 232
#define T_WORLD_73		 233
#define T_WORLD_74		 234
#define T_WORLD_75		 235
#define T_WORLD_76		 236
#define T_WORLD_77		 237
#define T_WORLD_78		 238
#define T_WORLD_79		 239
#define T_WORLD_80		 240
#define T_WORLD_81		 241
#define T_WORLD_82		 242
#define T_WORLD_83		 243
#define T_WORLD_84		 244
#define T_WORLD_85		 245
#define T_WORLD_86		 246
#define T_WORLD_87		 247
#define T_WORLD_88		 248
#define T_WORLD_89		 249
#define T_WORLD_90		 250
#define T_WORLD_91		 251
#define T_WORLD_92		 252
#define T_WORLD_93		 253
#define T_WORLD_94		 254
#define T_WORLD_95		 255		/* 0xFF */
	/* Numeric keypad */
#define T_KP0		 256
#define T_KP1		 257
#define T_KP2		 258
#define T_KP3		 259
#define T_KP4		 260
#define T_KP5		 261
#define T_KP6		 262
#define T_KP7		 263
#define T_KP8		 264
#define T_KP9		 265
#define T_KP_PERIOD		 266
#define T_KP_DIVIDE		 267
#define T_KP_MULTIPLY	 268
#define T_KP_MINUS		 269
#define T_KP_PLUS		 270
#define T_KP_ENTER		 271
#define T_KP_EQUALS		 272
	/* Arrows + Home/End pad */
#define T_UP			 273
#define T_DOWN		 274
#define T_RIGHT		 275
#define T_LEFT		 276
#define T_INSERT		 277
#define T_HOME		 278
#define T_END		 279
#define T_PAGEUP		 280
#define T_PAGEDOWN		 281
	/* Function keys */
#define T_F1			 282
#define T_F2			 283
#define T_F3			 284
#define T_F4			 285
#define T_F5			 286
#define T_F6			 287
#define T_F7			 288
#define T_F8			 289
#define T_F9			 290
#define T_F10		 291
#define T_F11		 292
#define T_F12		 293
#define T_F13		 294
#define T_F14		 295
#define T_F15		 296
	/* Key state modifier keys */
#define T_NUMLOCK		 300
#define T_CAPSLOCK		 301
#define T_SCROLLOCK		 302
#define T_RSHIFT		 303
#define T_LSHIFT		 304
#define T_RCTRL		 305
#define T_LCTRL		 306
#define T_RALT		 307
#define T_LALT		 308
#define T_RMETA		 309
#define T_LMETA		 310
#define T_LSUPER		 311		/* Left "Windows" key */
#define T_RSUPER		 312		/* Right "Windows" key */
#define T_MODE		 313		/* "Alt Gr" key */
#define T_COMPOSE		 314		/* Multi-key compose key */
	/* Miscellaneous function keys */
#define T_HELP		 315
#define T_PRINT		 316
#define T_SYSREQ		 317
#define T_BREAK		 318
#define T_MENU		 319
#define T_POWER		 320		/* Power Macintosh power key */
#define T_EURO		 321		/* Some european keyboards */
#define T_UNDO		 322		/* Atari keyboard has Un*/
	
	

typedef SDL_Event Evenement_touche;
typedef Uint8     Tableau_touche;

/************************Group: Type Touche***********************/
/* Struct: Touche
 * Le type abstrait touche est de la forme T_UP, T_SPACE, T_a, T_DOWN, T_z...
 */
typedef SDLKey    Touche;


/**Function: toucheEstAppuyer
 * Permet de savoir si une touche est appuyé
 * 
 * Paramètre:
 * touche - la touche que l'on souhaite détecter l'appuie
 *
 * Retour:
 *  Si la touche n'est pas appuyé, 0 est renvoyé, sinon, dans le cas d'appuie, un entier
 *  différent de 0 est renvoyé
 *
 * Indication:
 * Les touches sont de la forme: T_UP, T_SPACE, T_a, T_DOWN, T_z...
 */
int             toucheEstAppuyer(Touche touche);


#endif
