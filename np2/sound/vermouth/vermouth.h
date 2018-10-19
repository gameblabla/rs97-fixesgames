
#ifndef __VERMOUTH_H
#define	__VERMOUTH_H

typedef struct {
	UINT	samprate;
} *MIDIMOD;

typedef struct {
	UINT	samprate;
	UINT	worksize;
} *MIDIHDL;


#ifdef __cplusplus
extern "C" {
#endif

UINT midiout_getver(OEMCHAR *string, int leng);

MIDIMOD midimod_create(UINT samprate);
void midimod_destroy(MIDIMOD hdl);
void midimod_loadprogram(MIDIMOD hdl, UINT num);
void midimod_loadrhythm(MIDIMOD hdl, UINT num);
void midimod_loadgm(MIDIMOD hdl);
void midimod_loadall(MIDIMOD hdl);

MIDIHDL midiout_create(MIDIMOD module, UINT worksize);
void midiout_destroy(MIDIHDL hdl);
void midiout_shortmsg(MIDIHDL hdl, UINT32 msg);
void midiout_longmsg(MIDIHDL hdl, const void *msg, UINT size);
const SINT32 *midiout_get(MIDIHDL hdl, UINT *samples);
UINT midiout_get32(MIDIHDL hdl, SINT32 *pcm, UINT size);
void midiout_setgain(MIDIHDL hdl, int gain);

#ifdef __cplusplus
}
#endif

#endif

