
#if defined(SUPPORT_PC9821)

// ---- plasma display

// vram off
static void SCRNCALL SDSYM(pex_0)(SDRAW sdraw, int maxy) {

	UINT8	*p;
	int		y;
	int		x;

	p = sdraw->dst;
	y = sdraw->y;
	do {
		if (sdraw->dirty[y]) {
			for (x=0; x<sdraw->width; x++) {
#ifdef PSP
				SDSETPIXEL(x, p, NP2PAL_TEXTEX);
#else
				SDSETPIXEL(p, NP2PAL_TEXTEX);
#endif
				p += sdraw->xalign;
			}
			p -= sdraw->xbytes;
		}
		p += sdraw->yalign;
	} while(++y < maxy);

	sdraw->dst = p;
	sdraw->y = y;
}

// text 1プレーン
static void SCRNCALL SDSYM(pex_t)(SDRAW sdraw, int maxy) {

const UINT8	*p;
	UINT8	*q;
	int		y;
	int		x;

	p = sdraw->src;
	q = sdraw->dst;
	y = sdraw->y;
	do {
		if (sdraw->dirty[y]) {
			for (x=0; x<sdraw->width; x++) {
#ifdef PSP
				SDSETPIXEL(x, q, (p[x] >> 4) + NP2PAL_TEXTEX);
#else
				SDSETPIXEL(q, (p[x] >> 4) + NP2PAL_TEXTEX);
#endif
				q += sdraw->xalign;
			}
			q -= sdraw->xbytes;
		}
		p += SURFACE_WIDTH;
		q += sdraw->yalign;
	} while(++y < maxy);

	sdraw->src = p;
	sdraw->dst = q;
	sdraw->y = y;
}

// grph 1プレーン
static void SCRNCALL SDSYM(pex_g)(SDRAW sdraw, int maxy) {

const UINT8	*p;
	UINT8	*q;
	int		y;
	int		x;

	p = sdraw->src;
	q = sdraw->dst;
	y = sdraw->y;
	do {
		if (sdraw->dirty[y]) {
			for (x=0; x<sdraw->width; x++) {
#ifdef PSP
				SDSETPIXEL(x, q, p[x] + NP2PAL_GRPHEX);
#else
				SDSETPIXEL(q, p[x] + NP2PAL_GRPHEX);
#endif
				q += sdraw->xalign;
			}
			q -= sdraw->xbytes;
		}
		p += SURFACE_WIDTH;
		q += sdraw->yalign;
	} while(++y < maxy);

	sdraw->src = p;
	sdraw->dst = q;
	sdraw->y = y;
}

// text + grph
static void SCRNCALL SDSYM(pex_2)(SDRAW sdraw, int maxy) {

const UINT8	*p;
const UINT8	*q;
	UINT8	*r;
	int		y;
	int		x;
	int		c;

	p = sdraw->src;
	q = sdraw->src2;
	r = sdraw->dst;
	y = sdraw->y;
	do {
		if (sdraw->dirty[y]) {
			for (x=0; x<sdraw->width; x++) {
				c = q[x];
				if (c != 0) {
					c = (c >> 4) + NP2PAL_TEXTEX;
				}
				else {
					c = p[x] + NP2PAL_GRPHEX;
				}
#ifdef PSP
				SDSETPIXEL(x, r, c);
#else
				SDSETPIXEL(r, c);
#endif
				r += sdraw->xalign;
			}
			r -= sdraw->xbytes;
		}
		p += SURFACE_WIDTH;
		q += SURFACE_WIDTH;
		r += sdraw->yalign;
	} while(++y < maxy);

	sdraw->src = p;
	sdraw->src2 = q;
	sdraw->dst = r;
	sdraw->y = y;
}


static const SDRAWFN SDSYM(pex)[] = {
		SDSYM(pex_0),	SDSYM(pex_t),	SDSYM(pex_g),	SDSYM(pex_2),
	};


// ---- normal display

#ifdef SUPPORT_NORMALDISP

// vram off
static void SCRNCALL SDSYM(nex_0)(SDRAW sdraw, int maxy) {

	UINT8	*p;
	int		y;
	int		x;

	p = sdraw->dst;
	y = sdraw->y;
	do {
		if (sdraw->dirty[y]) {
#ifdef PSP
			SDSETPIXEL(x, p, NP2PAL_TEXTEX3);
#else
			SDSETPIXEL(p, NP2PAL_TEXTEX3);
#endif
			for (x=0; x<sdraw->width; x++) {
				p += sdraw->xalign;
#ifdef PSP
				SDSETPIXEL(x, p, NP2PAL_TEXTEX);
#else
				SDSETPIXEL(p, NP2PAL_TEXTEX);
#endif
			}
			p -= sdraw->xbytes;
		}
		p += sdraw->yalign;
	} while(++y < maxy);

	sdraw->dst = p;
	sdraw->y = y;
}

// text 1プレーン
static void SCRNCALL SDSYM(nex_t)(SDRAW sdraw, int maxy) {

const UINT8	*p;
	UINT8	*q;
	int		y;
	int		x;

	p = sdraw->src;
	q = sdraw->dst;
	y = sdraw->y;
	do {
		if (sdraw->dirty[y]) {
#ifdef PSP
			SDSETPIXEL(x, q, (p[0] >> 4) + NP2PAL_TEXTEX3);
#else
			SDSETPIXEL(q, (p[0] >> 4) + NP2PAL_TEXTEX3);
#endif
			q += sdraw->xalign;
			for (x=1; x<sdraw->width; x++) {
#ifdef PSP
				SDSETPIXEL(x, q, (p[x] >> 4) + NP2PAL_TEXTEX);
#else
				SDSETPIXEL(q, (p[x] >> 4) + NP2PAL_TEXTEX);
#endif
				q += sdraw->xalign;
			}
#ifdef PSP
			SDSETPIXEL(x, q, NP2PAL_TEXTEX);
#else
			SDSETPIXEL(q, NP2PAL_TEXTEX);
#endif
			q -= sdraw->xbytes;
		}
		p += SURFACE_WIDTH;
		q += sdraw->yalign;
	} while(++y < maxy);

	sdraw->src = p;
	sdraw->dst = q;
	sdraw->y = y;
}

// grph 1プレーン
static void SCRNCALL SDSYM(nex_g)(SDRAW sdraw, int maxy) {

const UINT8	*p;
	UINT8	*q;
	int		y;
	int		x;

	p = sdraw->src;
	q = sdraw->dst;
	y = sdraw->y;
	do {
		if (sdraw->dirty[y]) {
#ifdef PSP
			SDSETPIXEL(x, q, NP2PAL_TEXTEX3);
#else
			SDSETPIXEL(q, NP2PAL_TEXTEX3);
#endif
			for (x=0; x<sdraw->width; x++) {
				q += sdraw->xalign;
#ifdef PSP
				SDSETPIXEL(x, q, p[x] + NP2PAL_GRPHEX);
#else
				SDSETPIXEL(q, p[x] + NP2PAL_GRPHEX);
#endif
			}
			q -= sdraw->xbytes;
		}
		p += SURFACE_WIDTH;
		q += sdraw->yalign;
	} while(++y < maxy);

	sdraw->src = p;
	sdraw->dst = q;
	sdraw->y = y;
}

// text + grph
static void SCRNCALL SDSYM(nex_2)(SDRAW sdraw, int maxy) {

const UINT8	*p;
const UINT8	*q;
	UINT8	*r;
	int		y;
	int		x;
	int		c;

	p = sdraw->src;
	q = sdraw->src2;
	r = sdraw->dst;
	y = sdraw->y;
	do {
		if (sdraw->dirty[y]) {
#ifdef PSP
			SDSETPIXEL(x, r, (q[0] >> 4) + NP2PAL_TEXT3);
#else
			SDSETPIXEL(r, (q[0] >> 4) + NP2PAL_TEXT3);
#endif
			r += sdraw->xalign;
			for (x=1; x<sdraw->width; x++) {
				c = q[x];
				if (c) {
					c = (c >> 4) + NP2PAL_TEXTEX;
				}
				else {
					c = p[x-1] + NP2PAL_GRPHEX;
				}
#ifdef PSP
				SDSETPIXEL(x, r, c);
#else
				SDSETPIXEL(r, c);
#endif
				r += sdraw->xalign;
			}
#ifdef PSP
			SDSETPIXEL(x, r, p[x-1] + NP2PAL_GRPHEX);
#else
			SDSETPIXEL(r, p[x-1] + NP2PAL_GRPHEX);
#endif
			r -= sdraw->xbytes;
		}
		p += SURFACE_WIDTH;
		q += SURFACE_WIDTH;
		r += sdraw->yalign;
	} while(++y < maxy);

	sdraw->src = p;
	sdraw->src2 = q;
	sdraw->dst = r;
	sdraw->y = y;
}

static const SDRAWFN SDSYM(nex)[] = {
		SDSYM(nex_0),	SDSYM(nex_t),	SDSYM(nex_g),	SDSYM(nex_2)
	};
#endif

#endif

