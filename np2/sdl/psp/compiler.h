#ifndef PSP
#define	PSP
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <pspuser.h>
// #include <malloc.h>

#define MAINSCR_W 320
#define MAINSCR_H 240
#define VRAM_W 320
#define VRAM_H 240
#define MAINSCR_BYTE 2

#define	BYTESEX_LITTLE
#define	OSLANG_SJIS
#define RESOURCE_US
#define	OSLINEBREAK_CRLF
#define	DISABLE_MATH_H
#define SOUND_CRITICAL
#define SUPPORT_SOFTKBD 0 // キーボードの画像のみ使う

typedef unsigned char BYTE;
typedef	signed char SINT8;
typedef	unsigned char UINT8;
typedef	signed short SINT16;
typedef	unsigned short UINT16;
typedef	signed int SINT32;
typedef	unsigned int UINT32;
typedef	int BOOL;
typedef unsigned int UINT;
typedef signed int SINT;
// typedef unsigned int size_t;
#ifdef CPUCORE_IA32 //NP21の場合はMakefileでCPUCORE_IA32がdefineされる
typedef unsigned long UINT64;
typedef signed long SINT64;
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef	TRUE
#define	TRUE 1
#endif

#ifndef	FALSE
#define	FALSE 0
#endif

#ifndef	MAX_PATH
#define	MAX_PATH 256
#endif

#ifndef offsetof
#define offsetof(type, mem) ((size_t)((char *)&((type *)0)->mem - (char *)(type *)0))
#endif

#ifndef	max
#define	max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef	min
#define	min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef	ZeroMemory
#define	ZeroMemory(d,n) memset((d), 0, (n))
#endif
#ifndef	CopyMemory
#define	CopyMemory(d,s,n) memcpy((d), (s), (n))
#endif
#ifndef	FillMemory
#define	FillMemory(a, b, c) memset((a), (c), (b))
#endif

#ifndef	roundup
#define	roundup(x, y) ((((x)+((y)-1))/(y))*(y))
#endif

#define	UNUSED(v) ((void)(v))

#ifndef	NELEMENTS
#define	NELEMENTS(a) ((int)(sizeof(a) / sizeof(a[0])))
#endif

#define	BRESULT UINT8
#define	OEMCHAR char
#define	OEMTEXT(string) string
#define	OEMSPRINTF sprintf
#define	OEMSTRLEN strlen


#include "common.h"
#include "milstr.h"
#include "_memory.h"
#include "rect.h"
#include "lstarray.h"
#include "trace.h"

// GETTICK() returns clock in milisec 
#define	GETTICK() (SDL_GetTicks())
#if defined(TRACE)
#define	__ASSERT(s) assert(s)
#else
#define	__ASSERT(s)
#endif
#define	SPRINTF sprintf
#define	STRLEN strlen

#define	LABEL __declspec(naked)
#define	RELEASE(x) if (x) {(x)->Release(); (x) = NULL;}

#ifdef CPUCORE_IA32
#define sigjmp_buf              jmp_buf
#define sigsetjmp(env, mask)    setjmp(env)
#define siglongjmp(env, val)    longjmp(env, val)
#endif


#define	SUPPORT_SJIS

#define	SUPPORT_16BPP
#define SCREEN_BPP 16

#ifdef SIZE_QVGA
#undef RGB16
#define RGB16 UINT32
#endif

#define	SOUNDRESERVE 100

#ifdef CPUCORE_IA32
#define	SUPPORT_PC9821
#define	SUPPORT_CRT31KHZ
#else
#define	SUPPORT_CRT15KHZ
#endif

#define	SUPPORT_HWSEEKSND
