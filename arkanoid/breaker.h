

// Define.#define	LEVEL_Max	39

#define	LEVEL_SELECT_Max	(LEVEL_Max - 1)
//#define	LEVEL_SELECT_Max	(12 - 1)

#define	PLAYER_Lives_Start	3
#define	PLAYER_Lives_Max	10

#define	TIMER_GameOver	(60 * 5)
#define	TIMER_DisplayLevel	(60 * 3)

#define	BRICK_ComingBackCnt	(5 * 60)

#define	WALL_XMin	(10)
#define	WALL_XMax	(SCR_Width-10)
#define	WALL_YMin	16

#define	BRICK_Width		20
#define	BRICK_Height	10

#define	ABS(x)	((x) < 0 ? -(x) : (x))
#define	SGN(x)	((x) >= 0 ? 1 : -1)
#define	MIN(x,y) ((x) < (y) ? (x) : (y))
#define NBELEM(tab) (sizeof(tab) / sizeof(tab[0]))

#define	TABLE_Width		15
#define	TABLE_Height	17

// Define de la balle

#define	BALL_Speed_Min	0x200
#define	BALL_Speed_Max	0x700 //0x380
#define	BALL_Speed_Step	0x080

// Flags pour la balle.
#define	BALL_Flg_Traversante		(1 << 0)
#define	BALL_Flg_Aimantee			(1 << 1)	// Balle collée sur la raquette.

// Flags pour la raquette.
#define	PLAYER_Flg_Aimant			(1 << 0)
#define	PLAYER_Flg_Mitrailleuse		(1 << 1)
#define	PLAYER_Flg_NoKill			(1 << 2)
#define	PLAYER_Flg_DoorR			(1 << 3)
#define	PLAYER_Flg_BossWait			(1 << 4)

// Flags pour la brique.
#define BRICK_Flg_Indestructible	(1 << 0)
#define	BRICK_Flg_ComingBack		(1 << 1)


// Phases de jeu.
enum
{
	e_Game_SelectLevel = 0,		// Selection du level.
	e_Game_Normal,				// Jeu.
	e_Game_LevelCompleted,		// Niveau terminé.
	e_Game_GameOver,			// Game over.
	e_Game_AllClear,			// Jeu terminé.
	e_Game_Pause,				// Pause.

};


// Structures.
struct SBrique
{
	u8	nPres;		// Brique présente ou pas.
	u8	nCnt;		// Nb de touches restantes avant la destruction.
	u8	nFlags;		// Flags : Voir liste.
	u16	nResetCnt;	// Compteur pour retour de la brique.

	u32	nSprNo;		// Sprite par défaut.
	s32	nAnmNo;		// Remplacé par l'anim si != -1.

	u32	*pAnmHit;	// Anim à utiliser pour le hit.
	u32	*pAnmExplo;	// Anim à utiliser pour la disparition.
	u16	nScore;		// Nb de points rapportés par la brique.

};

#define	BALL_GfxLg	32
#define	BALL_MAX_NB	12
#define	BALL_MAX_SIZE	3		// [0;BALL_MAX_SIZE]
struct SBall
{
	u8	nUsed;			// Slot utilisé ou pas.

	s32	nPosX, nPosY;	// Virgule fixe 8 bits.
	s32	nSpeed;			// Virgule fixe 8 bits.
	u8	nAngle;
	u32	nFlags;
	u32	nSize;

	u32 nRayon;// = 3;	// Offset du centre.
	u32 nDiam;// = 7;	// Hauteur. (Largeur = puissance de 2 à choisir).
	u8 pBallMask[BALL_GfxLg * BALL_GfxLg];	// Masque de la balle, avec une largeur qui nous arrange pour accélérer les tests.
	u32 nSpr;       // Sprite de la balle.

	s32	nOffsRaq;	// Offset sur la raquette (pour aimantation).

};

struct SBreaker
{
	struct SBall	pBalls[BALL_MAX_NB];
	u32	nBallsNb;			// Nb de balles gérés en cours.

	s32	nPlayerPosX, nPlayerPosY;
	//s32	nPlayerLastPosX;
	s32	nPlayerAnmNo;//nPlayerSprNo;	// Sprite de la raquette.
	s32	nPlayerAnmBonusM;				// Bonus central.
	s32	nPlayerAnmBonusG;				// Bonus G.
	s32	nPlayerAnmBonusD;				// Bonus D.
	s32	nPlayerAnmClignG;			// Clignotant gauche.
	s32	nPlayerAnmClignD;			// Clignotant droit.
	u32	nPlayerFlags;
	u32	nPlayerRSize;				// Taille de la raquette.
	u32	nPlayerLives;				// Nb de vies.
	u32	nPlayerScore, nPlayerLastScore;	// Score.

	struct SBrique	pLevel[TABLE_Width * TABLE_Height];	// Les briques.
	u32	nRemainingBricks;			// Nb de briques restantes.
	u32	nBricksComingBackNbCur;		// Nb de briques qui sont en phase disparue, en attente de revenir.
	u32	nBricksComingBackTotal;		// Nb de briques qui reviennent total du niveau.

	u32	nPhase;		// Phase de jeu (init, jeu, game over...).
	u32	nLevel;		// Level en cours.

	u32	nTimerGameOver;		// Countdown pour game over.
	u32	nTimerLevelDisplay;	// Compteur pour affichage du n° de level.

};

extern	struct SBreaker	gBreak;


// Prototypes.
void ExgBrkInit(void);
void BreakerInit(void);
void Breaker(void);

u32 BrickHit(u32 nBx, u32 nBy, u32 nBallFlags);

void BreakerBonusSetAimant(void);
void BreakerBonusSetMitrailleuse(void);
void BreakerBonusRaquetteSize(s32 nSens);
void BreakerBonusBallTraversante(void);
void BreakerBonusBallBigger(void);
void BreakerBonusBallX3(void);
void BreakerBonus1Up(void);
void BreakerBonusSpeedUp(s32 nSens);

void BallsKill(void);

void Balle_Acceleration (struct SBall *pBall,int acceleration);

