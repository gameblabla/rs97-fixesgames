// Gestion des animations de sprites.

#include "includes.h"


// Flags.
enum
{
	e_AnmFlag_End = (1 << 0),

};


struct SAnim
{
	u8	nUsed;		// 0 = slot vide, 1 = slot occupé.
	u8	nFlags;
	u32	*pOrg;		// Ptr sur le début de l'anim.

	u32	*pAnm;
	u32	nKey;		// Clef d'anim. 16b Priorité | 16b No.
	u32	nFramesCnt;	// Compteur de frames restant pour l'image en cours.
	u32	nCurSpr;	// N° du sprite en cours.

};

#define	ANM_MAX_SLOTS	64
struct SAnim	pAnmSlots[ANM_MAX_SLOTS];
u32	gnAnmLastUsed;

// RAZ moteur.
void AnmInitEngine(void)
{
	u32	i;

	// RAZ de tous les slots.
	for(i = 0; i < ANM_MAX_SLOTS; i++)
	{
		pAnmSlots[i].nUsed = 0;
	}
	gnAnmLastUsed = 0;

}

// Cherche un slot libre.
// Out : N° d'un slot libre. -1 si erreur.
s32 AnmGetSlot(void)
{
	u32	i;

	for (i = gnAnmLastUsed; i < ANM_MAX_SLOTS; i++)
	{
		if (pAnmSlots[i].nUsed == 0)
		{
			gnAnmLastUsed = i + 1;		// La recherche commencera au suivant.
			return (i);
		}
	}
	return (-1);
}

// Libère un slot.
void AnmReleaseSlot(s32 nSlotNo)
{
	if (nSlotNo == -1) return;

	// Pour accélérer la recherche des slots libres.
	if ((u32)nSlotNo < gnAnmLastUsed)
	{
		gnAnmLastUsed = nSlotNo;
	}
	pAnmSlots[nSlotNo].nUsed = 0;

}

// Récupère la clef d'une anim.
u32 AnmGetKey(s32 nSlotNo)
{
	return (pAnmSlots[nSlotNo].nKey);
}

// Teste si l'anim est terminée (e_Anm_End).
// 0 si pas terminée, x si terminée.
u32 AnmCheckEnd(s32 nSlotNo)
{
	return (pAnmSlots[nSlotNo].nFlags & e_AnmFlag_End);
}

// Init une anim si ce n'est pas la même que précédement, et si la priorité est ok.
s32 AnmSetIfNew(u32 *pAnm, s32 nSlotNo)
{

	if (nSlotNo == -1)
	{
		return (AnmSet(pAnm, nSlotNo));
	}
	else if (pAnmSlots[nSlotNo].pOrg != pAnm)
	{
		// Anim différente. On teste la priorité.
		if ((*pAnm) >> 16 >= pAnmSlots[nSlotNo].nKey >> 16)
		{
			return (AnmSet(pAnm, nSlotNo));
		}
	}

	// C'est la même, ou pas la même mais avec une priorité <, on ne réinitialise pas.
	return (nSlotNo);
}

// Init d'une anim.
// Out : N° du slot. -1 si erreur.
s32 AnmSet(u32 *pAnm, s32 nSlotNo)
{

	// Si nSlotNo == -1, on cherche un nouveau slot.
	if (nSlotNo == -1)
	{
		if ((nSlotNo = AnmGetSlot()) == -1) return (-1);
		pAnmSlots[nSlotNo].nUsed = 1;
	}
	pAnmSlots[nSlotNo].nFlags = 0;			// Flags.
	pAnmSlots[nSlotNo].nKey = *pAnm;		// Clef d'anim.
	pAnmSlots[nSlotNo].pOrg = pAnm;			// Ptr sur le début de l'anim.

	// On fait un GetImage pour initialiser le slot.
	pAnmSlots[nSlotNo].pAnm = pAnm - 1;
	pAnmSlots[nSlotNo].nFramesCnt = 1;
	AnmGetImage(nSlotNo);

	return (nSlotNo);
}

// Renvoie l'image en cours et avance l'anim.
s32	AnmGetImage(s32 nSlotNo)
{
//	SSprite	*pSpr;

	// Décrémentation et avancée si nécéssaire.
	if (--pAnmSlots[nSlotNo].nFramesCnt == 0)
	{
		pAnmSlots[nSlotNo].pAnm += 2;		// Avance le ptr sur la suite.
		while (((*pAnmSlots[nSlotNo].pAnm) & BIT31) != 0)
		{
			// Code de contrôle.
			switch (*pAnmSlots[nSlotNo].pAnm)
			{
			case e_Anm_Jump:	// Ajoute un offset au pointeur.
				pAnmSlots[nSlotNo].pAnm += *(s32 *)(pAnmSlots[nSlotNo].pAnm + 1);// * 2;
				break;

			case e_Anm_Goto:	// Fait sauter le pointeur à une autre adresse.
				pAnmSlots[nSlotNo].pAnm = (u32 *)*(pAnmSlots[nSlotNo].pAnm + 1);
				pAnmSlots[nSlotNo].pOrg = pAnmSlots[nSlotNo].pAnm;
				pAnmSlots[nSlotNo].nKey = *(pAnmSlots[nSlotNo].pAnm);		// Clef d'anim.
				pAnmSlots[nSlotNo].pAnm++;
				break;

			case e_Anm_End:		// Fin de l'anim. Place le flag End et renvoie SPR_NoSprite.
				pAnmSlots[nSlotNo].pAnm -= 2;			// Recule le ptr pour repointer sur e_Anm_End au prochain tour.
				pAnmSlots[nSlotNo].nFramesCnt = 1;		// Reset compteur.
				pAnmSlots[nSlotNo].nCurSpr = SPR_NoSprite;	// End => No Sprite.
				pAnmSlots[nSlotNo].nFlags |= e_AnmFlag_End;	// Flag.
				return (pAnmSlots[nSlotNo].nCurSpr);
				break;

			case e_Anm_Kill:	// Fin de l'anim + libération du slot.
				AnmReleaseSlot(nSlotNo);
				return (-1);
				break;

			default:
				printf("Anm: Unknown control code.\n");
				break;
			}

		}
		// Image.
		pAnmSlots[nSlotNo].nFramesCnt = *pAnmSlots[nSlotNo].pAnm;		// Compteur de frames restant pour l'image en cours.
		pAnmSlots[nSlotNo].nCurSpr = *(pAnmSlots[nSlotNo].pAnm + 1);	// N° du sprite en cours.
	}

	return (pAnmSlots[nSlotNo].nCurSpr);	// N° du sprite en cours.

}


// Renvoie la dernière image affichée.
s32	AnmGetLastImage(s32 nSlotNo)
{
	return (pAnmSlots[nSlotNo].nCurSpr);	// N° du sprite en cours.

}

