#include "includes.h"


// 1 u32 : 1 clef.
// Ensuite :
// 1 u32 : Nb de frames d'affichage
//  Si b31, code de contrôle : Voir liste en haut de anims.h.
// 1 u32 : N° du sprite.


// Raquette joueur.
u32	gAnm_Raquette[] =
{
e_AnmKey_Null,
65000, e_Spr_Raquette,
e_Anm_Jump, (u32)-2
//e_Anm_Goto, (u32)gAnm_Raquette
};

// Raquette, apparition.
u32	gAnm_RaqAppear[] =
{
e_AnmKey_PlyrAppear,
4, e_Spr_RaquetteApparition,
4, e_Spr_RaquetteApparition + 1,
4, e_Spr_RaquetteApparition + 2,
4, e_Spr_RaquetteApparition + 3,
4, e_Spr_RaquetteApparition + 4,
4, e_Spr_RaquetteApparition + 5,
4, e_Spr_RaquetteApparition + 6,
4, e_Spr_RaquetteApparition + 7,
4, e_Spr_RaquetteApparition + 8,
4, e_Spr_RaquetteApparition + 9,
4, e_Spr_RaquetteApparition + 10,
4, e_Spr_RaquetteApparition + 11,
4, e_Spr_RaquetteApparition + 12,
4, e_Spr_RaquetteApparition + 13,
e_Anm_Goto, (u32)gAnm_Raquette
};

// Raquette, aimant.
u32	gAnm_RaqAimant[] =
{
e_AnmKey_Null,
15, e_Spr_RaquetteAimant,
8, e_Spr_RaquetteAimant+1,
8, e_Spr_RaquetteAimant+2,
8, e_Spr_RaquetteAimant+1,
e_Anm_Goto, (u32)gAnm_RaqAimant
};

// Joueur, mort (sz 0).
u32	gAnm_RaqDeath0[] =
{
e_AnmKey_PlyrDeath,
5, e_Spr_RaquetteDeathSz0,
5, e_Spr_RaquetteDeathSz0 + 1,
5, e_Spr_RaquetteDeathSz0 + 2,
5, e_Spr_RaquetteDeathSz0 + 3,
5, e_Spr_RaquetteDeathSz0 + 4,
5, e_Spr_RaquetteDeathSz0 + 5,
5, e_Spr_RaquetteDeathSz0 + 6,
5, e_Spr_RaquetteDeathSz0 + 7,
e_Anm_End
};

// Joueur, mort (sz 1).
u32	gAnm_RaqDeath1[] =
{
e_AnmKey_PlyrDeath,
5, e_Spr_RaquetteDeathSz1,
5, e_Spr_RaquetteDeathSz1 + 1,
5, e_Spr_RaquetteDeathSz1 + 2,
5, e_Spr_RaquetteDeathSz1 + 3,
5, e_Spr_RaquetteDeathSz1 + 4,
5, e_Spr_RaquetteDeathSz1 + 5,
5, e_Spr_RaquetteDeathSz1 + 6,
5, e_Spr_RaquetteDeathSz1 + 7,
e_Anm_End
};

// Joueur, mort (sz 2).
u32	gAnm_RaqDeath2[] =
{
e_AnmKey_PlyrDeath,
5, e_Spr_RaquetteDeathSz2,
5, e_Spr_RaquetteDeathSz2 + 1,
5, e_Spr_RaquetteDeathSz2 + 2,
5, e_Spr_RaquetteDeathSz2 + 3,
5, e_Spr_RaquetteDeathSz2 + 4,
5, e_Spr_RaquetteDeathSz2 + 5,
5, e_Spr_RaquetteDeathSz2 + 6,
5, e_Spr_RaquetteDeathSz2 + 7,
e_Anm_End
};

// Joueur, mort (sz 3).
u32	gAnm_RaqDeath3[] =
{
e_AnmKey_PlyrDeath,
5, e_Spr_RaquetteDeathSz3,
5, e_Spr_RaquetteDeathSz3 + 1,
5, e_Spr_RaquetteDeathSz3 + 2,
5, e_Spr_RaquetteDeathSz3 + 3,
5, e_Spr_RaquetteDeathSz3 + 4,
5, e_Spr_RaquetteDeathSz3 + 5,
5, e_Spr_RaquetteDeathSz3 + 6,
5, e_Spr_RaquetteDeathSz3 + 7,
e_Anm_End
};

// Raquette, se rallonge (0).
u32	gAnm_RaqRallonge0[] =
{
e_AnmKey_Null,
2, e_Spr_RaquetteRallonge0,
2, e_Spr_RaquetteRallonge0+1,
2, e_Spr_RaquetteRallonge0+2,
2, e_Spr_RaquetteRallonge0+3,
2, e_Spr_RaquetteRallonge0+4,
2, e_Spr_RaquetteRallonge0+5,
65000, e_Spr_Raquette,
e_Anm_Jump, (u32)-2
};

// Raquette, se réduit (0).
u32	gAnm_RaqReduit0[] =
{
e_AnmKey_Null,
2, e_Spr_Raquette,
2, e_Spr_RaquetteRallonge0+5,
2, e_Spr_RaquetteRallonge0+4,
2, e_Spr_RaquetteRallonge0+3,
2, e_Spr_RaquetteRallonge0+2,
2, e_Spr_RaquetteRallonge0+1,
65000, e_Spr_RaquetteRallonge0,
e_Anm_Jump, (u32)-2
};

// Raquette, se rallonge (1).
u32	gAnm_RaqRallonge1[] =
{
e_AnmKey_Null,
2, e_Spr_Raquette,
2, e_Spr_RaquetteRallonge1,
2, e_Spr_RaquetteRallonge1+1,
2, e_Spr_RaquetteRallonge1+2,
2, e_Spr_RaquetteRallonge1+3,
2, e_Spr_RaquetteRallonge1+4,
65000, e_Spr_RaquetteRallonge1+5,
e_Anm_Jump, (u32)-2
};

// Raquette, se réduit (1).
u32	gAnm_RaqReduit1[] =
{
e_AnmKey_Null,
2, e_Spr_RaquetteRallonge1+5,
2, e_Spr_RaquetteRallonge1+4,
2, e_Spr_RaquetteRallonge1+3,
2, e_Spr_RaquetteRallonge1+2,
2, e_Spr_RaquetteRallonge1+1,
2, e_Spr_RaquetteRallonge1,
65000, e_Spr_Raquette,
e_Anm_Jump, (u32)-2
};

// Raquette, se rallonge (2).
u32	gAnm_RaqRallonge2[] =
{
e_AnmKey_Null,
2, e_Spr_RaquetteRallonge1+5,
2, e_Spr_RaquetteRallonge2,
2, e_Spr_RaquetteRallonge2+1,
2, e_Spr_RaquetteRallonge2+2,
2, e_Spr_RaquetteRallonge2+3,
2, e_Spr_RaquetteRallonge2+4,
65000, e_Spr_RaquetteRallonge2+5,
e_Anm_Jump, (u32)-2
};

// Raquette, se réduit (2).
u32	gAnm_RaqReduit2[] =
{
e_AnmKey_Null,
2, e_Spr_RaquetteRallonge2+5,
2, e_Spr_RaquetteRallonge2+4,
2, e_Spr_RaquetteRallonge2+3,
2, e_Spr_RaquetteRallonge2+2,
2, e_Spr_RaquetteRallonge2+1,
2, e_Spr_RaquetteRallonge2,
65000, e_Spr_RaquetteRallonge1+5,
e_Anm_Jump, (u32)-2
};

// Raquette, mitrailleuse gauche, repos.
u32	gAnm_RaqMitGRepos[] =
{
e_AnmKey_Null,
65000, e_Spr_RaquetteMitrG,
e_Anm_Jump, (u32)-2
};

// Raquette, mitrailleuse droite, repos.
u32	gAnm_RaqMitDRepos[] =
{
e_AnmKey_Null,
65000, e_Spr_RaquetteMitrD,
e_Anm_Jump, (u32)-2
};

// Raquette, mitrailleuse gauche, tir.
u32	gAnm_RaqMitGShoot[] =
{
e_AnmKey_Null,
2, e_Spr_RaquetteMitrG+1,
2, e_Spr_RaquetteMitrG+2,
2, e_Spr_RaquetteMitrG+3,
e_Anm_Goto, (u32)gAnm_RaqMitGRepos
};

// Raquette, mitrailleuse droite, tir.
u32	gAnm_RaqMitDShoot[] =
{
e_AnmKey_Null,
2, e_Spr_RaquetteMitrD+1,
2, e_Spr_RaquetteMitrD+2,
2, e_Spr_RaquetteMitrD+3,
e_Anm_Goto, (u32)gAnm_RaqMitDRepos
};

// Tir mitraillette joueur.
u32	gAnm_PlyrShot[] =
{
e_AnmKey_Null,
65000, e_Spr_PlyrShot,
e_Anm_Jump, (u32)-2
};


// Raquette, clignotant gauche.
u32	gAnm_RaqClignG[] =
{
e_AnmKey_Null,
3, e_Spr_RaqClignG,
3, e_Spr_RaqClignG + 1,
3, e_Spr_RaqClignG + 2,
3, e_Spr_RaqClignG + 3,
3, e_Spr_RaqClignG + 4,
3, e_Spr_RaqClignG + 3,
3, e_Spr_RaqClignG + 2,
3, e_Spr_RaqClignG + 1,
e_Anm_Goto, (u32)gAnm_RaqClignG
};

// Raquette, clignotant droit.
u32	gAnm_RaqClignD[] =
{
e_AnmKey_Null,
3, e_Spr_RaqClignD,
3, e_Spr_RaqClignD + 1,
3, e_Spr_RaqClignD + 2,
3, e_Spr_RaqClignD + 3,
3, e_Spr_RaqClignD + 4,
3, e_Spr_RaqClignD + 3,
3, e_Spr_RaqClignD + 2,
3, e_Spr_RaqClignD + 1,
e_Anm_Goto, (u32)gAnm_RaqClignD
};


// Disparition d'une brique normale.
u32	gAnm_BrickExplo[] =
{
e_AnmKey_Null,
4, (e_Spr_BricksExplo | SPR_Flag_Shadow),
2, (e_Spr_BricksExplo | SPR_Flag_Shadow)+1,
2, (e_Spr_BricksExplo | SPR_Flag_Shadow)+2,
2, (e_Spr_BricksExplo | SPR_Flag_Shadow)+3,
2, (e_Spr_BricksExplo | SPR_Flag_Shadow)+4,
2, (e_Spr_BricksExplo | SPR_Flag_Shadow)+5,
2, (e_Spr_BricksExplo | SPR_Flag_Shadow)+6,
2, (e_Spr_BricksExplo | SPR_Flag_Shadow)+7,
2, (e_Spr_BricksExplo | SPR_Flag_Shadow)+8,
2, (e_Spr_BricksExplo | SPR_Flag_Shadow)+9,
e_Anm_Kill, 0
};

// Disparition d'une brique à taper 2 fois.
u32	gAnm_Brick2HitExplo[] =
{
e_AnmKey_Null,
4, (e_Spr_Bricks2HitExplo | SPR_Flag_Shadow),
2, (e_Spr_Bricks2HitExplo | SPR_Flag_Shadow)+1,
2, (e_Spr_Bricks2HitExplo | SPR_Flag_Shadow)+2,
2, (e_Spr_Bricks2HitExplo | SPR_Flag_Shadow)+3,
2, (e_Spr_Bricks2HitExplo | SPR_Flag_Shadow)+4,
2, (e_Spr_Bricks2HitExplo | SPR_Flag_Shadow)+5,
2, (e_Spr_Bricks2HitExplo | SPR_Flag_Shadow)+6,
2, (e_Spr_Bricks2HitExplo | SPR_Flag_Shadow)+7,
2, (e_Spr_Bricks2HitExplo | SPR_Flag_Shadow)+8,
2, (e_Spr_Bricks2HitExplo | SPR_Flag_Shadow)+9,
e_Anm_Kill, 0
};

// Disparition d'une brique qui revient.
u32	gAnm_BrickCBExplo[] =
{
e_AnmKey_Null,
4, (e_Spr_BricksCBExplo | SPR_Flag_Shadow),
2, (e_Spr_BricksCBExplo | SPR_Flag_Shadow)+1,
2, (e_Spr_BricksCBExplo | SPR_Flag_Shadow)+2,
2, (e_Spr_BricksCBExplo | SPR_Flag_Shadow)+3,
2, (e_Spr_BricksCBExplo | SPR_Flag_Shadow)+4,
2, (e_Spr_BricksCBExplo | SPR_Flag_Shadow)+5,
2, (e_Spr_BricksCBExplo | SPR_Flag_Shadow)+6,
2, (e_Spr_BricksCBExplo | SPR_Flag_Shadow)+7,
2, (e_Spr_BricksCBExplo | SPR_Flag_Shadow)+8,
2, (e_Spr_BricksCBExplo | SPR_Flag_Shadow)+9,
e_Anm_Kill, 0
};

// Brique à taper 2 fois, hit.
u32	gAnm_Brick2Hit[] =
{
e_AnmKey_Null,
2, e_Spr_Brick2Hit,
2, e_Spr_Brick2Hit+1,
2, e_Spr_Brick2Hit+2,
2, e_Spr_Brick2Hit+3,
2, e_Spr_Brick2Hit+4,
2, e_Spr_Brick2Hit+5,
e_Anm_Kill, 0
};

// Brique qui revient, hit.
u32	gAnm_BrickCBHit[] =
{
e_AnmKey_Null,
2, e_Spr_BrickCBHit,
2, e_Spr_BrickCBHit+1,
2, e_Spr_BrickCBHit+2,
2, e_Spr_BrickCBHit+3,
2, e_Spr_BrickCBHit+4,
2, e_Spr_BrickCBHit+5,
e_Anm_Kill, 0
};

// Brique indestructible, hit.
u32	gAnm_BrickIndesHit[] =
{
e_AnmKey_Null,
2, e_Spr_BrickIndesHit,
2, e_Spr_BrickIndesHit+1,
2, e_Spr_BrickIndesHit+2,
2, e_Spr_BrickIndesHit+3,
2, e_Spr_BrickIndesHit+4,
2, e_Spr_BrickIndesHit+5,
e_Anm_Kill, 0
};


// Animations des items (pillules qui descendent).
u32	gAnm_Itm1[] =
{
e_AnmKey_Null,
16, e_Spr_Itm1,
3, e_Spr_Itm1+1,
3, e_Spr_Itm1+2,
3, e_Spr_Itm1+3,
3, e_Spr_Itm1+4,
e_Anm_Goto, (u32)gAnm_Itm1
};

u32	gAnm_Itm2[] =
{
e_AnmKey_Null,
16, e_Spr_Itm2,
3, e_Spr_Itm2+1,
3, e_Spr_Itm2+2,
3, e_Spr_Itm2+3,
3, e_Spr_Itm2+4,
e_Anm_Goto, (u32)gAnm_Itm2
};

u32	gAnm_Itm3[] =
{
e_AnmKey_Null,
16, e_Spr_Itm3,
3, e_Spr_Itm3+1,
3, e_Spr_Itm3+2,
3, e_Spr_Itm3+3,
3, e_Spr_Itm3+4,
e_Anm_Goto, (u32)gAnm_Itm3
};

u32	gAnm_Itm4[] =
{
e_AnmKey_Null,
16, e_Spr_Itm4,
3, e_Spr_Itm4+1,
3, e_Spr_Itm4+2,
3, e_Spr_Itm4+3,
3, e_Spr_Itm4+4,
e_Anm_Goto, (u32)gAnm_Itm4
};

u32	gAnm_Itm5[] =
{
e_AnmKey_Null,
16, e_Spr_Itm5,
3, e_Spr_Itm5+1,
3, e_Spr_Itm5+2,
3, e_Spr_Itm5+3,
3, e_Spr_Itm5+4,
e_Anm_Goto, (u32)gAnm_Itm5
};

u32	gAnm_Itm6[] =
{
e_AnmKey_Null,
16, e_Spr_Itm6,
3, e_Spr_Itm6+1,
3, e_Spr_Itm6+2,
3, e_Spr_Itm6+3,
3, e_Spr_Itm6+4,
e_Anm_Goto, (u32)gAnm_Itm6
};

u32	gAnm_Itm7[] =
{
e_AnmKey_Null,
16, e_Spr_Itm7,
3, e_Spr_Itm7+1,
3, e_Spr_Itm7+2,
3, e_Spr_Itm7+3,
3, e_Spr_Itm7+4,
e_Anm_Goto, (u32)gAnm_Itm7
};

u32	gAnm_Itm8[] =
{
e_AnmKey_Null,
16, e_Spr_Itm8,
3, e_Spr_Itm8+1,
3, e_Spr_Itm8+2,
3, e_Spr_Itm8+3,
3, e_Spr_Itm8+4,
e_Anm_Goto, (u32)gAnm_Itm8
};

u32	gAnm_Itm9[] =
{
e_AnmKey_Null,
16, e_Spr_Itm9,
3, e_Spr_Itm9+1,
3, e_Spr_Itm9+2,
3, e_Spr_Itm9+3,
3, e_Spr_Itm9+4,
e_Anm_Goto, (u32)gAnm_Itm9
};

u32	gAnm_Itm10[] =
{
e_AnmKey_Null,
16, e_Spr_Itm10,
3, e_Spr_Itm10+1,
3, e_Spr_Itm10+2,
3, e_Spr_Itm10+3,
3, e_Spr_Itm10+4,
e_Anm_Goto, (u32)gAnm_Itm10
};

u32	gAnm_Itm11[] =
{
e_AnmKey_Null,
16, e_Spr_Itm11,
3, e_Spr_Itm11+1,
3, e_Spr_Itm11+2,
3, e_Spr_Itm11+3,
3, e_Spr_Itm11+4,
e_Anm_Goto, (u32)gAnm_Itm11
};


// Monstres : Ouverture de la porte.
u32	gAnm_MstDoorOpen[] =
{
e_AnmKey_Null,
3, e_Spr_SortieMst,
3, e_Spr_SortieMst + 1,
3, e_Spr_SortieMst + 2,
e_Anm_Goto, (u32)gAnm_MstDoorOpened
};
// Monstres : Porte ouverte (monstre sort).
u32	gAnm_MstDoorOpened[] =
{
1,//e_AnmKey_Null,
30, e_Spr_SortieMst + 2,
e_Anm_Goto, (u32)gAnm_MstDoorClose
};
// Monstres : Fermeture de la porte.
u32	gAnm_MstDoorClose[] =
{
2,//e_AnmKey_Null,
3, e_Spr_SortieMst + 2,
3, e_Spr_SortieMst + 1,
3, e_Spr_SortieMst,
e_Anm_Goto, (u32)gAnm_MstDoorWait
};
u32	gAnm_MstDoorWait[] =
{
e_AnmKey_Null,
65000, e_Spr_SortieMst,
e_Anm_Jump, (u32)-2
};


// Monstres des niveaux 0.
u32	gAnm_Mst1[] =
{
e_AnmKey_Null,
5, e_Spr_Mst1,
5, e_Spr_Mst1 + 1,
5, e_Spr_Mst1 + 2,
5, e_Spr_Mst1 + 3,
5, e_Spr_Mst1 + 4,
5, e_Spr_Mst1 + 5,
5, e_Spr_Mst1 + 6,
5, e_Spr_Mst1 + 7,
e_Anm_Goto, (u32)gAnm_Mst1
};

// Monstres des niveaux 1.
u32	gAnm_Mst2[] =
{
e_AnmKey_Null,
5, e_Spr_Mst2,
5, e_Spr_Mst2 + 1,
5, e_Spr_Mst2 + 2,
5, e_Spr_Mst2 + 3,
5, e_Spr_Mst2 + 4,
5, e_Spr_Mst2 + 5,
5, e_Spr_Mst2 + 6,
5, e_Spr_Mst2 + 7,
5, e_Spr_Mst2 + 8,
5, e_Spr_Mst2 + 9,
5, e_Spr_Mst2 + 10,
5, e_Spr_Mst2 + 11,
e_Anm_Goto, (u32)gAnm_Mst2
};

// Monstres des niveaux 2.
u32	gAnm_Mst3[] =
{
e_AnmKey_Null,
15, e_Spr_Mst3,		5, e_Spr_Mst3 + 1,		5, e_Spr_Mst3 + 2,		5, e_Spr_Mst3 + 3,
5, e_Spr_Mst3 + 4,	5, e_Spr_Mst3 + 5,		5, e_Spr_Mst3 + 6,		5, e_Spr_Mst3 + 7,
5, e_Spr_Mst3 + 8,
15, e_Spr_Mst3 + 9,	5, e_Spr_Mst3 + 10,		5, e_Spr_Mst3 + 11,		5, e_Spr_Mst3 + 12,
5, e_Spr_Mst3 + 13,	5, e_Spr_Mst3 + 14,		5, e_Spr_Mst3 + 15,
15, e_Spr_Mst3 + 16,	5, e_Spr_Mst3 + 17,		5, e_Spr_Mst3 + 18,		5, e_Spr_Mst3 + 19,
5, e_Spr_Mst3 + 20,		5, e_Spr_Mst3 + 21,		5, e_Spr_Mst3 + 22,		5, e_Spr_Mst3 + 23,
e_Anm_Goto, (u32)gAnm_Mst3
};

// Monstres des niveaux 3.
u32	gAnm_Mst4[] =
{
e_AnmKey_Null,
5, e_Spr_Mst4,
5, e_Spr_Mst4 + 1,
5, e_Spr_Mst4 + 2,
5, e_Spr_Mst4 + 3,
5, e_Spr_Mst4 + 4,
5, e_Spr_Mst4 + 5,
5, e_Spr_Mst4 + 6,
5, e_Spr_Mst4 + 7,
5, e_Spr_Mst4 + 8,
5, e_Spr_Mst4 + 9,
5, e_Spr_Mst4 + 10,
e_Anm_Goto, (u32)gAnm_Mst4
};

// Explosion générique pour les monstres.
u32	gAnm_MstExplo1[] =
{
e_AnmKey_Null,
3, e_Spr_MstExplo1,
3, e_Spr_MstExplo1 + 1,
3, e_Spr_MstExplo1 + 2,
3, e_Spr_MstExplo1 + 3,
3, e_Spr_MstExplo1 + 4,
3, e_Spr_MstExplo1 + 5,
3, e_Spr_MstExplo1 + 6,
e_Anm_Kill, 0
};

// Disparition du tir du boss.
u32	gAnm_DohMissileDisp[] =
{
e_AnmKey_Null,
3, e_Spr_DohMisDust,
3, e_Spr_DohMisDust + 1,
3, e_Spr_DohMisDust + 2,
3, e_Spr_DohMisDust + 3,
e_Anm_Kill, 0
};


// Monstre: Porte latérale ouverte vers le niveau suivant.
u32	gAnm_MstDoorRight[] =
{
e_AnmKey_Null,
3, e_Spr_MstPorteLevel,
3, e_Spr_MstPorteLevel + 2,
3, e_Spr_MstPorteLevel + 4,
3, e_Spr_MstPorteLevel + 6,
3, e_Spr_MstPorteLevel + 8,
3, e_Spr_MstPorteLevel + 10,
3, e_Spr_MstPorteLevel + 12,
3, e_Spr_MstPorteLevel + 14,
3, e_Spr_MstPorteLevel + 16,
3, e_Spr_MstPorteLevel + 18,
e_Anm_Goto, (u32)gAnm_MstDoorRight
};


// Doh: Idle.
u32	gAnm_MstDohIdle[] =
{
e_AnmKey_Null,
256, e_Spr_DohIdle,
e_Anm_Jump, (u32)-2
};

// Doh: Appears.
u32	gAnm_MstDohAppears[] =
{
e_AnmKey_MstDohAppears,
16, e_Spr_DohDisappear + 4,
16, e_Spr_DohDisappear + 3,
16, e_Spr_DohDisappear + 2,
16, e_Spr_DohDisappear + 1,
16, e_Spr_DohDisappear,
e_Anm_Goto, (u32)gAnm_MstDohIdle
};

// Doh: Disappears.
u32	gAnm_MstDohDisappears[] =
{
e_AnmKey_MstDohDisappears,
25, e_Spr_DohIdle,			// Pour laisser le temps aux explosions de disparaître.
16, e_Spr_DohDisappear,
16, e_Spr_DohDisappear + 1,
16, e_Spr_DohDisappear + 2,
16, e_Spr_DohDisappear + 3,
16, e_Spr_DohDisappear + 4,
8, SPR_NoSprite,
e_Anm_Jump, (u32)-2
//e_Anm_End, 0
};

// Doh: Pour finir le hit, spr de hit avec clef null.
u32	gAnm_MstDohHit2[] =
{
e_AnmKey_Null,
8, e_Spr_DohHit,
e_Anm_Goto, (u32)gAnm_MstDohHit2
};
// Doh: Hit.
u32	gAnm_MstDohHit[] =
{
e_AnmKey_MstDohHit,
8, e_Spr_DohHit,
e_Anm_Goto, (u32)gAnm_MstDohHit2
};

// Doh: Tir.
u32	gAnm_MstDohShoot[] =
{
e_AnmKey_Null,
256, e_Spr_DohShoot,
e_Anm_Jump, (u32)-2
};

// Doh: Ouvre la bouche.
u32	gAnm_MstDohMouthOpens[] =
{
e_AnmKey_MstDohMoutOpens,
10, e_Spr_DohIdle,
e_Anm_Goto, (u32)gAnm_MstDohShoot
};

// Doh: Ferme la bouche.
u32	gAnm_MstDohMouthCloses[] =
{
e_AnmKey_MstDohMoutCloses,
10, e_Spr_DohShoot,
e_Anm_Goto, (u32)gAnm_MstDohIdle
};

// Tir de Doh.
u32	gAnm_DohMissile[] =
{
e_AnmKey_Null,
4, e_Spr_DohMissile,
2, e_Spr_DohMissile+1,
2, e_Spr_DohMissile+2,
4, e_Spr_DohMissile+3,
2, e_Spr_DohMissile+2,
2, e_Spr_DohMissile+1,
e_Anm_Goto, (u32)gAnm_DohMissile
};


