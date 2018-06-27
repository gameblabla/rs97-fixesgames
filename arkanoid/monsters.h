
// Structures.
struct SMstTb
{
	void (*pFctInit) (struct SMstCommon *pMst);
	s32 (*pFctMain) (struct SMstCommon *pMst);
	u32	*pAnm;
	u16	nPoints;
};


// Prototypes.
void MstDoorROpen(void);
u32 MstCheckStructSizes(void);	// Debug.


// Liste des monstres.
enum
{
	e_Mst_Pill_0 = 0,		// Pour base du random.
	e_Mst_Pill_Aimant = e_Mst_Pill_0,
	e_Mst_Pill_Mitrailleuse,
	e_Mst_Pill_BallTraversante,
	e_Mst_Pill_BallBigger,
	e_Mst_Pill_BallX3,
	e_Mst_Pill_RaqRallonge,
	e_Mst_Pill_RaqReduit,
	e_Mst_Pill_1Up,
	e_Mst_Pill_DoorR,
	e_Mst_Pill_SpeedUp,
	e_Mst_Pill_SpeedDown,
	e_Mst_Generateur,		// Generateur d'ennemis.
	e_Mst_Mst1,				// Monstres des niveaux (x & 3).
	e_Mst_DoorR,			// Porte à droite.
	e_Mst_Doh,				// Doh !

};



