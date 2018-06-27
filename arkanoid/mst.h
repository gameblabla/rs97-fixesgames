

// Structure commune à tous les monstres.
#define MST_COMMON_DATA_SZ  64
struct SMstCommon
{
	u8	nUsed;			// 0 = slot vide, 1 = slot occupé.
	u8	nMstNo;			// No du monstre.

	void (*pFctInit) (struct SMstCommon *pMst);	// Fct d'init du monstre.
	s32 (*pFctMain) (struct SMstCommon *pMst);		// Fct principale du monstre.

	s32	nPosX, nPosY;
	s32	nSpd;
	u8	nAngle;
	s32	nAnm;			// Anim.
	u8	nPhase;

	u8	pData[MST_COMMON_DATA_SZ];	// On fera pointer les structures spécifiques ici.

};

extern	u32	gnMstPrio;		// Pour priorité de l'affichage.
#define	MSTPRIO_AND	31

// Prototypes.
void MstInitEngine(void);
void MstManage(void);
s32 MstAdd(u32 nMstNo, s32 nPosX, s32 nPosY);

u32 MstCheckRectangle(s32 nXMin, s32 nXMax, s32 nYMin, s32 nYMax);


