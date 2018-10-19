
#if !defined(_WIN32_WCE) && !defined(SLZAURUS)
#define	ENABLE_TREMOLO
#define	ENABLE_VIRLATE
#define	ENABLE_GSRX
#define	PANPOT_REVA
// #define	VOLUME_ACURVE
#else
#define	MIDI_GMONLY
#endif

struct _midimodule;
typedef	struct _midimodule	_MIDIMOD;
typedef	struct _midimodule	*MIDIMOD;

struct _midictrl;
typedef	struct _midictrl	_MIDIHDL;
typedef	struct _midictrl	*MIDIHDL;

enum {
	MIDIOUT_SUCCESS		= 0,
	MIDIOUT_FAILURE		= -1
};

#define	VOICE_MAX		24

#define	SAMP_SHIFT		12
#define	SAMP_LIMIT		((1 << (SAMP_SHIFT + 1)) - 1)

#define	FREQ_SHIFT		12
#define	FREQ_MASK		((1 << FREQ_SHIFT) - 1)

#define	ENV_RATE		22
#define	ENVRATE_SHIFT	10

#define	TRESWEEP_SHIFT	16
#define	TRERATE_SHIFT	5
#define	TRESWEEP_TUNE	38
#define	TRERATE_TUNE	38

#define	VIBSWEEP_SHIFT	16
#define	VIBRATE_SHIFT	6
#define	VIBSWEEP_TUNE	38
#define	VIBRATE_TUNE	38

#define	REL_COUNT		20


#if defined(MIDI_GMONLY)
#define	MIDI_BANKS	1
#else
#define	MIDI_BANKS	128
#endif

#include	"midimod.h"
#include	"midinst.h"
#include	"midvoice.h"
#include	"midtable.h"

struct _midimodule {
	UINT		samprate;
	INSTRUMENT	*tone[MIDI_BANKS * 2];
	TONECFG		tonecfg[MIDI_BANKS * 2];

	PATHLIST	pathlist;
	LISTARRAY	pathtbl;
	LISTARRAY	namelist;
};


struct _midictrl {
	UINT		samprate;
	UINT		worksize;
	int			level;
	UINT8		status;
	SINT8		gain;
	UINT8		master;

	MIDIMOD		module;
	INSTRUMENT	*bank0[2];

	SINT32		*sampbuf;
	SAMPLE		resampbuf;

	_CHANNEL	channel[16];
	_VOICE		voice[VOICE_MAX];
};

#ifndef VERMOUTH_OVL_EXPORTS
#define	AEXTERN
#define	AEXPORT
#else
#define	AEXTERN		__declspec(dllexport)
#define	AEXPORT		WINAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

AEXTERN UINT AEXPORT midiout_getver(OEMCHAR *string, int leng);

AEXTERN _MIDIMOD AEXPORT * midimod_create(UINT samprate);
AEXTERN void AEXPORT midimod_destroy(MIDIMOD hdl);
AEXTERN void AEXPORT midimod_loadprogram(MIDIMOD hdl, UINT num);
AEXTERN void AEXPORT midimod_loadrhythm(MIDIMOD hdl, UINT num);
AEXTERN void AEXPORT midimod_loadgm(MIDIMOD hdl);
AEXTERN void AEXPORT midimod_loadall(MIDIMOD hdl);

AEXTERN _MIDIHDL AEXPORT * midiout_create(MIDIMOD module, UINT worksize);
AEXTERN void AEXPORT midiout_destroy(MIDIHDL hdl);
AEXTERN void AEXPORT midiout_shortmsg(MIDIHDL hdl, UINT32 msg);
AEXTERN void AEXPORT midiout_longmsg(MIDIHDL hdl, const UINT8 *msg, UINT size);
AEXTERN const SINT32 AEXPORT * midiout_get(MIDIHDL hdl, UINT *samples);
AEXTERN UINT AEXPORT midiout_get32(MIDIHDL hdl, SINT32 *pcm, UINT size);
AEXTERN void AEXPORT midiout_setgain(MIDIHDL hdl, int gain);

#ifdef __cplusplus
}
#endif

