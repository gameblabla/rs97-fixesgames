
#include "includes.h"


// itoa.
void MyItoA(s32 nNb, char *pDst)
{
	char	cMin = ' ';
	char	*pPtr;
	u32	nTmp;

	// Cas des nombres négatifs.
	if (nNb < 0)
	{
		cMin = '-';
		nNb = -nNb;
	}

	pPtr = pDst + strlen(pDst) - 1;
	nTmp = nNb;
	do
	{
		*pPtr-- = (char)((nTmp % 10) + '0');
	} while (pPtr >= pDst && (nTmp /= 10) > 0);

	// Négatifs.
	if (cMin != ' ' && pPtr >= pDst) *pPtr = cMin;

}


// Affichage d'une phrase en sprites.
// Renvoie la largeur en pixels de la phrase.
u32 Font_Print(s32 nPosX, s32 nPosY, char *pStr, u32 nFlags)
{
	char	cChr;
	struct SSprite	*pSpr;
	s32	nPosXOrg = nPosX;

	while (*pStr)
	{
		cChr = *pStr++;
		cChr -= ' ';
		if (cChr != 0)
		{
			// Char normal.
			cChr--;
			pSpr = SprGetDesc(e_Spr_FontSmall + cChr);
			if ((nFlags & FONT_NoDisp) == 0) SprDisplay(e_Spr_FontSmall + cChr, nPosX, nPosY, e_Prio_HUD);
		}
		else
		{
			// Espace, on avance de la taille d'un 'I'.
			pSpr = SprGetDesc(e_Spr_FontSmall + 'I' - ' ');
		}
		nPosX += pSpr->nLg + 1;
	}

	return ((u32)(nPosX - nPosXOrg));

}



