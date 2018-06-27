// Menus.

#include "includes.h"

#define	MENU_Bkg_Mvt	1

enum
{
	MENU_State_FadeIn = 0,
	MENU_State_Input,
	MENU_State_FadeOut,
};
#define	FADE_Step	8

#define HISC_Nb	10
#define HISC_NameLg (16+1)
#define HISC_Filename "high.scr"
struct SScore
{
	char	pName[HISC_NameLg];
	u16	nRound;
	u32	nScore;
};
struct SScore	gpHighScores[HISC_Nb];


struct SMenuItm
{
	u32	nMenuVal;
	u32	nLg;		// Largeur en pixels du texte (pour centrage).
	char	*pTxt;
};


#define CURS_Acc 0x200

struct SMenuGen
{
	u32	nChoix;
	s32	nOldMousePosX, nOldMousePosY;		// Ancienne position de la souris.
	u32	nState;
	s32	nFadeVal;

	s16	nCursPos;		// Pour faire bouger le curseur.
	s16	nCursAccel;

	// Variables pour saisie du nom.
	u32	nScIdx;				// Pos dans la chaîne. / Pas remis à 0 !
	char	pScName[HISC_NameLg];	// Nom saisi. / Pas remis à 0 !
	u8	nRank;
	u8	nKeyDown;

};
struct SMenuGen	gMenu;

//=============================================================================

// Inits de trucs généraux, appelé une fois en début de programme.
void MenuInit(void)
{
	gMenu.nScIdx = 0;		// RAZ entrée du nom pour High Score.

}


// Curseur - Init.
void CursorInit(void)
{
	gMenu.nCursPos = 0;
	gMenu.nCursAccel = CURS_Acc;
}

// Curseur - Déplacement.
void CursorMove(void)
{
	gMenu.nCursAccel -= 0x20;		// Gravité.
	gMenu.nCursPos += gMenu.nCursAccel;
	if (gMenu.nCursPos < 0) CursorInit();
}

//=============================================================================

// Fade.
void Fade(s32 nFadeVal)//, SDL_Surface *pSurf)
{
	if (nFadeVal < 0) return;

	SDL_Color	pPal[256];
	SDL_Color	*pSrc = gVar.pColors;//pSurf->format->palette->colors;
	u32	i, nNbColors;

	nNbColors = 256;//pSurf->format->palette->ncolors;
	if (nFadeVal > 255) nFadeVal = 255;
	for (i = 0; i < nNbColors; i++)
	{
		pPal[i].r = (pSrc->r * nFadeVal) / 255;
		pPal[i].g = (pSrc->g * nFadeVal) / 255;
		pPal[i].b = (pSrc->b * nFadeVal) / 255;
		pSrc++;
	}
	SDL_SetPalette(gVar.pScreen, SDL_PHYSPAL, pPal, 0, nNbColors);

}

// Init du fader. Appelé dans les Init de chaque menu.
void MenuInitFade(void)
{
	// Fader.
	gMenu.nState = MENU_State_FadeIn;
	gMenu.nFadeVal = 0;

	// Replace la souris au milieu de l'écran.
	gMenu.nOldMousePosX = SCR_Width / 2;
	gMenu.nOldMousePosY = SCR_Height / 2;
	SDL_WarpMouse(SCR_Width / 2, SCR_Height / 2);

}

//=============================================================================

#define	MENU_Main_StartLn	160

struct SMenuItm gpMenuItems_Main[] =
{
	{ MENU_Game, 0, "START" },
	{ MENU_HallOfFame, 0, "HALL OF FAME" },
	{ MENU_Quit, 0, "QUIT" },
};

// Menu main : Init.
void MenuMain_Init(void)
{
	u32	i;

	MenuInitFade();

	// Récupère les longueurs des phrases.
	for (i = 0; i < NBELEM(gpMenuItems_Main); i++)
	{
		gpMenuItems_Main[i].nLg = Font_Print(0, 0, gpMenuItems_Main[i].pTxt, FONT_NoDisp);
	}

	gMenu.nChoix = 0;

	CursorInit();

	// Décor.
	gVar.pBackground = gVar.pBkg[0];
#ifdef MENU_Bkg_Mvt
	gVar.pBkgRect = &gVar.sBkgRect;
	gVar.sBkgRect.w = SCR_Width;
	gVar.sBkgRect.h = SCR_Height;
#endif
	//Son
	load_sounds();
	jouer_son(intro_sound);

	//Active le clavier
	clavier_actif = 1;
}

// Menu main : Main.
u32 MenuMain_Main(void)
{
	u32	nRetVal = MENU_Null;
	u32	i;
	s32	nDiff;
	static	u8	nWait = 0;

#ifdef MENU_Bkg_Mvt
	static	u8	nSinIdx1 = 0;

	// Déplacement du décor (sinus).
//	gVar.sBkgRect.x = (gVar.pSin[nSinIdx1] >> 3) & 31;
//	gVar.sBkgRect.y = 0;
//	gVar.sBkgRect.y = (gVar.pSin[(2 * nSinIdx1) & 0xFF] >> 3) & 31;
//	nSinIdx1++;
	// Déplacement du décor (ligne).
	gVar.sBkgRect.x = (nSinIdx1) & 31;
	gVar.sBkgRect.y = 0;
//	gVar.sBkgRect.x = 0;
//	gVar.sBkgRect.y = (nSinIdx1 >> 1) & 31;
	nSinIdx1++;
#endif


	// Selon l'état.
	switch (gMenu.nState)
	{
	case MENU_State_FadeIn :
		gMenu.nFadeVal += FADE_Step;
		if (gMenu.nFadeVal > 256)
		{
			gMenu.nState = MENU_State_Input;
			gMenu.nFadeVal = -1;
		}
		break;

	case MENU_State_FadeOut :
		gMenu.nFadeVal -= FADE_Step;
		if (gMenu.nFadeVal < 0)
		{
			nWait = 0;
			nRetVal = gpMenuItems_Main[gMenu.nChoix].nMenuVal;
		}
		break;

	case MENU_State_Input :
		// Déplacement du curseur.
		if (nWait) nWait--;
		if (nWait == 0)
		{
			nDiff = gVar.nMousePosY - gMenu.nOldMousePosY;
			if (ABS(nDiff) > 8)
			{
				u32	nLastChoix = gMenu.nChoix;
				if (nDiff < 0)
				{
					if (gMenu.nChoix > 0) { gMenu.nChoix--; }
				}
				else
				{
					if (gMenu.nChoix < NBELEM(gpMenuItems_Main) - 1) { gMenu.nChoix++; }
				}
				if (nLastChoix != gMenu.nChoix)
				{
					CursorInit();	// Slt parce que c'est plus joli.
					nWait = 12;
				}
			}
			if (gVar.pKeys[SDLK_UP])
			{
				if (gMenu.nChoix > 0) 
				{
					gMenu.nChoix--;
				} 
				else 
				{
					gMenu.nChoix=NBELEM(gpMenuItems_Main) - 1;
				}
				nWait = 8;
				jouer_son(menu_sound);

			}
			if (gVar.pKeys[SDLK_DOWN])
			{
				if (gMenu.nChoix < NBELEM(gpMenuItems_Main) - 1) 
				{
					gMenu.nChoix++;
				} 
				else 
				{
					gMenu.nChoix=0;
				}
				nWait = 8;
				jouer_son(menu_sound);

			}
		}

		// Clic souris ? => Validation.
		if (gVar.nMouseButtons & MOUSE_BtnLeft)
		{
			gMenu.nFadeVal = 256;
			gMenu.nState = MENU_State_FadeOut;
		}
		// Validation B ou Start
		if (gVar.pKeys[SDLK_RETURN] || gVar.pKeys[SDLK_LALT])
		{
			Mix_FreeChunk( menu_sound );
			Mix_FreeChunk( intro_sound );
			gMenu.nFadeVal = 256;
			gMenu.nState = MENU_State_FadeOut;
			if (gMenu.nChoix == NBELEM(gpMenuItems_Main) - 1)
			{
				clean_up();
			}
		}
		break;
	}

	// Replace la souris au milieu de l'écran.
	SDL_WarpMouse(SCR_Width / 2, SCR_Height / 2);

	CursorMove();

	//>>> Affichage.

//	// Logo statique.
//	SprDisplay(e_Spr_Logo, SCR_Width / 2, 100, 200);
	// Logo qui bouge.
	s32	pOfs[] = { 0, 26, 48, 73, 99, 127, 158, 166 };
	static	u8	nSin = 0;
	for (i = 0; i < 8; i++)
	{
		s32	nMul = 8;
		SprDisplay(e_Spr_Logo8 + i,
			(SCR_Width / 2) -96 + pOfs[i] + (gVar.pCos[(nSin + (i * nMul)) & 0xFF] / 8),
			90 - (gVar.pSin[(nSin + (i * nMul)) & 0xFF] / 16), 220+i);
	}
	nSin -= 2;

	// Menu.
	for (i = 0; i < NBELEM(gpMenuItems_Main); i++)
	{
		Font_Print((SCR_Width / 2) - (gpMenuItems_Main[i].nLg / 2), MENU_Main_StartLn + (i * 12), gpMenuItems_Main[i].pTxt, 0);
		// Selecteur.
		if (i == gMenu.nChoix)
		{
			Font_Print((SCR_Width / 2) - (gpMenuItems_Main[i].nLg / 2) - 18+4 - (gMenu.nCursPos >> 8), MENU_Main_StartLn + (i * 12), ">", 0);
			Font_Print((SCR_Width / 2) + (gpMenuItems_Main[i].nLg / 2) + 10-4 + (gMenu.nCursPos >> 8), MENU_Main_StartLn + (i * 12), "<", 0);
		}
	}

	gVar.nFadeVal = gMenu.nFadeVal;
	return (nRetVal);
	SDL_BlitSurface(gVar.pScreen, NULL, gVar.hwscreen, NULL);
	SDL_UpdateRect(gVar.hwscreen, 0, 0, SCR_Width, SCR_Height);
}

//=============================================================================

// Scores - Check si un score entre au Hall of Fame.
// Out : -1, pas dedans / >= 0, rang.
s32 Scr_CheckHighSc(u32 nScorePrm)
{
	s32	i, nRank;

	nRank = -1;
	for (i = HISC_Nb - 1; i >= 0; i--)
	{
		if (nScorePrm >= gpHighScores[i].nScore)
		{
			nRank = i;
		}
	}
	return (nRank);

}

// Insère un nom dans la table.
void Scr_PutNameInTable(char *pName, u32 nRound, u32 nScore)
{
	s32	nRank = Scr_CheckHighSc(nScore);
	s32	i;

	if (nRank < 0) return;		// Ne devrait pas arriver.

	// Décalage de la table.
	for (i = HISC_Nb - 2; i >= nRank; i--)
	{
		strcpy(gpHighScores[i + 1].pName, gpHighScores[i].pName);
		gpHighScores[i + 1].nRound = gpHighScores[i].nRound;
		gpHighScores[i + 1].nScore = gpHighScores[i].nScore;
	}
	// Le score à insérer.
	strcpy(gpHighScores[nRank].pName, pName);
	gpHighScores[nRank].nRound = nRound;
	gpHighScores[nRank].nScore = nScore;

}


// RAZ de la table des high scores.
void Scr_RazTable(void)
{
	char	pDefault[HISC_NameLg] = "----------------";
	u32	i;

	for (i = 0; i < HISC_Nb; i++)
	{
		strcpy(gpHighScores[i].pName, pDefault);
		gpHighScores[i].nRound = 0;
		gpHighScores[i].nScore = 0;
	}

}

// Calcule le checksum de la table des scores.
u32 Scr_CalcChecksum(void)
{
	u32	i, j;
	u32	nChk = 0;

	for (i = 0; i < HISC_Nb; i++)
	{
		nChk += gpHighScores[i].nScore;
		nChk += gpHighScores[i].nRound;
		for (j = 0; j < HISC_NameLg; j++) nChk += ((u32)gpHighScores[i].pName[j]) << (8 * (j & 3));
	}
	return (nChk);
}

// Lecture du fichier des high scores.
void Scr_Load(void)
{
	FILE	*pFile;
	u32	nChk;

	if ((pFile = fopen(HISC_Filename, "rb")) != NULL)
	{
		// Le fichier existe, lecture.
		fread(gpHighScores, sizeof(struct SScore), HISC_Nb, pFile);
		fread(&nChk, sizeof(u32), 1, pFile);
		fclose(pFile);
		// Checksum ok ?
		if (nChk != Scr_CalcChecksum())
		{
			// Wrong checksum, RAZ table.
			printf("Scr_Load: Wrong checksum! Resetting table.\n");
			Scr_RazTable();
		}
	}
	else
	{
		// Le fichier n'existe pas, RAZ table.
		Scr_RazTable();
	}

}

// Sauvegarde du fichier des high scores.
void Scr_Save(void)
{
	FILE	*pFile;
	u32	nChk;

	if ((pFile = fopen(HISC_Filename, "wb")) == NULL)
	{
		printf("Unable to save highscores table\n");
		return;
	}
	// Sauvegarde des enregistrements.
	fwrite(gpHighScores, sizeof(struct SScore), HISC_Nb, pFile);
	// Checksum.
	nChk = Scr_CalcChecksum();
	fwrite(&nChk, sizeof(u32), 1, pFile);
	fclose(pFile);

}

//=============================================================================

struct SBouffonerie1	// Effet de lignes.
{
	u32	nWait;
	s32	nSpdMax;
	s32	nSpd;
	s32	nPosX;

};
struct SBouffonerie1	gpBouf1[HISC_Nb];

struct SBouffonerie2	// Quelques monstres.
{
	s32	nAnmNo;
	u32	nSens;	// 0:Bas / 1:Haut / 2:Droite / 3:Gauche.
	u32	nWait, nWait2;
	s32	nSpd;
	s32	nPosX, nPosY;

};
#define	BOUF2_Nb	8
struct SBouffonerie2	gpBouf2[BOUF2_Nb];

// Menu des high-scores : Init.
void MenuHighScores_Init(void)
{
	u32	i;

	MenuInitFade();
	AnmInitEngine();	// Pour monstres.

	// Décor.
	gVar.pBackground = gVar.pBkg[1];
#ifdef MENU_Bkg_Mvt
	gVar.pBkgRect = &gVar.sBkgRect;
	gVar.sBkgRect.w = SCR_Width;
	gVar.sBkgRect.h = SCR_Height;
#endif

	// Init effet des lignes.
	for (i = 0; i < HISC_Nb; i++)
	{
		gpBouf1[i].nSpdMax = -0x800;//(i & 1 ? -1 : 1) * 0x800;
		gpBouf1[i].nSpd = gpBouf1[i].nSpdMax;
		gpBouf1[i].nPosX = SCR_Width << 8;//(i & 1 ? SCR_Width : -SCR_Width) << 8;
		gpBouf1[i].nWait = (HISC_Nb - i) * 8;
	}

	// Init des monstres (init réelle quand nWait = 0).
	for (i = 0; i < BOUF2_Nb; i++)
	{
		gpBouf2[i].nAnmNo = AnmSet(gAnm_Mst1, -1);	// Réserve un slot.
		gpBouf2[i].nWait = (rand() & 63) | 16;		// Attente mini.
	}

}

// Menu des high-scores : Main.
#define	MENU_HiSc_Interligne	19
u32 MenuHighScores_Main(void)
{
	u32	nRetVal = MENU_Null;
	u32	i;
	s32 nPosX, nPosY;

#ifdef MENU_Bkg_Mvt
	static	u8	nSinIdx1 = 0;

	// Déplacement du décor (scroll vertical).
	gVar.sBkgRect.x = 0;
	gVar.sBkgRect.y = nSinIdx1 & 0x0F;
	nSinIdx1++;
#endif

	// Selon l'état.
	switch (gMenu.nState)
	{
	case MENU_State_FadeIn :
		gMenu.nFadeVal += FADE_Step;
		if (gMenu.nFadeVal > 256)
		{
			gMenu.nState = MENU_State_Input;
			gMenu.nFadeVal = -1;
		}
		break;

	case MENU_State_FadeOut :
		gMenu.nFadeVal -= FADE_Step;
		if (gMenu.nFadeVal < 0)
		{
			nRetVal = MENU_Main;	// Sortie.
		}
		break;

	case MENU_State_Input :
		// Clic souris ? => Validation.
		if (gVar.nMouseButtons & MOUSE_BtnLeft)
		{
			gMenu.nFadeVal = 256;
			gMenu.nState = MENU_State_FadeOut;
		}
		if (gVar.pKeys[SDLK_RETURN] || gVar.pKeys[SDLK_LALT])
		{
			gMenu.nFadeVal = 256;
			gMenu.nState = MENU_State_FadeOut;
		}
		break;
	}

	// Replace la souris au milieu de l'écran.
	SDL_WarpMouse(SCR_Width / 2, SCR_Height / 2);


	if (gMenu.nState != MENU_State_FadeIn)
	{
		// Effet des lignes.
		for (i = 0; i < HISC_Nb; i++)
		{
			if (gpBouf1[i].nWait == 0)
			{

				if (gpBouf1[i].nSpdMax)
				{
					s32	nLastPosX = gpBouf1[i].nPosX;

					gpBouf1[i].nPosX += gpBouf1[i].nSpd;
					if (ABS(gpBouf1[i].nSpdMax) == 0x100 && gpBouf1[i].nPosX >> 8 == 0)
					{
						// Stop.
						gpBouf1[i].nSpdMax = 0;
					}
					else
					{
						if (SGN(gpBouf1[i].nPosX) != SGN(nLastPosX))
						{
							// Retourne la vitesse max.
							gpBouf1[i].nSpdMax = -gpBouf1[i].nSpdMax / 2;
						}

						if ( (SGN(gpBouf1[i].nSpd) != SGN(gpBouf1[i].nSpdMax)) ||
							(SGN(gpBouf1[i].nSpd) == SGN(gpBouf1[i].nSpdMax) && ABS(gpBouf1[i].nSpd) < ABS(gpBouf1[i].nSpdMax)) )
						{
							gpBouf1[i].nSpd += SGN(gpBouf1[i].nSpdMax) << 6;
						}
					}

				}

			}
			else
			{
				gpBouf1[i].nWait--;
			}
		}


		// Les monstres.
		for (i = 0; i < HISC_Nb; i++)	// Slt pour tester la fin de l'effet de lignes.
		{
			if (!(gpBouf1[i].nWait == 0 && gpBouf1[i].nSpdMax == 0)) break;
		}
		if (i == HISC_Nb)				// Effet des lignes fini ?
		for (i = 0; i < BOUF2_Nb; i++)
		{
			if (gpBouf2[i].nWait == 0)
			{
				// Déplacement.
				switch (gpBouf2[i].nSens)
				{
				case 0:	// 0:Bas.
					gpBouf2[i].nPosY += gpBouf2[i].nSpd;
					if (gpBouf2[i].nPosY >> 8 >= SCR_Height +16) gpBouf2[i].nWait = (rand() & 63) | 16;
					break;
				case 1:	// 1:Haut.
					gpBouf2[i].nPosY -= gpBouf2[i].nSpd;
					if (gpBouf2[i].nPosY >> 8 <= -16) gpBouf2[i].nWait = (rand() & 63) | 16;
					break;
				case 2:	// 2:Droite.
					gpBouf2[i].nPosX += gpBouf2[i].nSpd;
					if (gpBouf2[i].nPosX >> 8 >= SCR_Width +16) gpBouf2[i].nWait = (rand() & 63) | 16;
					break;
				case 3:	// 3:Gauche.
					gpBouf2[i].nPosX -= gpBouf2[i].nSpd;
					if (gpBouf2[i].nPosX >> 8 <= -16) gpBouf2[i].nWait = (rand() & 63) | 16;
					break;
				}
				SprDisplay(AnmGetImage(gpBouf2[i].nAnmNo), gpBouf2[i].nPosX >> 8, gpBouf2[i].nPosY >> 8, 50+i);
				// Changement de direction ?
				if (--gpBouf2[i].nWait2 == 0)
				{
					gpBouf2[i].nSens = (gpBouf2[i].nSens & 2 ? 0 : 2);
					gpBouf2[i].nSens += (rand() & 1);
					//gpBouf2[i].nSens &= 3;
					gpBouf2[i].nWait2 = (rand() & 63) | 64;
				}
			}
			else
			{
				gpBouf2[i].nWait--;
				if (gpBouf2[i].nWait == 0)
				{
					// Init.
					u32	*pAnm[] = { gAnm_Mst1, gAnm_Mst2, gAnm_Mst3, gAnm_Mst4 };

					AnmSet(pAnm[rand() & 3], gpBouf2[i].nAnmNo);	// Anim.
					gpBouf2[i].nSens = rand() & 3;
					switch (gpBouf2[i].nSens)
					{
					case 0:	// 0:Bas.
						gpBouf2[i].nPosX = (rand() % (SCR_Width - 32)) + 16;
						gpBouf2[i].nPosY = -16;
						break;
					case 1:	// 1:Haut.
						gpBouf2[i].nPosX = (rand() % (SCR_Width - 32)) + 16;
						gpBouf2[i].nPosY = SCR_Height +16;
						break;
					case 2:	// 2:Droite.
						gpBouf2[i].nPosY = (rand() % (SCR_Height - 32)) + 16;
						gpBouf2[i].nPosX = -16;
						break;
					case 3:	// 3:Gauche.
						gpBouf2[i].nPosY = (rand() % (SCR_Height - 32)) + 16;
						gpBouf2[i].nPosX = SCR_Width +16;
						break;
					}
					gpBouf2[i].nPosX <<= 8;
					gpBouf2[i].nPosY <<= 8;
					gpBouf2[i].nSpd = 0x200;
					gpBouf2[i].nWait2 = (rand() & 63) | 64;
				}
			}

		}


	}


	//>>> Affichage.

	// Titre.
	char	pTitle[] = "- HALL OF FAME -";
	u32	nLg = Font_Print(0, 8, pTitle, FONT_NoDisp);
	Font_Print((SCR_Width - nLg) / 2, 24, pTitle, 0);

	// Affichage des lignes.
	nPosY = 48;
	for (i = 0; i < HISC_Nb; i++)
	{
		char	pStr[8+1];// = "00000000";
		s32	nOfs;

		nPosX = 8+16 + (gpBouf1[i].nPosX >> 8);		// L'effet.

		// Pos.
		strcpy(pStr, "00");
		MyItoA(i + 1, pStr);
		Font_Print(nPosX, nPosY + (i * MENU_HiSc_Interligne), pStr, 0);
		// Nom.
		Font_Print(nPosX + (8 * 3) + 4+4, nPosY + (i * MENU_HiSc_Interligne), gpHighScores[i].pName, 0);
		// Round.
		nOfs = 0;
		if (gpHighScores[i].nRound + 1 > LEVEL_Max)
		{
			strcpy(pStr, "ALL");
			nOfs = -4;
		}
		else
		{
			strcpy(pStr, "00");
			MyItoA(gpHighScores[i].nRound + 1, pStr);
		}
		Font_Print(nPosX + (8 * 20) + 8+8 + nOfs, nPosY + (i * MENU_HiSc_Interligne), pStr, 0);
		// Score.
		strcpy(pStr, "00000000");
		MyItoA(gpHighScores[i].nScore, pStr);
		Font_Print(nPosX + (8 * 23) + 12+12, nPosY + (i * MENU_HiSc_Interligne), pStr, 0);

	}

	gVar.nFadeVal = gMenu.nFadeVal;
	return (nRetVal);

}

//=============================================================================

struct SMenuItm gpMenuItems_GetName[] =
{
	{ 0, 0, "CONGRATULATIONS!" },
	{ 0, 0, "YOU RANKED #0@" },
	{ 0, 0, "ENTER YOUR NAME:" },
};

// Init.
void MenuGetName_Init(void)
{
	u32	i;

	MenuInitFade();

	// Rank atteint.
	gMenu.nRank = Scr_CheckHighSc(gExg.nScore);
	// Calcul de la longueur des chaînes.
	for (i = 0; i < NBELEM(gpMenuItems_GetName); i++)
	{
		gpMenuItems_GetName[i].nLg = Font_Print(0, 8, gpMenuItems_GetName[i].pTxt, FONT_NoDisp);
	}
	//
	gMenu.nKeyDown = 0;

}
char cChar=65;

void letterplus ()
{
	if (cChar < 127)
	{
		cChar++;
	} else {
		cChar=32;
	}
}

void letterminus ()
{
	if (cChar > 31)
	{
		cChar--;
	} else {
		cChar=126;
	}
}
// Saisie du nom quand high-score.
u32 MenuGetName_Main(void)
{
	u32	nRet = MENU_Null;
	u32	i;
	static u32	nCligno = 0;

	// Selon l'état.
	switch (gMenu.nState)
	{
	case MENU_State_FadeIn :
		gMenu.nFadeVal += FADE_Step;
		if (gMenu.nFadeVal > 256)
		{
			gMenu.nState = MENU_State_Input;
			gMenu.nFadeVal = -1;
		}
		break;

	case MENU_State_FadeOut :
		gMenu.nFadeVal -= FADE_Step;
		if (gMenu.nFadeVal < 0)
		{
			// Si pas de nom, mettre Ezial.
			char	pDefName[] = "Ezial";
			if (gMenu.nScIdx == 0)
			{
				strcpy(gMenu.pScName, pDefName);
				gMenu.nScIdx = strlen(pDefName);
			}
			// Rajoute le nom dans les High-scores.
			Scr_PutNameInTable(gMenu.pScName, gExg.nLevel, gExg.nScore);
			Scr_Save();				// Sauvegarde du fichier des scores.

			nRet = MENU_Main;		// Sortie.
		}
		break;

	case MENU_State_Input :
		// Gestion du clavier.
		if (gVar.pKeys[SDLK_RETURN])
		{
			gMenu.nState = MENU_State_FadeOut;
			gMenu.nFadeVal = 256;
			break;
		}

		// On regarde quelle touche est enfoncée.
		//u32	nChr = 0;
		int i;
		int nWait = 10;
		if (nWait) nWait--;

		if (gVar.pKeys[SDLK_UP] & (nWait==0))
		{
			letterplus();
			nWait = 10;
		}
		if (gVar.pKeys[SDLK_DOWN] & (nWait==0))
		{
			letterminus();
			nWait = 10;
		}
		if (gVar.pKeys[SDLK_RIGHT] & (nWait==0))
		{
			for (i=0;i<5;i++)
			{
				letterplus();
			}
			nWait = 10;
		}
		if (gVar.pKeys[SDLK_LEFT] & (nWait==0))
		{
			for (i=0;i<5;i++)
			{
				letterminus();
			}
			nWait = 10;
		}
		
		if (gVar.pKeys[SDLK_LCTRL] & (nWait==0))
		{
			if (gMenu.nScIdx) gMenu.pScName[--gMenu.nScIdx] = 0;
			nWait=10;
		}
		if (gVar.pKeys[SDLK_LCTRL] & (nWait==0))			
		{
			if (gMenu.nScIdx < HISC_NameLg - 1)//strlen(gMenu.pScName))
			{
				gMenu.pScName[gMenu.nScIdx++] = cChar;
				gMenu.pScName[gMenu.nScIdx] = 0;
			}
			gMenu.nKeyDown = 1;
			nWait=10;
		}
		
		break;
		/*if (gVar.pKeys[SDLK_SPACE]) nChr = ' ';
		for (i = SDLK_a; i <= SDLK_z; i++)
		{
			if (gVar.pKeys[i])
			{
				nChr = i - SDLK_a + 'A';
				break;
			}
		}
		for (i = SDLK_0; i <= SDLK_9; i++)
		{
			if (gVar.pKeys[i])
			{
				nChr = i - SDLK_0 + '0';
				break;
			}
		}
		if (gVar.pKeys[SDLK_BACKSPACE])
		{
			nChr = SDLK_BACKSPACE;
		}
		
		// Pseudo trigger.
		if (gMenu.nKeyDown == 0 && nChr)
		{
			if (nChr == SDLK_BACKSPACE)
			{
				if (gMenu.nScIdx) gMenu.pScName[--gMenu.nScIdx] = 0;
			}
			else if (gMenu.nScIdx < HISC_NameLg - 1)//strlen(gMenu.pScName))
			{
				gMenu.pScName[gMenu.nScIdx++] = nChr;
				gMenu.pScName[gMenu.nScIdx] = 0;
			}
			gMenu.nKeyDown = 1;
		}
		else if (gMenu.nKeyDown == 1 && nChr == 0)
		{
			gMenu.nKeyDown = 0;		// Release.
		}

		break;*/
	}


	// Replace la souris au milieu de l'écran.
	SDL_WarpMouse(SCR_Width / 2, SCR_Height / 2);

	// Affichage.

	// On rajoute le rank dans sa ligne.
	char pRank[30];
	strcpy(pRank, gpMenuItems_GetName[1].pTxt);
	char *pPtr = strchr(pRank, '@');
	if (pPtr != NULL)
	{
		MyItoA(gMenu.nRank + 1, pPtr-1);
	}

	// Lignes.
	for (i = 0; i < NBELEM(gpMenuItems_GetName); i++)
	{
		Font_Print((SCR_Width - gpMenuItems_GetName[i].nLg) / 2, 80 + (i*32), (i == 1 ? pRank : gpMenuItems_GetName[i].pTxt), 0);
	}
	// Nom en cours.
	i = Font_Print(0, 0, gMenu.pScName, FONT_NoDisp);
	Font_Print((SCR_Width - i) / 2, 80+64+16, gMenu.pScName, 0);
	// Curseur au bout du nom.
	if ((++nCligno) & 8)
	{
		Font_Print(((SCR_Width - i) / 2) + i, 80+64+16, "_", 0);
	}

	gVar.nFadeVal = gMenu.nFadeVal;
	return (nRet);

}




