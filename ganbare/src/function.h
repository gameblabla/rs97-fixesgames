#ifndef __FUNCTION
#define __FUNCTION

#include "include/general.h"

enum { sdljbTriangle = 0, sdljbCircle, sdljbCross, sdljbSquare, sdljbLTrig, sdljbRTrig, sdljbDown, sdljbLeft, sdljbUp, sdljbRight, sdljbSelect, sdljbStart, sdljbHome } SDLJoyBtn;

extern void ResetGameFlag( void );
extern void ResetGameFlag2( void );
extern int LoadGameFlag( char *fn );
extern int SaveGameFlag( char *fn );
extern int LoadGameFlag2( char *fn );
extern int SaveGameFlag2( char *fn );
extern int SaveFile( char *fn, long *buff, long size );
extern int LoadFile( char *fn, long *buff, long size );
extern long GetConfig( char* fn, char* cParam );
extern long LogFileWrite( char* fn, char* cParam );

extern long LoadBitmap( char *fname, int bmpindex, int flag );
extern void ReleaseBitmap( int bmpindex );
extern void Blt( int bmpindex, int dstX, int dstY );
extern void BltRect(int bmpindex, int srcX, int srcY, int dstX, int dstY, int width, int height);
extern void ClearSecondary( void );
extern void BltNumericImage( long value, long length, long x, long y, long plane, long num_stpos_x, long num_stpos_y, long num_width, long num_height );
extern void BltNumericImage2( long value, long length, long x, long y, long plane, long num_stpos_x, long num_stpos_y, long num_width, long num_height );
extern void SetGscreenPalette( SDL_Surface *surface );
extern void SetPalette(int getbmpindex, int setbmpindex);
extern void BltRectRotZoom( int bmpindex, int dstX, int dstY, int srcX, int srcY, int width, int height, double angle, double zoom, int smooth);
extern void CreateSurface( int bmpindex, int size_x, int size_y  );
extern void SwapToSecondary( int bmpindex );
extern void SaveBmp( int bmpindex, char *fn );
extern void drawGRPline(f32 x1, f32 y1, f32 x2, f32 y2, Uint32 color);

extern int IsPushKey( int keycode );
extern int IsPressKey( int keycode );
extern void KeyInput( void );
extern int initPAD(void);
extern void closePAD(void);
extern int getPAD(void);
extern int IsPushOKKey( void );
extern int IsPushCancelKey( void );

extern void FunctionInit( void );
extern void FPSWait( void );
extern int system_keys( void );
extern int Set_Volume( int vol );

extern long funcSin( long rdo );
extern long funcCos( long rdo );
extern long funcTan2( long posX, long posY );
extern long get2keta( long val, long st );

#endif
