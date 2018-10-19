#ifndef	_GENERAL_
#define _GENERAL_

typedef int BOOL_D;
typedef unsigned int UINT;
typedef signed int SINT;
typedef unsigned char UINT8;
typedef signed char SINT8;
typedef unsigned short UINT16;
typedef signed short SINT16;
typedef signed int SINT32;
typedef unsigned int UINT32;
typedef signed long SLONG;
typedef unsigned long ULONG;
typedef signed short ANGLE;
typedef SINT32 f32;

enum{
	X,Y,Z,XYZ,XYZS,XY = Z,S = XYZ,SX = Z,SY = S
};

enum{
	PAD_DOWN = 0x01,
	PAD_LEFT = 0x02,
	PAD_RIGHT = 0x04,
	PAD_UP = 0x08,
	PAD_BUTTON1 = 0x10,
	PAD_BUTTON2 = 0x20,
	PAD_BUTTON3 = 0x40,
	PAD_BUTTON4 = 0x80,
	PAD_BUTTON5 = 0x100,
	PAD_BUTTON6 = 0x200,
	PAD_BUTTON7 = 0x400,
	PAD_BUTTON8 = 0x800,
	PAD_BUTTON9 = 0x1000,
	PAD_BUTTONA = 0x2000,
	PAD_BUTTONB = 0x4000,

	PAD_DIR = PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT,
	PAD_BUTTON = PAD_BUTTON1 | PAD_BUTTON2 | PAD_BUTTON3 | PAD_BUTTON4 | PAD_BUTTON5 | PAD_BUTTON6 | PAD_BUTTON7 | PAD_BUTTON8 | PAD_BUTTON9 | PAD_BUTTONA | PAD_BUTTONB,
	PAD_ALL = PAD_DIR | PAD_BUTTON,

	JOYSTICK_AXIS = 16384,

	REP_MIN = 2,
	REP_MAX = 30
};

#ifndef NULL
#define NULL 0
#endif

#define FALSE 0
#define TRUE 1
#define NONE 0
#define ABS(a) ((a) < 0 ? -(a) : (a))

typedef union REG32{
	struct{
		UINT16 low;
		UINT16 high;
	}half;
	UINT32 full;
}REG32;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(PSPUMODE) 
#if !defined(DREAMCAST)
#include <memory.h>
#endif
#endif

#include <math.h>

#endif /* _GENERAL_ */
