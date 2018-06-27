
#include "includes.h"
#define PI 3.1415927

// Précalcul des tables de sinus-cosinus.
// 256 angles, val *256 (=> varie de -256 à 256).
void PrecaSinCos(void)
{
	u32	i;

	for (i = 0; i < 256 + 64; i++)
	{
		gVar.pSinCos[i] = (s16) (cos(i * 2 * PI / 256) * 256);
		//printf("i = %d : %f\n", i, cos(i * 2 * PI / 256) * 256);
		//printf("i = %d : %d\n", i, gVar.pSinCos[i]);
	}
	gVar.pSin = gVar.pSinCos + 64;
	gVar.pCos = gVar.pSinCos;

	/*
	for (i = 0; i < 256; i++)
	{
		printf("i = %d : sin = %d - cos = %d\n", i, gVar.pSin[i], gVar.pCos[i]);
	}
	*/

}


