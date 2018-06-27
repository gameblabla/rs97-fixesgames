
// Codes de contrôle.
#define	BIT31	(1 << 31)
enum
{
	e_Anm_Jump	= BIT31 | 1,	// Ptr + offset.
	e_Anm_Goto	= BIT31 | 2,	// Initialise une autre anim.
	e_Anm_End	= BIT31 | 3,	// Fin d'une anim. Renvoie SPR_NoSprite, place e_AnmFlag_End, ne libère pas le slot.
	e_Anm_Kill	= BIT31 | 4,	// Fin d'une anim. Renvoie -1 et libère le slot (ex: dust).

};

// Clefs d'anim.	16b = Priorité (à faire) | 16b = No.
#define	ANMPRIO(x)	(x << 16)
enum
{
	e_AnmKey_Null		= 0,
	e_AnmKey_PlyrAppear	= ANMPRIO(1) + 0,
	e_AnmKey_PlyrDeath	= ANMPRIO(2) + 0,

	e_AnmKey_MstDohMoutOpens = ANMPRIO(1) + 0,
	e_AnmKey_MstDohMoutCloses = ANMPRIO(1) + 1,
	e_AnmKey_MstDohAppears = ANMPRIO(1) + 2,
	e_AnmKey_MstDohDisappears = ANMPRIO(3) + 0,
	e_AnmKey_MstDohHit	= ANMPRIO(2) + 0,

};


// Définition des anims.

extern u32	gAnm_Raquette[];
extern u32	gAnm_RaqAppear[];
extern u32	gAnm_RaqAimant[];
extern u32	gAnm_RaqDeath0[];
extern u32	gAnm_RaqDeath1[];
extern u32	gAnm_RaqDeath2[];
extern u32	gAnm_RaqDeath3[];
extern u32	gAnm_RaqRallonge0[];
extern u32	gAnm_RaqReduit0[];
extern u32	gAnm_RaqRallonge1[];
extern u32	gAnm_RaqReduit1[];
extern u32	gAnm_RaqRallonge2[];
extern u32	gAnm_RaqReduit2[];
extern u32	gAnm_RaqMitGRepos[];
extern u32	gAnm_RaqMitDRepos[];
extern u32	gAnm_RaqMitGShoot[];
extern u32	gAnm_RaqMitDShoot[];
extern u32	gAnm_PlyrShot[];
extern u32	gAnm_RaqClignG[];
extern u32	gAnm_RaqClignD[];
extern u32	gAnm_BrickExplo[];
extern u32	gAnm_Brick2HitExplo[];
extern u32	gAnm_BrickCBExplo[];
extern u32	gAnm_Brick2Hit[];
extern u32	gAnm_BrickCBHit[];
extern u32	gAnm_BrickIndesHit[];
extern u32	gAnm_Itm1[];
extern u32	gAnm_Itm2[];
extern u32	gAnm_Itm3[];
extern u32	gAnm_Itm4[];
extern u32	gAnm_Itm5[];
extern u32	gAnm_Itm6[];
extern u32	gAnm_Itm7[];
extern u32	gAnm_Itm8[];
extern u32	gAnm_Itm9[];
extern u32	gAnm_Itm10[];
extern u32	gAnm_Itm11[];
extern u32	gAnm_MstDoorOpen[];
extern u32	gAnm_MstDoorOpened[];
extern u32	gAnm_MstDoorClose[];
extern u32	gAnm_MstDoorWait[];
extern u32	gAnm_Mst1[];
extern u32	gAnm_Mst2[];
extern u32	gAnm_Mst3[];
extern u32	gAnm_Mst4[];
extern u32	gAnm_MstExplo1[];
extern u32	gAnm_DohMissileDisp[];
extern u32	gAnm_MstDoorRight[];
extern u32	gAnm_MstDohIdle[];
extern u32	gAnm_MstDohHit[];
extern u32	gAnm_MstDohShoot[];
extern u32	gAnm_MstDohAppears[];
extern u32	gAnm_MstDohDisappears[];
extern u32	gAnm_MstDohMouthOpens[];
extern u32	gAnm_MstDohMouthCloses[];
extern u32	gAnm_DohMissile[];



