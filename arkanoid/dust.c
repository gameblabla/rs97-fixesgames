// Gestion des poussières.

#include "includes.h"


struct SDust
{
	u8	nUsed;		// 0 = slot vide, 1 = slot occupé.

	s32	nAnm;		// Anim.
	s32	nPosX, nPosY;

};

#define	DUST_MAX_SLOTS	64
struct SDust	gpDustSlots[DUST_MAX_SLOTS];
u32	gnDustLastUsed;

// RAZ moteur.
void DustInitEngine(void)
{
	u32	i;

	// RAZ de tous les slots.
	for(i = 0; i < DUST_MAX_SLOTS; i++)
	{
		gpDustSlots[i].nUsed = 0;
	}
	gnDustLastUsed = 0;

}

// Cherche un slot libre.
// Out : N° d'un slot libre. -1 si erreur.
s32 DustGetSlot(void)
{
	u32	i;

	for (i = gnDustLastUsed; i < DUST_MAX_SLOTS; i++)
	{
		if (gpDustSlots[i].nUsed == 0)
		{
			gnDustLastUsed = i + 1;		// La recherche commencera au suivant.
			return (i);
		}
	}
	return (-1);
}

// Libère un slot.
void DustReleaseSlot(u32 nSlotNo)
{
	// Libère l'anim.
	AnmReleaseSlot(gpDustSlots[nSlotNo].nAnm);
	// Pour accélérer la recherche des slots libres.
	if (nSlotNo < gnDustLastUsed)
	{
		gnDustLastUsed = nSlotNo;
	}
	gpDustSlots[nSlotNo].nUsed = 0;

}

// Init d'une anim.
// Out : N° du slot. -1 si erreur.
s32 DustSet(u32 *pAnm, s32 nPosX, s32 nPosY)
{
	s32	nSlotNo;

	if ((nSlotNo = DustGetSlot()) == -1) return (-1);
//printf("Dust slot: %d\n", (int)nSlotNo);
	if ((gpDustSlots[nSlotNo].nAnm = AnmSet(pAnm, -1)) == -1) return (-1);

	gpDustSlots[nSlotNo].nUsed = 1;
	gpDustSlots[nSlotNo].nPosX = nPosX;
	gpDustSlots[nSlotNo].nPosY = nPosY;

	return (nSlotNo);

}


// Avance les anims toutes les frames.
void DustManage(void)
{
	u32	i;

	for (i = 0; i < DUST_MAX_SLOTS; i++)
	{
		if (gpDustSlots[i].nUsed)
		{
			s32	nSpr;

			nSpr = AnmGetImage(gpDustSlots[i].nAnm);
			if (nSpr == -1)
			{
				// L'anim est finie. On kille la poussière.
				DustReleaseSlot(i);
			}
			else
			{
				// Affichage de la poussière.
				SprDisplay(nSpr, gpDustSlots[i].nPosX, gpDustSlots[i].nPosY, e_Prio_Dust);
			}
		}
	}

}




