// primitive graphics for Hello World PSP
#ifndef _PG_H_
#define _PG_H_

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define PIXELSIZE 1        //in short
#define LINESIZE 512       //in short
#define FRAMESIZE 0x44000  //in byte

// sound
#define MAXVOLUME 0x8000
#define OUT_FMT_STEREO 0x00
#define OUT_FMT_MONO   0x10

void pgSetDrawBuffer(void *buf);
char *pgGetVramAddr(unsigned long x,unsigned long y);
void pgaSetChannelCallback(void *callback);
int pgaInit(int samples);
void pgaPause(int n);
void pgScreenFlip(void);
void pgGeInit(void);
void pgScreenFrame(long mode, long frame);
#endif // _PG_H_
