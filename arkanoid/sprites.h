

#define	SPR_Flag_Shadow		(1 << 31)


// Structures.
struct SSprite
{
	u32	nPtRefX, nPtRefY;	// Points de ref.
	u32	nLg, nHt;			// Largeur et hauteur du sprite.
	u8	*pGfx;
	u8	*pMask;

};

// Prototypes.
void SprInitEngine(void);
void SprRelease(void);
void SprLoadBMP(char *pFilename, SDL_Color *pSprPal, u32 nPalIdx);
void SprDisplayLock(u32 nSprNo, s32 nPosX, s32 nPosY);
void SprDisplay(u32 nSprNo, s32 nPosX, s32 nPosY, u32 nPrio);
void SprDisplayAll(void);
struct SSprite *SprGetDesc(u32 nSprNo);
u32 SprCheckColBox(u32 nSpr1, s32 nPosX1, s32 nPosY1, u32 nSpr2, s32 nPosX2, s32 nPosY2);

extern	u32	gnSprSto;


// Define priorités.
enum
{
	e_Prio_Ombres = 0,
	e_Prio_Briques = 10,		// Pillules, prio des briques + 1 + prio mst [0 ; MSTPRIO-AND]
	e_Prio_Dust = 50,
	e_Prio_Monstres = 60,		// Monstres, prio + prio mst [0 ; MSTPRIO-AND]
	e_Prio_Tirs = 100,
	e_Prio_Raquette = 110,
	e_Prio_Balles = 120,
	e_Prio_HUD = 200,

};

#define	SPR_NoSprite	((u32)-2)		// Sprite qui n'affiche rien.

// Define sprites.
enum
{
	e_Spr_Bricks = 0,
	e_Spr_BricksSpe = e_Spr_Bricks + 10,
	e_Spr_BricksExplo = e_Spr_BricksSpe + 3,
	e_Spr_Bricks2HitExplo = e_Spr_BricksExplo + 10,
	e_Spr_BricksCBExplo = e_Spr_Bricks2HitExplo + 10,
	e_Spr_BrickIndesHit = e_Spr_BricksCBExplo + 10,
	e_Spr_Brick2Hit = e_Spr_BrickIndesHit + 6,
	e_Spr_BrickCBHit = e_Spr_Brick2Hit + 6,
	e_Spr_RaqClignG = e_Spr_BrickCBHit + 6,
	e_Spr_RaqClignD = e_Spr_RaqClignG + 5,
	e_Spr_Raquette = e_Spr_RaqClignD + 5,
	e_Spr_HUDRaquette = e_Spr_Raquette + 1,

	e_Spr_RaquetteAimant = e_Spr_HUDRaquette + 1,
	e_Spr_RaquetteMitrG = e_Spr_RaquetteAimant + 3,
	e_Spr_RaquetteMitrD = e_Spr_RaquetteMitrG + 4,
	e_Spr_PlyrShot = e_Spr_RaquetteMitrD + 4,
	e_Spr_RaquetteRallonge0 = e_Spr_PlyrShot + 1,
	e_Spr_RaquetteRallonge1 = e_Spr_RaquetteRallonge0 + 6,
	e_Spr_RaquetteRallonge2 = e_Spr_RaquetteRallonge1 + 6,
	e_Spr_RaquetteApparition = e_Spr_RaquetteRallonge2 + 6,
	e_Spr_RaquetteDeathSz0 = e_Spr_RaquetteApparition + 14,
	e_Spr_RaquetteDeathSz1 = e_Spr_RaquetteDeathSz0 + 8,
	e_Spr_RaquetteDeathSz2 = e_Spr_RaquetteDeathSz1 + 8,
	e_Spr_RaquetteDeathSz3 = e_Spr_RaquetteDeathSz2 + 8,

	e_Spr_Ball = e_Spr_RaquetteDeathSz3 + 8,
	e_Spr_BallTrav = e_Spr_Ball + 4,
	e_Spr_Itm1 = e_Spr_BallTrav + 4,
	e_Spr_Itm2 = e_Spr_Itm1 + 5,
	e_Spr_Itm3 = e_Spr_Itm2 + 5,
	e_Spr_Itm4 = e_Spr_Itm3 + 5,
	e_Spr_Itm5 = e_Spr_Itm4 + 5,
	e_Spr_Itm6 = e_Spr_Itm5 + 5,
	e_Spr_Itm7 = e_Spr_Itm6 + 5,
	e_Spr_Itm8 = e_Spr_Itm7 + 5,
	e_Spr_Itm9 = e_Spr_Itm8 + 5,
	e_Spr_Itm10 = e_Spr_Itm9 + 5,
	e_Spr_Itm11 = e_Spr_Itm10 + 5,
	e_Spr_Itm12 = e_Spr_Itm11 + 5,
	e_Spr_Itm13 = e_Spr_Itm12 + 1,

	e_Spr_Mst1 = e_Spr_Itm13 + 1,
	e_Spr_Mst2 = e_Spr_Mst1 + 8,
	e_Spr_Mst3 = e_Spr_Mst2 + 12,
	e_Spr_Mst4 = e_Spr_Mst3 + 24,
	e_Spr_MstExplo1 = e_Spr_Mst4 + 11,
	e_Spr_MstPorteLevel = e_Spr_MstExplo1 + 7,
	e_Spr_SortieMstCache = e_Spr_MstPorteLevel + 20,
	e_Spr_SortieMst = e_Spr_SortieMstCache + 1,
	e_Spr_CacheDroit = e_Spr_SortieMst + 3,

	e_Spr_DohShoot = e_Spr_CacheDroit + 1,
	e_Spr_DohIdle = e_Spr_DohShoot + 1,
	e_Spr_DohHit = e_Spr_DohIdle + 1,
	e_Spr_DohDisappear = e_Spr_DohHit + 1,
	e_Spr_DohMissile = e_Spr_DohDisappear + 5,
	e_Spr_DohMisDust = e_Spr_DohMissile + 4,

	e_Spr_Logo = e_Spr_DohMisDust + 4,
	e_Spr_Logo8 = e_Spr_Logo + 1,

	e_Spr_BossBar = e_Spr_Logo8 + 8,
	e_Spr_BossBarTop = e_Spr_BossBar + 1,
	e_Spr_BossBarPts = e_Spr_BossBarTop + 1,

	e_Spr_FontSmall = e_Spr_BossBarPts + 8,
	e_Spr_NEXT = e_Spr_FontSmall + 64,

};

