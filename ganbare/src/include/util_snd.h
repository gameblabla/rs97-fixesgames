#ifndef _UTIL_SND_
#define _UTIL_SND_

enum{
	SOUND_MUSBANK = 32,
	SOUND_CHUNKBANK = 32,
	SOUND_MIXBANK = 8
};

enum {
	BGM_GAME01 = 0,

	SE_SEL = 0,
	SE_COLLECT,
	SE_SHOT,
	SE_SHIPBRK,
	SE_CHIP,
	SE_DMG,
	SE_ENEEXP,
	SE_ENEBRK,
	SE_EXTEND,
	SE_ALERT,
};

extern void soundInitBuffer(void);
extern void soundRelease(void);
extern void soundLoadBuffer(SINT num, UINT8 *fname, int loop);
extern void soundLoadBuffer2(SINT num, UINT8 *fname1, UINT8 *fname2);
extern void soundLoadBufferSE(SINT num, UINT8 *fname);
extern void soundStopBgm(SINT num);
extern void soundStopBgmPlaying(void);
extern int soundIsPlayBgm(void);
extern void soundPlayBgm(SINT num);
extern void soundPlayFadeFlag(SINT flag, SINT time);
extern void soundPlayCtrl(void);
extern void soundSetVolumeMaster(SINT vol);
extern void soundSetVolumeBgm(SINT vol, SINT num);
extern void soundSetVolumeAll(SINT vol);
extern void soundStopSe(SINT num);
extern void soundPlaySe(SINT num);
extern int soundIsPlaySe(SINT num);
extern void soundStopSeAll(void);

extern SINT sound_buff[];

#endif /* _UTIL_SND_ */
