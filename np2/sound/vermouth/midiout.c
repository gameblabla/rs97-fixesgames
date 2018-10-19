#include	"compiler.h"
#include	"midiout.h"


#define	MIDIOUT_VERSION		0x105
#define	MIDIOUT_VERSTRING	"VERMOUTH 1.05"


static const OEMCHAR vermouthver[] = OEMTEXT(MIDIOUT_VERSTRING);

static const int gaintbl[24+1] =
				{ 16,  19,  22,  26,  32,  38,  45,  53,
				  64,  76,  90, 107, 128, 152, 181, 215,
				 256, 304, 362, 430, 512, 608, 724, 861, 1024};


// ---- voice

static void voice_volupdate(VOICE v) {

	CHANNEL	ch;
	int		vol;

	ch = v->channel;
#if defined(VOLUME_ACURVE)
	vol = ch->level * acurve[v->velocity];
	vol >>= 8;
#else
	vol = ch->level * v->velocity;
	vol >>= 7;
#endif
	vol *= v->sample->volume;
	vol >>= (21 - 16);
#if defined(PANPOT_REVA)
	switch(v->flag & VOICE_MIXMASK) {
		case VOICE_MIXNORMAL:
			v->volleft = (vol * revacurve[v->panpot ^ 127]) >> 8;
			v->volright = (vol * revacurve[v->panpot]) >> 8;
			break;

		case VOICE_MIXCENTRE:
			v->volleft = (vol * 155) >> 8;
			break;

		default:
			v->volleft = vol;
			break;
	}
#else
	v->volleft = vol;
	if ((v->flag & VOICE_MIXMASK) == VOICE_MIXNORMAL) {
		if (v->panpot < 64) {
			vol *= (v->panpot + 1);
			vol >>= 6;
			v->volright = vol;
		}
		else {
			v->volright = vol;
			vol *= (127 - v->panpot);
			vol >>= 6;
			v->volleft = vol;
		}
	}
#endif
}

static INSTLAYER selectlayer(VOICE v, INSTRUMENT inst) {

	int			layers;
	INSTLAYER	layer;
	INSTLAYER	layerterm;
	int			freq;
	int			diffmin;
	int			diff;
	INSTLAYER	layersel;

	layers = inst->layers;
	layer = (INSTLAYER)(inst + 1);

	if (layers == 1) {
		return(layer);
	}

	layerterm = layer + layers;
	freq = v->frequency;
	do {
		if ((freq >= layer->freqlow) && (freq <= layer->freqhigh)) {
			return(layer);
		}
		layer++;
	} while(layer < layerterm);

	layer = (INSTLAYER)(inst + 1);
	layersel = layer;
	diffmin = layer->freqroot - freq;
	if (diffmin < 0) {
		diffmin *= -1;
	}
	layer++;
	do {
		diff = layer->freqroot - freq;
		if (diff < 0) {
			diff *= -1;
		}
		if (diffmin > diff) {
			diffmin = diff;
			layersel = layer;
		}
		layer++;
	} while(layer < layerterm);
	return(layersel);
}

static void freq_update(VOICE v) {

	CHANNEL	ch;
	float	step;

	if (v->flag & VOICE_FIXPITCH) {
		return;
	}

	ch = v->channel;
	step = v->freq;
	if (ch->pitchbend != 0x2000) {
		step *= ch->pitchfactor;
	}
#if defined(ENABLE_VIRLATE)
	v->freqnow = step;
#endif
	step *= (float)(1 << FREQ_SHIFT);
	if (v->sampstep < 0) {
		step *= -1.0;
	}
	v->sampstep = (int)step;
}

static void voice_on(MIDIHDL midi, CHANNEL ch, VOICE v, int key, int vel) {

	INSTRUMENT	inst;
	INSTLAYER	layer;
	int			panpot;

	key &= 0x7f;
	if (!(ch->flag & CHANNEL_RHYTHM)) {
		inst = ch->inst;
		if (inst == NULL) {
			return;
		}
		if (inst->freq) {
			v->frequency = inst->freq;
		}
		else {
			v->frequency = freq_table[key];
		}
		layer = selectlayer(v, inst);
	}
	else {
#if !defined(MIDI_GMONLY)
		inst = ch->rhythm[key];
		if (inst == NULL) {
			inst = midi->bank0[1][key];
		}
#else
		inst = midi->bank0[1][key];
#endif
		if (inst == NULL) {
			return;
		}
		layer = (INSTLAYER)(inst + 1);
		if (inst->freq) {
			v->frequency = inst->freq;
		}
		else {
			v->frequency = freq_table[key];
		}
	}
	v->sample = layer;

	v->phase = VOICE_ON;
	v->channel = ch;
	v->note = key;
	v->velocity = vel;
	v->samppos = 0;
	v->sampstep = 0;

#if defined(ENABLE_TREMOLO)
	v->tremolo.count = 0;
	v->tremolo.step = layer->tremolo_step;
	v->tremolo.sweepstep = layer->tremolo_sweep;
	v->tremolo.sweepcount = 0;
#endif

#if defined(ENABLE_VIRLATE)
	v->vibrate.sweepstep = layer->vibrate_sweep;
	v->vibrate.sweepcount = 0;
	v->vibrate.rate = layer->vibrate_rate;
	v->vibrate.count = 0;
	v->vibrate.phase = 0;
#endif

	if (!(ch->flag & CHANNEL_RHYTHM)) {
		panpot = ch->panpot;
	}
	else {
		panpot = layer->panpot;
	}
#if defined(PANPOT_REVA)
	if (panpot == 64) {
		v->flag = VOICE_MIXCENTRE;
	}
	else if (panpot < 3) {
		v->flag = VOICE_MIXLEFT;
	}
	else if (panpot >= 126) {
		v->flag = VOICE_MIXRIGHT;
	}
	else {
		v->flag = VOICE_MIXNORMAL;
		v->panpot = panpot;
	}
#else
	if ((panpot >= 60) && (panpot < 68)) {
		v->flag = VOICE_MIXCENTRE;
	}
	else if (panpot < 5) {
		v->flag = VOICE_MIXLEFT;
	}
	else if (panpot >= 123) {
		v->flag = VOICE_MIXRIGHT;
	}
	else {
		v->flag = VOICE_MIXNORMAL;
		v->panpot = panpot;
	}
#endif
	if (!layer->samprate) {
		v->flag |= VOICE_FIXPITCH;
	}
	else {
		v->freq = (float)layer->samprate / (float)midi->samprate *
					(float)v->frequency / (float)layer->freqroot;
	}
	voice_setphase(v, VOICE_ON);
	freq_update(v);
	voice_volupdate(v);
	v->envcount = 0;
	if (layer->mode & MODE_ENVELOPE) {
		v->envvol = 0;
		envlope_setphase(v, 0);
	}
	else {
		v->envstep = 0;
	}
	voice_setmix(v);
	envelope_updates(v);
}

static void voice_off(VOICE v) {

	voice_setphase(v, VOICE_OFF);
	if (v->sample->mode & MODE_ENVELOPE) {
		envlope_setphase(v, 3);
		voice_setmix(v);
		envelope_updates(v);
	}
}

static void allresetvoices(MIDIHDL midi) {

	VOICE	v;
	VOICE	vterm;

	v = midi->voice;
	vterm = v + VOICE_MAX;
	do {
		voice_setfree(v);
		v++;
	} while(v < vterm);
}


// ---- key

static void key_on(MIDIHDL midi, CHANNEL ch, int key, int vel) {

	VOICE	v;
	VOICE	v1;
	VOICE	v2;
	int		vol;
	int		volmin;

	v = NULL;
	v1 = midi->voice;
	v2 = v1 + VOICE_MAX;
	do {
		v2--;
		if (v2->phase == VOICE_FREE) {
			v = v2;
		}
		else if ((v2->channel == ch) &&
				((v2->note == key) || (ch->flag & CHANNEL_MONO))) {
			voice_setphase(v2, VOICE_REL);
			voice_setmix(v2);
		}
	} while(v1 < v2);

	if (v != NULL) {
		voice_on(midi, ch, v, key, vel);
		return;
	}

	volmin = 0x7fffffff;
	v2 = v1 + VOICE_MAX;
	do {
		v2--;
		if (!(v2->phase & (VOICE_ON | VOICE_REL))) {
			vol = v2->envleft;
			if ((v2->flag & VOICE_MIXMASK) == VOICE_MIXNORMAL) {
				vol = max(vol, v2->envright);
			}
			if (volmin > vol) {
				volmin = vol;
				v = v2;
			}
		}
	} while(v1 < v2);

	if (v != NULL) {
		voice_setfree(v);
		voice_on(midi, ch, v, key, vel);
	}
}

static void key_off(MIDIHDL midi, CHANNEL ch, int key) {

	VOICE	v;
	VOICE	vterm;

	v = midi->voice;
	vterm = v + VOICE_MAX;
	do {
		if ((v->phase & VOICE_ON) &&
			(v->channel == ch) && (v->note == key)) {
			if (ch->flag & CHANNEL_SUSTAIN) {
				voice_setphase(v, VOICE_SUSTAIN);
			}
			else {
				voice_off(v);
			}
			return;
		}
		v++;
	} while(v < vterm);
}

static void key_pressure(MIDIHDL midi, CHANNEL ch, int key, int vel) {

	VOICE	v;
	VOICE	vterm;

	v = midi->voice;
	vterm = v + VOICE_MAX;
	do {
		if ((v->phase & VOICE_ON) &&
			(v->channel == ch) && (v->note == key)) {
			v->velocity = vel;
			voice_volupdate(v);
			envelope_updates(v);
			break;
		}
		v++;
	} while(v < vterm);
}


// ---- control

static void volumeupdate(MIDIHDL midi, CHANNEL ch) {

	VOICE	v;
	VOICE	vterm;

#if defined(VOLUME_ACURVE)
	ch->level = (midi->level * acurve[ch->volume] * ch->expression) >> 15;
#else
	ch->level = (midi->level * ch->volume * ch->expression) >> 14;
#endif
	v = midi->voice;
	vterm = v + VOICE_MAX;
	do {
		if ((v->phase & (VOICE_ON | VOICE_SUSTAIN)) && (v->channel == ch)) {
			voice_volupdate(v);
			envelope_updates(v);
		}
		v++;
	} while(v < vterm);
}

static void pedaloff(MIDIHDL midi, CHANNEL ch) {

	VOICE	v;
	VOICE	vterm;

	v = midi->voice;
	vterm = v + VOICE_MAX;
	do {
		if ((v->phase & VOICE_SUSTAIN) && (v->channel == ch)) {
			voice_off(v);
		}
		v++;
	} while(v < vterm);
}

static void allsoundsoff(MIDIHDL midi, CHANNEL ch) {

	VOICE	v;
	VOICE	vterm;

	v = midi->voice;
	vterm = v + VOICE_MAX;
	do {
		if ((v->phase != VOICE_FREE) && (v->channel == ch)) {
			voice_setphase(v, VOICE_REL);
			voice_setmix(v);
		}
		v++;
	} while(v < vterm);
}

static void resetallcontrollers(CHANNEL ch) {

	ch->flag &= CHANNEL_MASK;
	if (ch->flag == 9) {
		ch->flag |= CHANNEL_RHYTHM;
	}
	ch->volume = 90;
	ch->expression = 127;
	ch->pitchbend = 0x2000;
	ch->pitchfactor = 1.0;
}

static void allnotesoff(MIDIHDL midi, CHANNEL ch) {

	VOICE	v;
	VOICE	vterm;

	v = midi->voice;
	vterm = v + VOICE_MAX;
	do {
#if 1
		if ((v->phase & (VOICE_ON | VOICE_SUSTAIN)) && (v->channel == ch)) {
			voice_off(v);
		}
#else
		if ((v->phase & VOICE_ON) && (v->channel == ch)) {
			if (ch->flag & CHANNEL_SUSTAIN) {
				voice_setphase(v, VOICE_SUSTAIN);
			}
			else {
				voice_off(v);
			}
		}
#endif
		v++;
	} while(v < vterm);
}

static void ctrlchange(MIDIHDL midi, CHANNEL ch, int ctrl, int val) {

	val &= 0x7f;
	switch(ctrl & 0x7f) {
#if !defined(MIDI_GMONLY)
		case CTRL_PGBANK:
#if defined(ENABLE_GSRX)
			if (!(ch->gsrx[2] & GSRX2_BANKSELECT)) {
				break;
			}
#endif
			ch->bank = val;
			break;
#endif

		case CTRL_DATA_M:
//			TRACEOUT(("data: %x %x %x", c->rpn_l, c->rpn_m, val));
			if ((ch->rpn_l == 0) && (ch->rpn_m == 0)) {
				if (val >= 24) {
					val = 24;
				}
				ch->pitchsens = val;
			}
			break;

		case CTRL_VOLUME:
			ch->volume = val;
			volumeupdate(midi, ch);
			break;

		case CTRL_PANPOT:
			ch->panpot = val;
			break;

		case CTRL_EXPRESS:
			ch->expression = val;
			volumeupdate(midi, ch);
			break;

		case CTRL_PEDAL:
			if (val == 0) {
				ch->flag &= ~CHANNEL_SUSTAIN;
				pedaloff(midi, ch);
			}
			else {
				ch->flag |= CHANNEL_SUSTAIN;
			}
			break;

		case CTRL_RPN_L:
			ch->rpn_l = val;
			break;

		case CTRL_RPN_M:
			ch->rpn_m = val;
			break;

		case CTRL_SOUNDOFF:
			allsoundsoff(midi, ch);
			break;

		case CTRL_RESETCTRL:
			resetallcontrollers(ch);
			break;

		case CTRL_NOTEOFF:
			allnotesoff(midi, ch);
			break;

		case CTRL_MONOON:
			ch->flag |= CHANNEL_MONO;
			break;

		case CTRL_POLYON:
			ch->flag &= ~CHANNEL_MONO;
			break;

		default:
//			TRACEOUT(("ctrl: %x %x", ctrl, val);
			break;
	}
}

static void progchange(MIDIHDL midi, CHANNEL ch, int val) {

#if !defined(MIDI_GMONLY)
	MIDIMOD		module;
	INSTRUMENT	*bank;
	INSTRUMENT	inst;

	module = midi->module;
	inst = NULL;
	if (ch->bank < MIDI_BANKS) {
		bank = module->tone[ch->bank * 2];
		if (bank) {
			inst = bank[val];
		}
	}
	if (inst == NULL) {
		bank = midi->bank0[0];
		inst = bank[val];
	}
	ch->inst = inst;

	bank = NULL;
	if (ch->bank < MIDI_BANKS) {
		bank = module->tone[ch->bank * 2 + 1];
	}
	if (bank == NULL) {
		bank = midi->bank0[1];
	}
	ch->rhythm = bank;
#else
	ch->inst = midi->bank0[0][val];
#endif
	ch->program = val;
}

static void chpressure(MIDIHDL midi, CHANNEL ch, int vel) {

	VOICE	v;
	VOICE	vterm;

	v = midi->voice;
	vterm = v + VOICE_MAX;
	do {
		if ((v->phase & VOICE_ON) && (v->channel == ch)) {
			v->velocity = vel;
			voice_volupdate(v);
			envelope_updates(v);
			break;
		}
		v++;
	} while(v < vterm);
}

static void pitchbendor(MIDIHDL midi, CHANNEL ch, int val1, int val2) {

	VOICE	v;
	VOICE	vterm;

	val1 &= 0x7f;
	val1 += (val2 & 0x7f) << 7;
	if (1) {
		ch->pitchbend = val1;
		val1 -= 0x2000;
		if (!val1) {
			ch->pitchfactor = 1.0;
		}
		else {
			val1 *= ch->pitchsens;
			ch->pitchfactor = bendhtbl[(val1 >> (6 + 7)) + 24] *
												bendltbl[(val1 >> 7) & 0x3f];
		}
		v = midi->voice;
		vterm = v + VOICE_MAX;
		do {
			if ((v->phase != VOICE_FREE) && (v->channel == ch)) {
				freq_update(v);
			}
			v++;
		} while(v < vterm);
	}
}

static void allvolupdate(MIDIHDL midi) {

	int		level;
	CHANNEL	ch;
	CHANNEL	chterm;
	VOICE	v;
	VOICE	vterm;

	level = gaintbl[midi->gain + 16] >> 1;
	level *= midi->master;
	midi->level = level;
	ch = midi->channel;
	chterm = ch + 16;
	do {
		ch->level = (level * ch->volume * ch->expression) >> 14;
		ch++;
	} while(ch < chterm);
	v = midi->voice;
	vterm = v + VOICE_MAX;
	do {
		if (v->phase & (VOICE_ON | VOICE_SUSTAIN)) {
			voice_volupdate(v);
			envelope_updates(v);
		}
		v++;
	} while(v < vterm);
}

#if defined(ENABLE_GSRX)
static void allresetmidi(MIDIHDL midi, BOOL gs)
#else
#define allresetmidi(m, g)		_allresetmidi(m)
static void _allresetmidi(MIDIHDL midi)
#endif
{
	CHANNEL	ch;
	CHANNEL	chterm;
	UINT	flag;

	midi->master = 127;
	ch = midi->channel;
	chterm = ch + 16;
	ZeroMemory(ch, sizeof(_CHANNEL) * 16);
	flag = 0;
	do {
		ch->flag = flag++;
		ch->pitchsens = 2;
#if !defined(MIDI_GMONLY)
		ch->bank = 0;
#endif
		ch->panpot = 64;
		progchange(midi, ch, 0);
		resetallcontrollers(ch);
#if defined(ENABLE_GSRX)
		ch->keyshift = 0x40;
		ch->noterange[0] = 0x00;
		ch->noterange[1] = 0x7f;
		if (gs) {
			ch->gsrx[0] = 0xff;
			ch->gsrx[1] = 0xff;
			ch->gsrx[2] = 0xff;
		}
		else {
			ch->gsrx[0] = 0x7f;
			ch->gsrx[1] = 0xff;
			ch->gsrx[2] = 0x02;
		}
#endif
		ch++;
	} while(ch < chterm);
	allresetvoices(midi);
	allvolupdate(midi);
}


// ----

UINT midiout_getver(OEMCHAR *string, int leng) {

	milstr_ncpy(string, vermouthver, leng);
	return((MIDIOUT_VERSION << 8) | 0x00);
}

MIDIHDL midiout_create(MIDIMOD module, UINT worksize) {

	UINT	size;
	MIDIHDL	ret;

	if (module == NULL) {
		return(NULL);
	}
	worksize = min(worksize, 512);
	worksize = max(worksize, 16384);
	size = sizeof(_MIDIHDL);
	size += sizeof(SINT32) * 2 * worksize;
	size += sizeof(_SAMPLE) * worksize;
	ret = (MIDIHDL)_MALLOC(size, "MIDIHDL");
	if (ret) {
		ZeroMemory(ret, size);
		ret->samprate = module->samprate;
		ret->worksize = worksize;
		ret->module = module;
	//	ret->master = 127;
		ret->bank0[0] = module->tone[0];
		ret->bank0[1] = module->tone[1];
		ret->sampbuf = (SINT32 *)(ret + 1);
		ret->resampbuf = (SAMPLE)(ret->sampbuf + worksize * 2);
		allresetmidi(ret, FALSE);
	}
	return(ret);
}

void midiout_destroy(MIDIHDL hdl) {

	if (hdl) {
		_MFREE(hdl);
	}
}

void midiout_shortmsg(MIDIHDL hdl, UINT32 msg) {

	UINT	cmd;
	CHANNEL	ch;

	if (hdl == NULL) {
		return;
	}
	cmd = msg & 0xff;
	if (cmd & 0x80) {
		hdl->status = cmd;
	}
	else {
		msg <<= 8;
		msg += hdl->status;
	}
	ch = hdl->channel + (cmd & 0x0f);
	switch((cmd >> 4) & 7) {
		case (MIDI_NOTE_OFF >> 4) & 7:
			key_off(hdl, ch, (msg >> 8) & 0x7f);
			break;

		case (MIDI_NOTE_ON >> 4) & 7:
			if (msg & (0x7f << 16)) {
				key_on(hdl, ch, (msg >> 8) & 0x7f, (msg >> 16) & 0x7f);
			}
			else {
				key_off(hdl, ch, (msg >> 8) & 0x7f);
			}
			break;

		case (MIDI_KEYPRESS >> 4) & 7:
			key_pressure(hdl, ch, (msg >> 8) & 0x7f, (msg >> 16) & 0x7f);
			break;

		case (MIDI_CTRLCHANGE >> 4) & 7:
			ctrlchange(hdl, ch, (msg >> 8) & 0x7f, (msg >> 16) & 0x7f);
			break;

		case (MIDI_PROGCHANGE >> 4) & 7:
			progchange(hdl, ch, (msg >> 8) & 0x7f);
			break;

		case (MIDI_CHPRESS >> 4) & 7:
			chpressure(hdl, ch, (msg >> 8) & 0x7f);
			break;

		case (MIDI_PITCHBEND >> 4) & 7:
			pitchbendor(hdl, ch, (msg >> 8) & 0x7f, (msg >> 16) & 0x7f);
			break;
	}
}

static void longmsg_gm(MIDIHDL hdl, const UINT8 *msg, UINT size) {

	if ((size > 5) && (msg[2] == 0x7f) && (msg[3] == 0x09)) {
		allresetmidi(hdl, FALSE);					// GM reset
	}
}

static void longmsg_roland(MIDIHDL hdl, const UINT8 *msg, UINT size) {

	UINT	addr;
	UINT8	data;
	UINT	part;
	CHANNEL	ch;
#if defined(ENABLE_GSRX)
	UINT8	bit;
#endif

	if (size <= 10) {
		return;
	}
	// GS data set
	if ((msg[2] != 0x10) || (msg[3] != 0x42) || (msg[4] != 0x12)) {
		return;
	}
	addr = (msg[5] << 16) + (msg[6] << 8) + msg[7];
	msg += 8;
	size -= 10;
	while(size) {
		size--;
		data = (*msg++) & 0x7f;
		if ((addr & (~0x400000)) == 0x7f) {			// GS reset
			allresetmidi(hdl, TRUE);
			TRACEOUT(("GS-Reset"));
		}
		else if (addr == 0x400004) {				// Vol
			hdl->master = data;
			allvolupdate(hdl);
		}
		else if ((addr & (~(0x000fff))) == 0x401000) {	// GS CH
			part = (addr >> 8) & 0x0f;
			if (part == 0) {						// part10
				part = 9;
			}
			else if (part < 10) {					// part1-9
				part--;
			}
			ch = hdl->channel + part;
			switch(addr & 0xff) {
#if !defined(MIDI_GMONLY)
				case 0x00:							// TONE NUMBER
					ch->bank = data;
					break;
#endif

				case 0x01:							// PROGRAM NUMBER
					progchange(hdl, ch, data);
					break;

#if defined(ENABLE_GSRX)
				case 0x03:							// Rx.PITCHBEND
				case 0x04:							// Rx.CH PRESSURE
				case 0x05:							// Rx.PROGRAM CHANGE
				case 0x06:							// Rx.CONTROL CHANGE
				case 0x07:							// Rx.POLY PRESSURE
				case 0x08:							// Rx.NOTE MESSAGE
				case 0x09:							// Rx.PRN
				case 0x0a:							// Rx.NRPN
					bit = 1 << ((addr - 0x03) & 7);
					if (data == 0) {
						ch->gsrx[0] = ch->gsrx[0] & (~bit);
					}
					else if (data == 1) {
						ch->gsrx[0] = ch->gsrx[0] | bit;
					}
					break;

				case 0x0b:							// Rx.MODULATION
				case 0x0c:							// Rx.VOLUME
				case 0x0d:							// Rx.PANPOT
				case 0x0e:							// Rx.EXPRESSION
				case 0x0f:							// Rx.HOLD1
				case 0x10:							// Rx.PORTAMENTO
				case 0x11:							// Rx.SOSTENUTO
				case 0x12:							// Rx.SOFT
					bit = 1 << ((addr - 0x0b) & 7);
					if (data == 0) {
						ch->gsrx[1] = ch->gsrx[1] & (~bit);
					}
					else if (data == 1) {
						ch->gsrx[1] = ch->gsrx[1] | bit;
					}
					break;
#endif
				case 0x15:							// USE FOR RHYTHM PART
					if (data == 0) {
						ch->flag &= ~CHANNEL_RHYTHM;
						TRACEOUT(("ch%d - tone", part + 1));
					}
					else if ((data == 1) || (data == 2)) {
						ch->flag |= CHANNEL_RHYTHM;
						TRACEOUT(("ch%d - rhythm", part + 1));
					}
					break;

#if defined(ENABLE_GSRX)
				case 0x16:							// PITCH KEY SHIFT
					if ((data >= 0x28) && (data <= 0x58)) {
						ch->keyshift = data;
					}
					break;

				case 0x1d:							// KEYBOARD RANGE LOW
					ch->noterange[0] = data;
					break;

				case 0x1e:							// KEYBOARD RANGE HIGH
					ch->noterange[1] = data;
					break;

				case 0x23:							// Rx.BANK SELECT
				case 0x24:							// Rx.BANK SELECT LSB
					bit = 1 << ((addr - 0x23) & 7);
					if (data == 0) {
						ch->gsrx[2] = ch->gsrx[2] & (~bit);
					}
					else if (data == 1) {
						ch->gsrx[2] = ch->gsrx[2] | bit;
					}
					break;
#endif
				default:
					TRACEOUT(("Roland GS - %.6x", addr));
					break;
			}
		}
		else {
			TRACEOUT(("Roland GS - %.6x", addr));
		}
		addr++;
	}
}

void midiout_longmsg(MIDIHDL hdl, const UINT8 *msg, UINT size) {

	UINT	id;

	if ((hdl == NULL) || (msg == NULL)) {
		return;
	}
	if (size > 3) {							// (msg[size - 1] == 0xf7)
		id = msg[1];
		if (id == 0x7e) {					// GM
			longmsg_gm(hdl, msg, size);
		}
		else if (id == 0x41) {				// Roland
			longmsg_roland(hdl, msg, size);
		}
	}
}

const SINT32 *midiout_get(MIDIHDL hdl, UINT *samples) {

	UINT	size;
	VOICE	v;
	VOICE	vterm;
	BOOL	playing;
	SINT32	*buf;
	SAMPLE	src;
	SAMPLE	srcterm;
	UINT	cnt;
	UINT	pos;
	UINT	rem;

	if ((hdl == NULL) || (samples == NULL)) {
		goto moget_err;
	}
	size = min(*samples, hdl->worksize);
	if (size == 0) {
		goto moget_err;
	}
	buf = hdl->sampbuf;
	ZeroMemory(buf, size * 2 * sizeof(SINT32));
	v = hdl->voice;
	vterm = v + VOICE_MAX;
	playing = FALSE;
	do {
		if (v->phase != VOICE_FREE) {
			cnt = size;
			if (v->phase & VOICE_REL) {
				voice_setfree(v);
				if (cnt > REL_COUNT) {
					cnt = REL_COUNT;
				}
			}
			if (v->flag & VOICE_FIXPITCH) {
				pos = v->samppos >> FREQ_SHIFT;
				src = v->sample->data + pos;
				rem = (v->sample->datasize >> FREQ_SHIFT) - pos;
				if (cnt < rem) {
					v->samppos += cnt << FREQ_SHIFT;
					srcterm = src + cnt;
				}
				else {
					voice_setfree(v);
					srcterm = src + rem;
				}
			}
			else {
				src = hdl->resampbuf;
				srcterm = v->resamp(v, src, src + cnt);
			}
			if (src != srcterm) {
				v->mix(v, buf, src, srcterm);
			}
			playing = TRUE;
		}
		v++;
	} while(v < vterm);

	if (playing) {
		*samples = size;
		pos = 0;
		do {
			buf[pos*2+0] >>= (SAMP_SHIFT + 1);
			buf[pos*2+1] >>= (SAMP_SHIFT + 1);
		} while(++pos < size);
		return(buf);
	}

moget_err:
	return(NULL);
}

UINT midiout_get32(MIDIHDL hdl, SINT32 *pcm, UINT size) {

	UINT	step;
	VOICE	v;
	VOICE	vterm;
	SINT32	*buf;
	SAMPLE	src;
	SAMPLE	srcterm;
	UINT	cnt;
	UINT	pos;
	UINT	rem;

	if ((hdl != NULL) && (size)) {
		do {
			step = min(size, hdl->worksize);
			size -= step;
			buf = hdl->sampbuf;
			ZeroMemory(buf, step * 2 * sizeof(SINT32));
			v = hdl->voice;
			vterm = v + VOICE_MAX;
			do {
				if (v->phase != VOICE_FREE) {
					cnt = step;
					if (v->phase & VOICE_REL) {
						voice_setfree(v);
						if (cnt > REL_COUNT) {
							cnt = REL_COUNT;
						}
					}
					if (v->flag & VOICE_FIXPITCH) {
						pos = v->samppos >> FREQ_SHIFT;
						src = v->sample->data + pos;
						rem = (v->sample->datasize >> FREQ_SHIFT) - pos;
						if (cnt < rem) {
							v->samppos += cnt << FREQ_SHIFT;
							srcterm = src + cnt;
						}
						else {
							voice_setfree(v);
							srcterm = src + rem;
						}
					}
					else {
						src = hdl->resampbuf;
						srcterm = v->resamp(v, src, src + cnt);
					}
					if (src != srcterm) {
						v->mix(v, buf, src, srcterm);
					}
				}
				v++;
			} while(v < vterm);
			do {
				pcm[0] += buf[0] >> (SAMP_SHIFT + 1);
				pcm[1] += buf[1] >> (SAMP_SHIFT + 1);
				buf += 2;
				pcm += 2;
			} while(--step);
		} while(size);
	}
	return(0);
}

void midiout_setgain(MIDIHDL hdl, int gain) {

	if (hdl) {
		if (gain < -16) {
			gain = 16;
		}
		else if (gain > 8) {
			gain = 8;
		}
		hdl->gain = (SINT8)gain;
		allvolupdate(hdl);
	}
}

