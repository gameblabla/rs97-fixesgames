
#include "includes.h"

// Non géré:
// * Espace entre briques indestructible < taille maxi de la balle.
//	Si la balle grossit alors qu'elle est entre, ça pose un pb.
//	=> Graphiquement, ça ne peut pas marcher. => Placer les briques indestructibles en fonction.

//>> reste à faire
//todo: lancer la balle au bout de 5 secondes en début de vie.
//todo: time out après x minutes => les briques indestructibles se transforment en briques cassables.
// (avec un effet d'apparition en diagonale, du haut gauche vers le bas droite)

//todo: les "for (i = 0; i < BALL_MAX_NB; i++)" sont optimisables avec une sortie sur gBreak.nBallsNb;
//<< reste à faire



//=============================================================================

#include "levels.h"

struct SBreaker	gBreak;

void BallChangeSize(struct SBall *pBall, u32 nSize);
void BallInit(struct SBall *pBall, s32 nPosX, s32 nPosY, u32 nSize, u32 nFlags, s32 nSpeed, u8 nAngle);

//=============================================================================

// Bonus : Active l'aimant.
void BreakerBonusSetAimant(void)
{
	if ((gBreak.nPlayerFlags & PLAYER_Flg_Aimant) == 0)
	{
		gBreak.nPlayerFlags |= PLAYER_Flg_Aimant;
		gBreak.nPlayerAnmBonusM = AnmSet(gAnm_RaqAimant, gBreak.nPlayerAnmBonusM);
	}

}

// Bonus : Active la mitrailleuse.
void BreakerBonusSetMitrailleuse(void)
{
	if ((gBreak.nPlayerFlags & PLAYER_Flg_Mitrailleuse) == 0)
	{
		gBreak.nPlayerFlags |= PLAYER_Flg_Mitrailleuse;
		gBreak.nPlayerAnmBonusD = AnmSet(gAnm_RaqMitDRepos, gBreak.nPlayerAnmBonusD);
		gBreak.nPlayerAnmBonusG = AnmSet(gAnm_RaqMitGRepos, gBreak.nPlayerAnmBonusG);
//todo: dust pour faire dégager les bouts rouges de la raquette.
	}

}

void Balle_Acceleration (struct SBall *pBall,int acceleration)
{
	if (pBall->nSpeed < BALL_Speed_Max-acceleration) 
	{	
		pBall->nSpeed += acceleration;
	}
	else
	{
		pBall->nSpeed = BALL_Speed_Max;
	}
}

// Bonus : Allongement de la raquette.
// Malus : Rétrécissement de la raquette.
// In: + pour grandir, - pour réduire.
void BreakerBonusRaquetteSize(s32 nSens)
{
	u32	*pRallonge[] = { gAnm_RaqRallonge0, gAnm_RaqRallonge1, gAnm_RaqRallonge2 };
	u32	*pReduit[] = { gAnm_RaqReduit0, gAnm_RaqReduit1, gAnm_RaqReduit2 };

	if (nSens >= 0)
	{
		// Rallonge.
		if (gBreak.nPlayerRSize < 3)
		{
			AnmSet(pRallonge[gBreak.nPlayerRSize], gBreak.nPlayerAnmNo);
			gBreak.nPlayerRSize++;
		}
	}
	else
	{
		// Réduit.
		if (gBreak.nPlayerRSize > 0)
		{
			gBreak.nPlayerRSize--;
			AnmSet(pReduit[gBreak.nPlayerRSize], gBreak.nPlayerAnmNo);
		}
	}

}

// Bonus : Balle(s) traversante(s).
void BreakerBonusBallTraversante(void)
{
	struct SBall	*pBall;
	u32	i;

	for (i = 0; i < BALL_MAX_NB; i++)
	{
		pBall = &gBreak.pBalls[i];
		if (pBall->nUsed)
		{
			pBall->nFlags |= BALL_Flg_Traversante;
			BallChangeSize(pBall, pBall->nSize);		// Change le sprite.
		}
	}
}

// Bonus : Balle(s) x3.
void BreakerBonusBallX3(void)
{
	u8	pUsed[BALL_MAX_NB];
	struct SBall	*pBall;
	u32	i, j, nLibre;

	// On flague les balles présentes.
	for (i = 0; i < BALL_MAX_NB; i++) pUsed[i] = gBreak.pBalls[i].nUsed;
	// On récupère ces mêmes balles et on les multiplie !
	nLibre = 0;
	for (i = 0; i < BALL_MAX_NB; i++)
	{
		if (pUsed[i])
		{
			pBall = &gBreak.pBalls[i];
			// 2 balles pour chaque balle présente.
			for (j = 0; j < 2; j++)
			{
				// Recherche d'un slot libre.
				for (; nLibre < BALL_MAX_NB; nLibre++) if (gBreak.pBalls[nLibre].nUsed == 0) break;
				if (nLibre == BALL_MAX_NB) break;	// Plus de place, on sort.
				// Init balle
				BallInit(&gBreak.pBalls[nLibre], pBall->nPosX, pBall->nPosY,
					pBall->nSize, pBall->nFlags, pBall->nSpeed,
					pBall->nAngle + (j & 1 ? 10 : -10));
			}
		}
	}

}

// Bonus : Grossissement balle(s).
// Attention ! Si la balle déborde dans le mur, la recaler de force.
void BreakerBonusBallBigger(void)
{
	struct SBall	*pBall;
	struct SSprite *pSpr;
	u32	i;

	for (i = 0; i < BALL_MAX_NB; i++)
	{
		pBall = &gBreak.pBalls[i];
		if (pBall->nUsed)
		if (pBall->nSize + 1 <= BALL_MAX_SIZE)
		{
			// On augmente la taille de la balle.
			BallChangeSize(pBall, pBall->nSize + 1);
			// Dans le mur ? => recalage.
			pSpr = SprGetDesc(pBall->nSpr);
			if ((pBall->nPosX >> 8) - pSpr->nPtRefX < WALL_XMin)
			{
				pBall->nPosX = (WALL_XMin + pSpr->nPtRefX) << 8;
			}
			if ((pBall->nPosX >> 8) - pSpr->nPtRefX + pSpr->nLg > WALL_XMax)
			{
				pBall->nPosX = (WALL_XMax - pSpr->nLg + pSpr->nPtRefX) << 8;
			}
			if ((pBall->nPosY >> 8) - pSpr->nPtRefY < WALL_YMin)
			{
				pBall->nPosY = (WALL_YMin + pSpr->nPtRefY) << 8;
			}

//todo: Vérifier ici le cas des balles collées sur la raquette.
//(est-ce que quand le joueur attrape une pill grossissement et que des balles sont collées
// sur la raquette, pendant 1 frame, les balles ne débordent pas dans la raquette ???).
// Bon, au pire c'est pendant 1 frame, car ça sera recalé à la frame suivante.

		}
	}

}

// Bonus : 1 Up.
void BreakerBonus1Up(void)
{
	if (gBreak.nPlayerLives + 1 <= PLAYER_Lives_Max)
	{
		gBreak.nPlayerLives++;
	}

}

// Bonus : Slow down.
// Malus : Speed up.
// In: + pour accélérer, - pour ralentir les balles.
void BreakerBonusSpeedUp(s32 nSens)
{
	struct SBall	*pBall;
	u32	i;

	for (i = 0; i < BALL_MAX_NB; i++)
	{
		pBall = &gBreak.pBalls[i];
		if (pBall->nUsed)
		{
			if (nSens >= 0)
			{
				// Accélération.
				pBall->nSpeed += BALL_Speed_Step;
				if (pBall->nSpeed > BALL_Speed_Max) pBall->nSpeed = BALL_Speed_Max;
			}
			else
			{
				// Décélération.
				pBall->nSpeed -= BALL_Speed_Step;
				if (pBall->nSpeed < BALL_Speed_Min) pBall->nSpeed = BALL_Speed_Min;
			}
		}
	}

}


//todo: Autres bonus possibles...
// Malus : Toutes les briques se transforment en briques à frapper 2 fois.
// Bonus : Plusieurs niveaux de mitrailleuse, faire des missiles qui détruisent les briques
// 			indestructibles / Une explosion qui détruit toutes les briques dans son rayon (retirer le flag indestructible des briques !).
// Malus : Inverse des bonus, perte de l'aimant...

//=============================================================================


// Init level.
void InitLevel(u32 nLevel)
{
	u32	i;
	s8	*pLev = gpLevels[nLevel];	// Sur le level en cours.
	u16	pScores[] = { 50, 60, 70, 80, 90, 100, 110, 120, 100, 100,   100, 100, 0 };


	// Initialisation de la table des briques.
	gBreak.nRemainingBricks = 0;
	gBreak.nBricksComingBackNbCur = 0;
	gBreak.nBricksComingBackTotal = 0;
	for (i = 0; i < TABLE_Width * TABLE_Height; i++)
	{
		if (*(pLev + i) != -1)
		{
			// Une brique.
			gBreak.pLevel[i].nPres = 1;		// Brique présente ou pas.
			gBreak.pLevel[i].nCnt = 1;		// Nb de touches restantes avant la destruction.
			gBreak.pLevel[i].nFlags = 0;	// Flags : Voir liste.
			gBreak.pLevel[i].nResetCnt = 0;	// Compteur pour retour de la brique.

			gBreak.pLevel[i].nSprNo = e_Spr_Bricks + *(pLev + i);	// Sprite par défaut.
			gBreak.pLevel[i].nScore = pScores[*(pLev + i)];			// Score.
			gBreak.pLevel[i].nAnmNo = -1;	// Remplacé par l'anim si != -1.

			gBreak.pLevel[i].pAnmExplo = gAnm_BrickExplo;	// Anim à utiliser pour la disparition.
			gBreak.pLevel[i].pAnmHit = gAnm_Brick2Hit;	// Anim à utiliser pour le hit. Ne sert à rien pour les briques normales.

			gBreak.nRemainingBricks++;

			// Cas particuliers.
			switch (*(pLev + i))
			{
			case e_Spr_BricksSpe:			// Brique à toucher 2 fois.
				gBreak.pLevel[i].nCnt = 2;		// Nb de touches restantes avant la destruction.
				gBreak.pLevel[i].pAnmExplo = gAnm_Brick2HitExplo;	// Anim à utiliser pour la disparition.
				gBreak.pLevel[i].pAnmHit = gAnm_Brick2Hit;	// Anim à utiliser pour le hit.
				break;

			case e_Spr_BricksSpe + 1:		// Brique qui revient.
				gBreak.pLevel[i].nCnt = 2;		// Nb de touches restantes avant la destruction.
				gBreak.pLevel[i].nResetCnt = BRICK_ComingBackCnt;	// Compteur pour retour de la brique.
				gBreak.pLevel[i].nFlags |= BRICK_Flg_ComingBack;
				gBreak.nBricksComingBackTotal++;
				gBreak.pLevel[i].pAnmExplo = gAnm_BrickCBExplo;	// Anim à utiliser pour la disparition.
				gBreak.pLevel[i].pAnmHit = gAnm_BrickCBHit;	// Anim à utiliser pour le hit.
				break;

			case e_Spr_BricksSpe + 2:		// Brique indestructible.
				gBreak.pLevel[i].nFlags |= BRICK_Flg_Indestructible;
				gBreak.nRemainingBricks--;	// Celles la, elles ne comptent pas.
				gBreak.pLevel[i].pAnmHit = gAnm_BrickIndesHit;	// Anim à utiliser pour le hit.
				break;

			default:
				break;
			}

		}
		else
		{
			// Pas de brique.
			gBreak.pLevel[i].nPres = 0;		// Brique présente ou pas.
			gBreak.pLevel[i].nFlags = 0;	// Flags : Voir liste.
			//gBreak.pLevel[i].nResetCnt = 0;	// Compteur pour retour de la brique.
		}

	}



	if (gBreak.nLevel == LEVEL_Max - 1)
	{
		// Boss de fin.

		// Pointeur sur le décor.
		gVar.pLevel = gVar.pLev[4];
		// Rajoute Doh !
		MstAdd(e_Mst_Doh, SCR_Width / 2, 98);
		gBreak.nRemainingBricks = 1;	// Quand on tuera le boss, il décrémentera le nb de briques.
	}
	else
	{
		// Niveaux normaux.

		// Pointeur sur le décor.
		gVar.pLevel = gVar.pLev[gBreak.nLevel & 3];
		// Rajoute un générateur de monstres.
		MstAdd(e_Mst_Generateur, 0, 0);
	}
	// Rajoute une porte à droite.
	MstAdd(e_Mst_DoorR, WALL_XMax, SCR_Height - 13 - 6);

}




// Changement de la taille d'une balle.
void BallChangeSize(struct SBall *pBall, u32 nSize)
{
	struct SSprite *pSpr;
	u32	i, j;

	// Sprite, suivant la taille.
//todo: Si balle animée, faire une table avec les anims dans l'ordre et recupérer pAnm[size].
//todo: Si trop lent, on peut sortir l'init de l'anim, ça évitera de refaire le masque.
	pBall->nSize = nSize;
	pBall->nSpr = (pBall->nFlags & BALL_Flg_Traversante ? e_Spr_BallTrav : e_Spr_Ball) + pBall->nSize;
	pSpr = SprGetDesc(pBall->nSpr);
	pBall->nRayon = pSpr->nLg / 2;		// Offset du centre. / Note : lg impaires.
	pBall->nDiam = pSpr->nHt;			// Hauteur.

	// Cleare le masque.
	for (i = 0; i < BALL_GfxLg * BALL_GfxLg; i++) pBall->pBallMask[i] = 0;
	// Copie du masque.
	for (j = 0; j < pSpr->nHt; j++)
		for (i = 0; i < pSpr->nLg; i++)
			pBall->pBallMask[(j * BALL_GfxLg) + i] = ~pSpr->pMask[(j * pSpr->nHt) + i];

}

// Init d'une nouvelle balle.
void BallInit(struct SBall *pBall, s32 nPosX, s32 nPosY, u32 nSize, u32 nFlags, s32 nSpeed, u8 nAngle)
{

	pBall->nUsed = 1; gBreak.nBallsNb++;

	pBall->nFlags = nFlags;
	pBall->nPosX = nPosX;
	pBall->nPosY = nPosY;
	pBall->nSpeed = nSpeed;
	pBall->nAngle= nAngle;

	BallChangeSize(pBall, nSize);

	pBall->nOffsRaq = 0;

}

// Init d'une balle sur la raquette.
void BallInitOnPlayer(void)
{
	struct SBall	*pBall;

	pBall = &gBreak.pBalls[0];
	// Vitesse : Tous les 4 niveaux, ça part un peu plus vite.
	BallInit(pBall, gBreak.nPlayerPosX << 8, gBreak.nPlayerPosY << 8, 0, BALL_Flg_Aimantee,
		BALL_Speed_Min + ((gBreak.nLevel >> 2) * 0x18), 48);
	pBall->nPosY -= (pBall->nRayon + 1) << 8;

}

// Init des slots des balles. (1 utilisée, les autres à vide).
void BallsInitSlots(void)
{
	u32	i;

	gBreak.nBallsNb = 0;
	for (i = 0; i < BALL_MAX_NB; i++)
	{
		gBreak.pBalls[i].nUsed = 0;
	}

}

// Les balles disparaissent avec un dust.
void BallsKill(void)
{
	struct SBall	*pBall;
	u32	i;

	for (i = 0; i < BALL_MAX_NB; i++)
	{
		pBall = &gBreak.pBalls[i];
		if (pBall->nUsed)
		{
//todo: faire une anim de disparition de balle.
			DustSet(gAnm_MstExplo1, pBall->nPosX >> 8, pBall->nPosY >> 8);
			pBall->nUsed = 0; gBreak.nBallsNb--;	// Nombre de balles en jeu.
		}
	}

}




// Init life, reset de la raquette.
void Brk_PlyrInitLife(void)
{
	// Reset des flags.
	gBreak.nPlayerFlags &= ~(PLAYER_Flg_Aimant | PLAYER_Flg_Mitrailleuse);	// Armes.
	gBreak.nPlayerFlags &= ~PLAYER_Flg_NoKill;		// NoKill.
	gBreak.nPlayerFlags &= ~PLAYER_Flg_DoorR;		// Door Right.
	// Taille de la raquette.
	gBreak.nPlayerRSize = 1;
	// Raquette normale.
//	gBreak.nPlayerAnmNo = AnmSet(gAnm_Raquette, gBreak.nPlayerAnmNo);
	gBreak.nPlayerAnmNo = AnmSet(gAnm_RaqAppear, gBreak.nPlayerAnmNo);
	gBreak.nPlayerAnmClignG = AnmSet(gAnm_RaqClignG, gBreak.nPlayerAnmClignG);
	gBreak.nPlayerAnmClignD = AnmSet(gAnm_RaqClignD, gBreak.nPlayerAnmClignD);
	// Positionnement au centre.
	gBreak.nPlayerPosX = SCR_Width / 2;
	gBreak.nPlayerPosY = SCR_Height - 17;
	// Replace la souris à l'endroit du joueur.
	SDL_WarpMouse(gBreak.nPlayerPosX, gBreak.nPlayerPosY);

	gBreak.nTimerLevelDisplay = TIMER_DisplayLevel;	// Compteur pour affichage du n° de level.

}


// Init pour une partie, récupère/initialise les paramètres de gExg.
void ExgBrkInit(void)
{
	gBreak.nLevel = 0;//LEVEL_Max-1;//0;//
	gBreak.nPlayerLives = PLAYER_Lives_Start;
	gBreak.nPlayerScore = 0;

	gExg.nExitCode = 0;
	BreakerInit();

}

// Prépare la structure gExg pour sortie de la partie.
void ExgExit(u32 nExitCode)
{
	gExg.nExitCode = nExitCode;			// Code de sortie.
	gExg.nLevel = gBreak.nLevel;		// Level atteint au game over.
	gExg.nScore = gBreak.nPlayerScore;	// Score au game over.

}

// Initialisation (appelée à chaque début de niveau (pas de vie, de niveau)).
void BreakerInit(void)
{
	MstInitEngine();
	AnmInitEngine();
	FireInitEngine();
	DustInitEngine();
	InitLevel(gBreak.nLevel);

	srand(time(NULL));		// Init hasard.

	gBreak.nPhase = e_Game_SelectLevel;//e_Game_Normal;
	gBreak.nTimerGameOver = TIMER_GameOver;	// Countdown pour game over.

	// Taille de la raquette.
	gBreak.nPlayerRSize = 1;
	// Réservation des anims (l'anim n'est PAS importante, on les réaffectera plus tard. C'est juste pour réserver un slot).
	gBreak.nPlayerAnmNo = AnmSet(gAnm_Raquette, -1);
	gBreak.nPlayerAnmBonusM = AnmSet(gAnm_RaqAimant, -1);
	gBreak.nPlayerAnmBonusD = AnmSet(gAnm_RaqMitDRepos, -1);
	gBreak.nPlayerAnmBonusG = AnmSet(gAnm_RaqMitGRepos, -1);
	gBreak.nPlayerAnmClignG = AnmSet(gAnm_RaqClignG, -1);
	gBreak.nPlayerAnmClignD = AnmSet(gAnm_RaqClignD, -1);

	gBreak.nPlayerFlags = 0;
	if (gBreak.nLevel == LEVEL_Max - 1)		// Le patch. 1 fois à l'init du level, pas de la vie.
	{
		gBreak.nPlayerFlags |= PLAYER_Flg_BossWait;	// Le joueur attendra que le boss soit prêt.
	}

	Brk_PlyrInitLife();

	BallsInitSlots();
	BallInitOnPlayer();

}


// Tableau de bord.
void BreakerHUD(void)
{
	u32	i;

	// Le nombre de vies restantes.
	for (i = 0; i < gBreak.nPlayerLives; i++)
	{
		SprDisplay(e_Spr_HUDRaquette, WALL_XMin + (i * 16), SCR_Height - 7, e_Prio_HUD);
	}

	// Affichage du score. Note pour le centrage : Les chiffres sont en 8x8.
	char	pScore[8+1] = "00000000";
	MyItoA(gBreak.nPlayerScore, pScore);
	Font_Print((SCR_Width / 2) - ((strlen(pScore) * 8) / 2), 7, pScore, 0);

}

// Dessin du jeu.
void BreakerDraw(void)
{
	u32	i, j, k;

	// Dessin des briques.
	k = 0;
	for (j = 0; j < TABLE_Height; j++)
	{
		for (i = 0; i < TABLE_Width; i++)
		{
			//k = (j * TABLE_Width) + i;	// Index de la brique.

			// Brique présente ?
			if (gBreak.pLevel[k].nPres)
			{
				u32	nSpr;
				s32	nTmp;

				nSpr = gBreak.pLevel[k].nSprNo;
				if (gBreak.pLevel[k].nAnmNo != -1)
				{
					nTmp = AnmGetImage(gBreak.pLevel[k].nAnmNo);
					if (nTmp == -1)
					{
						gBreak.pLevel[k].nAnmNo = -1;
					}
					else
					{
						nSpr = nTmp;
					}
				}

				// Faut-il de l'ombre sur la brique ? (Pas si une à droite, une en dessous et une en dessous à droite. A ce moment là, l'ombre est cachée).
				if (j == TABLE_Height-1)
				{
					goto _Disp;
				}
				else if (i == TABLE_Width-1)
				{
					if (gBreak.pLevel[k + TABLE_Width].nPres == 0) goto _Disp;
				}
				else if (!(gBreak.pLevel[k + 1].nPres && gBreak.pLevel[k + TABLE_Width].nPres && gBreak.pLevel[k + TABLE_Width + 1].nPres))
				{
_Disp:
					nSpr |= SPR_Flag_Shadow;
				}

				SprDisplay(nSpr, WALL_XMin + (i * BRICK_Width), WALL_YMin + (j * BRICK_Height), e_Prio_Briques);

			} // if (gBreak.pLevel[k].nPres)
			k++;
		}
	}
	// Le cache des ombres qui débordent à droite.
	SprDisplay(e_Spr_CacheDroit, 309, 15, 1);


	// Dessin du joueur.

	// Dessin de la raquette.
    i = AnmGetImage(gBreak.nPlayerAnmNo);
	SprDisplay(i | SPR_Flag_Shadow, gBreak.nPlayerPosX, gBreak.nPlayerPosY, e_Prio_Raquette);

	// Si pas en mort ou en apparition...
	if (AnmGetKey(gBreak.nPlayerAnmNo) == e_AnmKey_Null)
	{
		// Les clignotants sur les côtés.
		struct SSprite *pSpr = SprGetDesc(i);
		SprDisplay(AnmGetImage(gBreak.nPlayerAnmClignG) | SPR_Flag_Shadow, gBreak.nPlayerPosX - pSpr->nPtRefX - 1, gBreak.nPlayerPosY, e_Prio_Raquette);
		SprDisplay(AnmGetImage(gBreak.nPlayerAnmClignD) | SPR_Flag_Shadow, gBreak.nPlayerPosX + pSpr->nLg - pSpr->nPtRefX, gBreak.nPlayerPosY, e_Prio_Raquette);
		// Aimant ?
		if (gBreak.nPlayerFlags & PLAYER_Flg_Aimant)
		{
			SprDisplay(AnmGetImage(gBreak.nPlayerAnmBonusM), gBreak.nPlayerPosX, gBreak.nPlayerPosY, e_Prio_Raquette + 1);
		}
		// Mitrailleuse ?
		if (gBreak.nPlayerFlags & PLAYER_Flg_Mitrailleuse)
		{
			SprDisplay(AnmGetImage(gBreak.nPlayerAnmBonusG), gBreak.nPlayerPosX - pSpr->nPtRefX, gBreak.nPlayerPosY, e_Prio_Raquette + 1);
			SprDisplay(AnmGetImage(gBreak.nPlayerAnmBonusD), gBreak.nPlayerPosX - pSpr->nPtRefX + pSpr->nLg - 1, gBreak.nPlayerPosY, e_Prio_Raquette + 1);
		}

	}


	// Dessin des balles.
	for (i = 0; i < BALL_MAX_NB; i++)
	{
		if (gBreak.pBalls[i].nUsed)
			SprDisplay(gBreak.pBalls[i].nSpr | SPR_Flag_Shadow, gBreak.pBalls[i].nPosX >> 8, gBreak.pBalls[i].nPosY >> 8, e_Prio_Raquette);
	}

	// Tableau de bord.
	BreakerHUD();

}


// Met l'anim de mort au joueur (en fct de la taille de la raquette).
void PlayerSetDeath(void)
{
	// Mort du joueur, sauf si flag (passage de la porte, boss tué...).
	if ((gBreak.nPlayerFlags & PLAYER_Flg_NoKill) == 0)
	{
		// Joueur: Anim de mort.
		u32	*pDeath[] = { gAnm_RaqDeath0, gAnm_RaqDeath1, gAnm_RaqDeath2, gAnm_RaqDeath3 };
		AnmSetIfNew(pDeath[gBreak.nPlayerRSize], gBreak.nPlayerAnmNo);
		FireRemoveDohShoots();
	}
}


// Collisions balle-murs.
u32 CollWalls(struct SBall *pBall, s32 *pnOldX, s32 *pnOldY)
{
	u32	RetVal = 0;
	//jouer_son(collisionmur_sound);
	// Mur droit.
	if (pBall->nPosX >> 8 > WALL_XMax - (s32)pBall->nRayon)
	{
		pBall->nAngle = 128 - pBall->nAngle;
		RetVal = 1;
	}
	// Mur gauche.
	if (pBall->nPosX >> 8 < WALL_XMin + (s32)pBall->nRayon)
	{
		pBall->nAngle = 128 - pBall->nAngle;
		RetVal = 1;
	}

	// Mur Haut.
	if (pBall->nPosY >> 8 < WALL_YMin + (s32)pBall->nRayon)
	{
		pBall->nAngle = -pBall->nAngle;
		RetVal = 1;
	}

	return (RetVal);
}


// Traitement d'une brique (relachement d'items, etc...).
// Renvoie les flags de la brique. -1 si pas de choc.
u32 BrickHit(u32 nBx, u32 nBy, u32 nBallFlags)
{
	u32	nRetVal = (u32)-1;

	if (nBx < TABLE_Width && nBy < TABLE_Height && gBreak.pLevel[(nBy * TABLE_Width) + nBx].nPres)
	{
		struct SBrique	*pBrick = &gBreak.pLevel[(nBy * TABLE_Width) + nBx];

		// Décrémentation du compteur de touchés. Si tombe à 0, la brique disparaît.
		if ((pBrick->nFlags & BRICK_Flg_Indestructible) == 0)
		{
			pBrick->nCnt--;
			// Balle traversante, on force le compteur à 0.
			if (nBallFlags & BALL_Flg_Traversante) pBrick->nCnt = 0;
			if (pBrick->nCnt == 0)
			{
				pBrick->nPres = 0;
				AnmReleaseSlot(pBrick->nAnmNo);
				pBrick->nAnmNo = -1;	// Spécial pour les briques qui reviennent.

				// Dust.
				DustSet(pBrick->pAnmExplo, WALL_XMin + (nBx * BRICK_Width), WALL_YMin + (nBy * BRICK_Height));

				// Génération item.
				if ((rand() & 7) == 0)
				{
					u8	*pBon = gpBonuses[gBreak.nLevel];

					MstAdd(e_Mst_Pill_0 + pBon[rand() % 32],
						WALL_XMin + (nBx * BRICK_Width) + (BRICK_Width / 2), WALL_YMin + (nBy * BRICK_Height) + (BRICK_Height / 2));
				}

				// Score.
				gBreak.nPlayerScore += pBrick->nScore;

				gBreak.nRemainingBricks--;	// Une brique de moins.

				if (pBrick->nFlags & BRICK_Flg_ComingBack) gBreak.nBricksComingBackNbCur++;	// Nb de briques qui doivent revenir.
			}
			else
			{
				// Anim de hit.
				pBrick->nAnmNo = AnmSet(pBrick->pAnmHit, pBrick->nAnmNo);
			}
		}
		else
		{
			// Anim de hit de brique indestructible.
			pBrick->nAnmNo = AnmSet(pBrick->pAnmHit, pBrick->nAnmNo);
		}

		// Flags.
		nRetVal = (u32)pBrick->nFlags;
	}

	return (nRetVal);
}


// Collisions balle-briques.
// Renvoie 1 et retourne l'angle quand choc sur une brique.
u32 CollBricks(struct SBall *pBall, s32 *pnOldX, s32 *pnOldY)
{
	u32	RetVal = 0;

	s32	vx1, vx2, vy1, vy2;
	s32	nBXMin, nBXMax, nBYMin, nBYMax;
	s32	i, j;
	u32	x, y;
	s32	nX, nY;
	s32	dx = 0, dy = 0;

	u32	cx, cy, coin;
	u32	nBFlags;


	// Numéros des briques extrêmes à tester.
	vx1 = ((pBall->nPosX >> 8) - WALL_XMin - pBall->nRayon) / BRICK_Width;		// Note: testé avec une table à la place du div, on ne gagne rien !
	vx2 = ((pBall->nPosX >> 8) - WALL_XMin + pBall->nRayon) / BRICK_Width;
	vy1 = ((pBall->nPosY >> 8) - WALL_YMin - pBall->nRayon) / BRICK_Height;
	vy2 = ((pBall->nPosY >> 8) - WALL_YMin + pBall->nRayon) / BRICK_Height;

	cx = 0; cy = 0; coin = 0;
	// Boucle dans les briques potentielles.
	for (j = vy1; j <= vy2; j++)
	{
		// Coordonnées min et max de la brique en pixels.
		nBYMin = (j * BRICK_Height) + WALL_YMin;
		nBYMax = (j * BRICK_Height) + WALL_YMin + BRICK_Height - 1;

		for (i = vx1; i <= vx2; i++)
		{
			// Une brique présente ?
			if ((u32)i < TABLE_Width && (u32)j < TABLE_Height && gBreak.pLevel[(j * TABLE_Width) + i].nPres)
			{
				// Test de la col.

				// Coordonnées min et max de la brique en pixels.
				nBXMin = (i * BRICK_Width) + WALL_XMin;
				nBXMax = (i * BRICK_Width) + WALL_XMin + BRICK_Width - 1;

				// Test avec chaque pixel du masque de la balle.
				nY = (pBall->nPosY >> 8) - pBall->nRayon;
				for (y = 0; y < pBall->nDiam; y++)
				{
					nX = (pBall->nPosX >> 8) - pBall->nRayon;
					for (x = 0; x < pBall->nDiam; x++)
					{
						// Pixel à tester ?
						if (pBall->pBallMask[(y * BALL_GfxLg) + x])
						{
							if (nX >= nBXMin && nX <= nBXMax && nY >= nBYMin && nY <= nBYMax)
							{
								u32	nFlg = 1;	// b0

								// Gestion du choc sur la brique.
								nBFlags = BrickHit(i, j, pBall->nFlags);
								if (nBFlags & BRICK_Flg_Indestructible) nFlg = 2;	// b1

								// Choc.
								if (pBall->nPosX >> 8 >= nBXMin && pBall->nPosX >> 8 <= nBXMax)
								{
									cy |= nFlg;
								}
								else if (pBall->nPosY >> 8 >= nBYMin && pBall->nPosY >> 8 <= nBYMax)
								{
									cx |= nFlg;
								}
								else
								{
									coin |= nFlg;
									dx = (pBall->nPosX >> 8) - (nBXMin + (BRICK_Width / 2));
									dy = (pBall->nPosY >> 8) - (nBYMin + (BRICK_Height / 2));
								}

								y = 1000; x = 1000; break;	// Sortie des boucles.
							}
						}
						nX++;
					} // for (x = 0; x < pBall->nDiam; x++)
					nY++;
				} // for (y = 0; y < pBall->nDiam; y++)

			} // if brique présente
		} // for (i = vx1; i <= vx2; i++)
	} // for (j = vy1; j <= vy2; j++)


	// Balle traversante ?
	if (pBall->nFlags & BALL_Flg_Traversante)
	{
		// On cleare les b0 => On ne garde que les chocs forcés.
		cx &= ~1;
		cy &= ~1;
		coin &= ~1;
	}

	// Coin ?
	if (cx == 0 && cy == 0 && coin)
	{
		RetVal = 1;				// Et avec ça ? Ca coince encore ???

		if (dx >= 0 && dy >= 0)			// Bas droite.
		{
			if ((s8)pBall->nAngle >= -32 && (s8)pBall->nAngle < 64+32)
			{
				pBall->nAngle -= 64;
			}
			else
			{
				pBall->nAngle += 64;
			}
		}
		else if (dx >= 0 && dy <= 0)	// Haut droite.
		{
			if (pBall->nAngle >= 32 && pBall->nAngle < 128+32)
			{
				pBall->nAngle -= 64;
			}
			else
			{
				pBall->nAngle += 64;
			}
		}
		else if (dx <= 0 && dy <= 0)	// Haut gauche.
		{
			if (pBall->nAngle >= 64+32 && pBall->nAngle < 192+32)
			{
				pBall->nAngle -= 64;
			}
			else
			{
				pBall->nAngle += 64;
			}
		}
		else if (dx <= 0 && dy >= 0)	// Bas gauche.
		{
			if ((s8)pBall->nAngle >= -64-32 && (s8)pBall->nAngle < 32)
			{
				pBall->nAngle -= 64;
			}
			else
			{
				pBall->nAngle += 64;
			}
		}

		// En y, on empêche les rebonds trop "horizontaux".
		if ((s8)pBall->nAngle > -16 && (s8)pBall->nAngle < 16)
		{
			pBall->nAngle = ((s8)pBall->nAngle >= 0 ? 16 : -16);
		}
		else if (pBall->nAngle > 128-16 && pBall->nAngle < 128+16)
		{
			pBall->nAngle = (pBall->nAngle >= 128 ? 128+16 : 128-16);
		}

	}

	// Col X.
	if (cx)
	{
		pBall->nAngle = 128 - pBall->nAngle;
		RetVal = 1;

		// En x, on empêche les rebonds trop "verticaux". (Pour éviter de longer les briques en montant, etc...).
		if (pBall->nAngle > 64-16 && pBall->nAngle < 64+16)
		{
			pBall->nAngle = (pBall->nAngle >= 64 ? 64+16 : 64-16);
		}
		else if (pBall->nAngle > 192-16 && pBall->nAngle < 192+16)
		{
			pBall->nAngle = (pBall->nAngle >= 192 ? 192+16 : 192-16);
		}
	}

	// Col Y.
	if (cy)
	{
		pBall->nAngle = -pBall->nAngle;
		RetVal = 1;

		// En y, on empêche les rebonds trop "horizontaux".
		if ((s8)pBall->nAngle > -16 && (s8)pBall->nAngle < 16)
		{
			pBall->nAngle = ((s8)pBall->nAngle >= 0 ? 16 : -16);
		}
		else if (pBall->nAngle > 128-16 && pBall->nAngle < 128+16)
		{
			pBall->nAngle = (pBall->nAngle >= 128 ? 128+16 : 128-16);
		}
	}

	return (RetVal);

}


#define	BALL_DEPL_MAX	0x300		// Pas plus grand que le rayon mini d'une balle !
// Déplacement de la balle.
void Brk_MoveBall(void)
{
	s32	nAddX, nAddY;
	s32	nOldX, nOldY;

	s32	nRemSpd;
	s32	nSpd;

	struct SBall	*pBall;
	u32	i;

	for (i = 0; i < BALL_MAX_NB; i++)
	{
		pBall = &gBreak.pBalls[i];
		if (pBall->nUsed)
		{
			// La balle est-elle collée à la raquette ?
			if (pBall->nFlags & BALL_Flg_Aimantee)
			{
				// Balle collée sur la raquette.
				pBall->nPosX = (gBreak.nPlayerPosX + pBall->nOffsRaq) << 8;
				pBall->nPosY = (gBreak.nPlayerPosY - pBall->nRayon - 1) << 8;

				// Recalage si la balle passe dans les murs.
				// Mur droit.
				if (pBall->nPosX >> 8 > WALL_XMax - (s32)pBall->nRayon)
				{
					pBall->nOffsRaq -= (pBall->nPosX >> 8) - (WALL_XMax - (s32)pBall->nRayon);
					pBall->nPosX = (gBreak.nPlayerPosX + pBall->nOffsRaq) << 8;
				}
				// Mur gauche.
				if (pBall->nPosX >> 8 < WALL_XMin + (s32)pBall->nRayon)
				{
					pBall->nOffsRaq -= (pBall->nPosX >> 8) - (WALL_XMin + (s32)pBall->nRayon);
					pBall->nPosX = (gBreak.nPlayerPosX + pBall->nOffsRaq) << 8;
				}

			}
			else
			{
				// Balle en mouvement.



	nRemSpd = pBall->nSpeed;
	while (nRemSpd)
	{
		if (nRemSpd > BALL_DEPL_MAX)
		{
			// En cours.
			nSpd = BALL_DEPL_MAX;
			nRemSpd -= BALL_DEPL_MAX;
		}
		else
		{
			// Dernier tour.
			nSpd = nRemSpd;
			nRemSpd = 0;
		}


		// Déplacement.
		nOldX = pBall->nPosX;
		nOldY = pBall->nPosY;
		nAddX = (gVar.pCos[pBall->nAngle] * nSpd) >> 8;
		nAddY = (gVar.pSin[pBall->nAngle] * nSpd) >> 8;
		pBall->nPosX += nAddX;
		pBall->nPosY += nAddY;

		// Si col, on repart de l'ancienne pos.
		if (CollBricks(pBall, &nOldX, &nOldY))
		{
			jouer_son(collisionbrique1_sound);
			Balle_Acceleration(pBall, 30); // Acceleration de la balle
			pBall->nPosX = nOldX;
			pBall->nPosY = nOldY;
		}
		if (CollWalls(pBall, &nOldX, &nOldY))
		{
			jouer_son(collisionmur_sound);
			Balle_Acceleration(pBall, 30); // Acceleration de la balle
			pBall->nPosX = nOldX;
			pBall->nPosY = nOldY;
		}

		// Collision avec la raquette ?
//		if (nOldY < (gBreak.nPlayerPosY - (s32)pBall->nRayon) << 8 &&
//			pBall->nPosY >= (gBreak.nPlayerPosY - (s32)pBall->nRayon) << 8)

		if (pBall->nAngle >= 128 &&		// Test un peu plus permissif.
			pBall->nPosY >= (gBreak.nPlayerPosY - (s32)pBall->nRayon) << 8 &&
			pBall->nPosY <= (gBreak.nPlayerPosY + 4 - (s32)pBall->nRayon) << 8)

		if (SprCheckColBox(AnmGetLastImage(gBreak.nPlayerAnmNo), gBreak.nPlayerPosX, gBreak.nPlayerPosY,
			pBall->nSpr, pBall->nPosX >> 8, pBall->nPosY >> 8))
		{
			struct SSprite *pSpr = SprGetDesc(AnmGetLastImage(gBreak.nPlayerAnmNo));
			s32	nXMin, nXMax;

			nXMin = gBreak.nPlayerPosX - pSpr->nPtRefX;
			nXMax = nXMin + pSpr->nLg - 1;
			nXMin -= 4;		// Pour être un peu plus permissif.
			nXMax += 4;

			// Balle tape sur le côté de la raquette ?
			if (pBall->nPosX >> 8 < nXMin || pBall->nPosX >> 8 > nXMax)
			{
				if (pBall->nPosX >> 8 < gBreak.nPlayerPosX)
				{
					// Sur la gauche de la raquette.
					if (pBall->nAngle > 192) pBall->nAngle = 128 - pBall->nAngle;
				}
				else
				{
					// Sur la droite de la raquette.
					if (pBall->nAngle < 192) pBall->nAngle = 128 - pBall->nAngle;
				}

			}
			else
			{
				// La balle tape sur la raquette.
				s32	dx;
				s32	ang;
				
				Balle_Acceleration(pBall, 20);// Acceleration de la balle
				pBall->nPosX = nOldX;
				pBall->nPosY = nOldY;
				// Selon la position où la balle tombe sur la raquette, on renvoie la balle où il faut.
				dx = (pBall->nPosX >> 8) - gBreak.nPlayerPosX;

				// Renvoi à la Arkanoid, on ne prend pas en compte l'angle d'arrivée.
				ang = (-32 * dx) / ((s32)pSpr->nLg / 2);
//printf("dx = %d / ang = %d\n", (int)dx, (int)ang);
				ang += 64;
				pBall->nAngle = ang;

				// Raquette aimantée ?
				if (gBreak.nPlayerFlags & PLAYER_Flg_Aimant)
				{
					pBall->nOffsRaq = dx;
					pBall->nFlags |= BALL_Flg_Aimantee;	// Balle collée sur la raquette.
					//pBall->nSpeed = 0;	// No nO No ! La balle conserve sa vitesse.
					nRemSpd = 0;	// Balle collée, on arrête la boucle de déplacement.
				}

			}	// if côté
			jouer_son(collisionraquette_sound);
		}	// if collision raquette


	}	// while déplacement

	// La balle est tombée ?
	if (pBall->nPosY > (SCR_Height + (s32)pBall->nRayon) << 8)		// ? Mettre une constante à la place du rayon ?
	{
		pBall->nUsed = 0; gBreak.nBallsNb--;	// Nombre de balles en jeu.
	}


			}	// else balle aimantée

		}	// if ball used
	}	// for slots



	// Reste-t'il des balles en jeu ?
	if (gBreak.nBallsNb == 0)
	{
		PlayerSetDeath();
	}

}


// Relache les balles aimantées à la raquette.
void Aimant_ReleaseBalls(void)
{
	struct SBall	*pBall;
	u32	i;

	for (i = 0; i < BALL_MAX_NB; i++)
	{
		pBall = &gBreak.pBalls[i];
		if (pBall->nUsed)
		{
			pBall->nFlags &= ~BALL_Flg_Aimantee;	// Coupe l'aimant sur la balle.
		}
	}

}

// Déplacement de la raquette du joueur.

void raquette_a_droite(int vitesse)
{
	s32	j, nXMin, nXMax;
	struct SSprite *pSpr = SprGetDesc(AnmGetLastImage(gBreak.nPlayerAnmNo));
	j = gVar.nMousePosX;
	nXMin = j - pSpr->nPtRefX;
	nXMax = nXMin + pSpr->nLg;

	int i;
	for (i=0;i<vitesse;i++) 
	{ 
		if (nXMax < WALL_XMax)
		{
		gBreak.nPlayerPosX++;
		}
	}
}

void raquette_a_gauche(int vitesse)
{
	s32	j, nXMin, nXMax;
	struct SSprite *pSpr = SprGetDesc(AnmGetLastImage(gBreak.nPlayerAnmNo));
	j = gVar.nMousePosX;
	nXMin = j - pSpr->nPtRefX;
	nXMax = nXMin + pSpr->nLg;

	int i;
	for (i=0;i<vitesse;i++) 
	{ 
		if (nXMin > WALL_XMin)
		{
		gBreak.nPlayerPosX--;
		}
	}
}

void Brk_MovePlayer(void)
{
	s32	i;
	struct SSprite *pSpr = SprGetDesc(AnmGetLastImage(gBreak.nPlayerAnmNo));
	s32	nXMin, nXMax;


	// Dans le passage à droite ?
	if (gBreak.nPlayerFlags & PLAYER_Flg_DoorR)
	{
		// On lache les balles tout le temps. On ne s'occupe pas du flag => Ca simplifie la gestion.
		Aimant_ReleaseBalls();

		// On force le déplacement.
		gBreak.nPlayerPosX++;

		// Complètement passé ?
		if (gBreak.nPlayerPosX - pSpr->nPtRefX > WALL_XMax + 8)
		{
			gBreak.nPhase = e_Game_LevelCompleted;
		}

		// Dans le passage, plus de clics possibles.
		gVar.nMouseButtons = 0;
		clavier_actif = 0;

		return;
	}


	// Mort.
	if (AnmGetKey(gBreak.nPlayerAnmNo) == e_AnmKey_PlyrDeath)
	{	
		boucle_son_levelstart = 1;
		clavier_actif = 0;
		// Explosion finie ?
		if (AnmCheckEnd(gBreak.nPlayerAnmNo))
		{
			// Il reste des vies ?
			if (gBreak.nPlayerLives)
			{
				// Oui.
				gBreak.nPlayerLives--;
				// Reset joueur.
				Brk_PlyrInitLife();

				BallInitOnPlayer();		// balle, à faire apparaitre après l'anim d'apparition.
				// ou alors, pendant l'anim d'apparition, ne pas afficher de balle.

			}
			else
			{
				// Game over. (ou continue ?)
				gBreak.nPhase = e_Game_GameOver;
			}
		}

		// On ne bouge pas pendant la mort.
		gVar.nMouseButtons = 0;		// Empèche les clics (tirs...).
		return;
	}


	// On se prend un tir ? (Seulement dans le niveau du boss, c'est pour ça qu'on utilise la même routine que pour les monstres !).
	if (gBreak.nLevel == LEVEL_Max - 1)
	{
		if (MstCheckFire(AnmGetLastImage(gBreak.nPlayerAnmNo), gBreak.nPlayerPosX, gBreak.nPlayerPosY))
		{
			jouer_son(ennemieexplosion_sound);
			PlayerSetDeath();
			BallsInitSlots();	//todo: Revoir, comme ça pour éviter le bug du mort avec balle collée, reinit et gBreak.nNbBalles == 2, et on ne meurt plus quand on perd la balle.
		}

		// Si en attente du boss, pas de clics.
		if (gBreak.nPlayerFlags & PLAYER_Flg_BossWait)
		{
			clavier_actif = 0;
			gVar.nMouseButtons = 0;
		}
	}


//todo: éventuellement, faire une vitesse de dépl max. / si last - pos > vit max, pos = last pos +- vmax.

	// Déplacement de la raquette.
	//gBreak.nPlayerLastPosX = gBreak.nPlayerPosX;	// Finalement, on ne s'en sert pas.

	i = gVar.nMousePosX;
	nXMin = i - pSpr->nPtRefX;
	nXMax = nXMin + pSpr->nLg;
	if (nXMin < WALL_XMin) i = WALL_XMin + pSpr->nPtRefX;
	if (nXMax > WALL_XMax) i = WALL_XMax - pSpr->nLg + pSpr->nPtRefX;
	gBreak.nPlayerPosX = i;
//
//
//	Test du clavier
//
//
	
int vitesse;

	if (gVar.pKeys[SDLK_BACKSPACE]) // Accelerateur (Gachette Droite)
	{
		vitesse=8; // Vitesse Rapide
	} else {
		vitesse=4; // Vitesse Normale
	}

	if (gVar.pKeys[SDLK_RIGHT]) raquette_a_droite(vitesse);
	if (gVar.pKeys[SDLK_LEFT]) raquette_a_gauche(vitesse);
		
}


// Teste si une balle se trouve dans un rectangle (pour retour des briques qui reviennent).
u32 BallsCheckRectangle(s32 nXMin, s32 nXMax, s32 nYMin, s32 nYMax)
{
	u32	i;
	struct SBall	*pBall;

	for (i = 0; i < BALL_MAX_NB; i++)
	{
		pBall = &gBreak.pBalls[i];
		if (pBall->nUsed)
		{
			if (pBall->nPosX >= nXMin && pBall->nPosX <= nXMax &&
				pBall->nPosY >= nYMin && pBall->nPosY <= nYMax) return (1);
		}
	}

	return (0);
}

#define	BCB_Offset	16
// Gestion des briques qui reviennent.
void Brk_BricksComingBack(void)
{
	u32	i, j, k, nb;

	if (gBreak.nBricksComingBackTotal == 0) return;	// Si pas de briques qui reviennent dans le niveau.
	if (gBreak.nRemainingBricks + gBreak.nBricksComingBackNbCur <= gBreak.nBricksComingBackTotal) return;	// S'il ne reste que des briques invisibles.

	k = 0;
	nb = 0;
	for (j = 0; j < TABLE_Height; j++)
	for (i = 0; i < TABLE_Width; i++)
	{
		if (nb >= gBreak.nBricksComingBackNbCur) goto _Skip;
		if (gBreak.pLevel[k].nPres == 0 && (gBreak.pLevel[k].nFlags & BRICK_Flg_ComingBack))
		{
			// On a trouvé une brique qui doit revenir. Countdown.
			if (--gBreak.pLevel[k].nResetCnt == 0)
			{
				s32	nXMin, nYMin, nXMax, nYMax;

				// Coordonées à tester. Avec l'offset, englobe les rectangles de col des monstres et des balles. Ca permet de ne tester qu'un point.
				nXMin = ((i * BRICK_Width) + WALL_XMin - BCB_Offset) << 8;
				nXMax = ((i * BRICK_Width) + WALL_XMin + BRICK_Width - 1 + BCB_Offset) << 8;
				nYMin = ((j * BRICK_Height) + WALL_YMin - BCB_Offset) << 8;
				nYMax = ((j * BRICK_Height) + WALL_YMin + BRICK_Height - 1 + BCB_Offset) << 8;
				if (MstCheckRectangle(nXMin, nXMax, nYMin, nYMax) == 0 &&
					BallsCheckRectangle(nXMin, nXMax, nYMin, nYMax) == 0)
				{
					// Tout est ok, la brique revient.
					gBreak.pLevel[k].nPres = 1;		// La brique revient.
					gBreak.nRemainingBricks++;		// Une brique en plus.
					//
					gBreak.pLevel[k].nCnt = 2;		// Nb de touches restantes avant la destruction.
					gBreak.pLevel[k].nResetCnt = BRICK_ComingBackCnt;	// Compteur pour retour de la brique.
					nb++;
				}
				else
				{
					// La brique ne peut pas revenir tout de suite. On réésayera plus tard.
					gBreak.pLevel[k].nResetCnt = 8;
				}
			}
		}
		k++;	// idx
	}
_Skip:
	gBreak.nBricksComingBackNbCur -= nb;	// Décrémentation APRES la boucle.

}

// Game.
void BreakerGame(void)
{
	struct SSprite *pSpr;
	u32	i;
	s32	nDiff;
	static	u8	nWait = 0;

	switch (gBreak.nPhase)
	{
	case e_Game_SelectLevel:	// Selection du niveau.

		if (nWait) nWait--;
		if (nWait == 0)
		{
			nDiff = gVar.nMousePosX - gBreak.nPlayerPosX;
			if (ABS(nDiff) > 8)
			{
				i = 0;
				if (nDiff < 0)
				{
					if (gBreak.nLevel > 0) { gBreak.nLevel--; i = 1; }
				}
				else
				{
					if (gBreak.nLevel < LEVEL_SELECT_Max) { gBreak.nLevel++; i = 1; }
				}
				if (i)
				{
					BreakerInit();
					gBreak.nPhase = e_Game_SelectLevel;
					nWait = 12;
				}
			}
			if (gVar.pKeys[SDLK_RIGHT])
			{	
				i = 0;
				if (gBreak.nLevel < LEVEL_SELECT_Max) 
				{
					gBreak.nLevel++;
					i = 1;
				} 
				else 
				{
					gBreak.nLevel=0;
					i = 1;
				}
				if (i)
				{
					BreakerInit();
					gBreak.nPhase = e_Game_SelectLevel;
					nWait = 12;
					jouer_son(selectionstage_sound);
				}
			}
			if (gVar.pKeys[SDLK_LEFT])
			{
				i = 0;
				if (gBreak.nLevel > 0) 
				{
					gBreak.nLevel--;
					i = 1;
				} 
				else 
				{
					gBreak.nLevel=LEVEL_SELECT_Max;
					i = 1;
				}
				if (i)
				{
					BreakerInit();
					gBreak.nPhase = e_Game_SelectLevel;
					nWait = 12;
					jouer_son(selectionstage_sound);
				}
			}
		}

		// Affichage de la phrase de selection.
		{
			char	pStrSel[] = "SELECT STARTING LEVEL : 00";
			MyItoA(gBreak.nLevel + 1, &pStrSel[24]);
			i = Font_Print(0, 10, pStrSel, FONT_NoDisp);	// Pour centrage.
			Font_Print((SCR_Width / 2) - (i / 2), 200, pStrSel, 0);
		}

		// Clic souris ? => Selection du level.
		if (gVar.nMouseButtons & MOUSE_BtnLeft)
		{
			nWait = 0;
			gBreak.nPhase = e_Game_Normal;
		}
		// Appuie sur Start ou B -> Sélection du niveau
		if (gVar.pKeys[SDLK_RETURN] || gVar.pKeys[SDLK_LALT])
		{
			nWait = 10;
			gBreak.nPhase = e_Game_Normal;
		}
		break;

	case e_Game_Normal:			// Jeu.

		// Affichage de la phrase "Level xx" en début de vie.
		if (gBreak.nTimerLevelDisplay)
		{
			if (boucle_son_levelstart)
			{
			boucle_son_levelstart = 0;
			clavier_actif = 1;
			jouer_son(levelstart_sound);
			}
			char	pStrSel[] = "LEVEL 00";
			MyItoA(gBreak.nLevel + 1, &pStrSel[6]);
			i = Font_Print(0, 10, pStrSel, FONT_NoDisp);	// Pour centrage.
			Font_Print((SCR_Width / 2) - (i / 2), 200, pStrSel, 0);

			gBreak.nTimerLevelDisplay--;
			if (gVar.nMouseButtons & MOUSE_BtnLeft || gVar.pKeys[SDLK_LALT] || gVar.pKeys[SDLK_LCTRL] ) gBreak.nTimerLevelDisplay = 0;	// On coupe.
		}

		// Déplacement du joueur.
		Brk_MovePlayer();

		// Clic souris ?
		if (gVar.nMouseButtons & MOUSE_BtnLeft)
		{
			// Aimant ? (Sert aussi au lancement initial de la balle).
			Aimant_ReleaseBalls();

			// Mitrailleuse ?
			if (gBreak.nPlayerFlags & PLAYER_Flg_Mitrailleuse)
			{
				gBreak.nPlayerAnmBonusD = AnmSetIfNew(gAnm_RaqMitDShoot, gBreak.nPlayerAnmBonusD);
				gBreak.nPlayerAnmBonusG = AnmSetIfNew(gAnm_RaqMitGShoot, gBreak.nPlayerAnmBonusG);
				// Balance les tirs.
				pSpr = SprGetDesc(AnmGetLastImage(gBreak.nPlayerAnmNo));
				FireAdd(0, gBreak.nPlayerPosX - pSpr->nPtRefX + 2, gBreak.nPlayerPosY - 2, -1);
				FireAdd(0, gBreak.nPlayerPosX - pSpr->nPtRefX + pSpr->nLg - 1 - 2, gBreak.nPlayerPosY - 2, -1);
			}

		}	// clic
		if (nWait) nWait--;
		if (gVar.pKeys[SDLK_LCTRL] & (nWait==0) & clavier_actif|| gVar.pKeys[SDLK_LALT] & (nWait==0) & clavier_actif)
		{
			nWait=10;
			// Aimant ? (Sert aussi au lancement initial de la balle).
			Aimant_ReleaseBalls();

			// Mitrailleuse ?
			if (gBreak.nPlayerFlags & PLAYER_Flg_Mitrailleuse)
			{
				jouer_son(tir_sound);
				gBreak.nPlayerAnmBonusD = AnmSetIfNew(gAnm_RaqMitDShoot, gBreak.nPlayerAnmBonusD);
				gBreak.nPlayerAnmBonusG = AnmSetIfNew(gAnm_RaqMitGShoot, gBreak.nPlayerAnmBonusG);
				// Balance les tirs.
				pSpr = SprGetDesc(AnmGetLastImage(gBreak.nPlayerAnmNo));
				FireAdd(0, gBreak.nPlayerPosX - pSpr->nPtRefX + 2, gBreak.nPlayerPosY - 2, -1);
				FireAdd(0, gBreak.nPlayerPosX - pSpr->nPtRefX + pSpr->nLg - 1 - 2, gBreak.nPlayerPosY - 2, -1);
			}

		}
		
		if (gVar.pKeys[SDLK_ESCAPE])
		{
			gBreak.nPhase = e_Game_GameOver;
		}

		Brk_MoveBall();

		// Gestion des briques qui reviennent.
		Brk_BricksComingBack();

		// Plus de briques dans le niveau ?
		if (gBreak.nRemainingBricks == 0)
		{
			// On déclenche l'ouverture de la porte.
			MstDoorROpen();
			// Le joueur peut continuer pour buter les monstres si ça le chante, mais ne peut plus mourir.
			gBreak.nPlayerFlags |= PLAYER_Flg_NoKill;
		}

		break;

	case e_Game_GameOver:		// Game Over.
		// Affichage du Game Over.
		{
			if (boucle_son_gameover)
			{
				boucle_son_gameover = 0;
				jouer_son(gameover_sound);
			}
			char	pGameOver[] = "GAME OVER";
			i = Font_Print(0, 10, pGameOver, FONT_NoDisp);	// Pour centrage.
			Font_Print((SCR_Width / 2) - (i / 2), 200, pGameOver, 0);
		}
		// On quitte après x secondes || Clic, on quitte tout de suite. || B, on quitte tour de suite
		if (--gBreak.nTimerGameOver == 0 || (gVar.nMouseButtons & MOUSE_BtnLeft) || gVar.pKeys[SDLK_RETURN] || gVar.pKeys[SDLK_LALT] || gVar.pKeys[SDLK_LCTRL])
		{
			Mix_FreeChunk( gameover_sound );
			ExgExit(e_Game_GameOver);
		}
		break;

	case e_Game_LevelCompleted:		// Niveau terminé.
		Mix_FreeChunk( bossexplosion_sound );
		Mix_FreeChunk( intro_music_sound );
		Mix_FreeMusic( victory_musique );
		clavier_actif = 1;
		if (gBreak.nLevel++ < LEVEL_Max - 1)
		{
			// Niveau suivant.
			BreakerInit();
			gBreak.nPhase = e_Game_Normal;
		}
		else
		{
			// Jeu terminé. Sortie.
			ExgExit(e_Game_AllClear);
		}
		break;

	case e_Game_Pause:			// Pause.
		// Normalement, on ne passe jamais ici...
		break;

	}

	// Replace la souris à l'endroit du joueur.
	SDL_WarpMouse(gBreak.nPlayerPosX, gBreak.nPlayerPosY);


}


// +1 vie à certains scores.
#define	SC_EVERY	100000
void CheckSpecialScore(u32 nLastScore)
{
	u32	pScores[] = { 20000, 50000, 100000 };
	static u32	nNextScore;		// Pour les scores au dela de 100000.
	u32	i;

	// Au delà du score bonus max ? Alors 1 vie tous les x points.
	if (nLastScore >= pScores[NBELEM(pScores) - 1])
	{
		if (gBreak.nPlayerScore >= nNextScore)
		{
			BreakerBonus1Up();
			// Le score suivant.
			nNextScore += SC_EVERY;
		}
		return;
	}

	// Recherche dans le tableau.
	i = 0;
	while (nLastScore >= pScores[i] && i < NBELEM(pScores)) i++;
	// +1 vie ?
	if (nLastScore < pScores[i] && gBreak.nPlayerScore >= pScores[i])
	{
		BreakerBonus1Up();
	}

	// Premier score en dehors du tableau.
	nNextScore = pScores[NBELEM(pScores) - 1] + SC_EVERY;

}


// Breaker.
void Breaker(void)
{
	u32	nLastScore = gBreak.nPlayerScore;

	if (gBreak.nPhase == e_Game_Pause) return;
	BreakerGame();
	FireManage();
	MstManage();
	CheckSpecialScore(nLastScore);
	DustManage();
	BreakerDraw();

}






