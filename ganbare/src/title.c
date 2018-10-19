#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include "define.h"
#include "function.h"
#include "util_snd.h"
#include "extern.h"
#include "title.h" 

#include "refresh.h"

void title_main( void );
void title_init( void );
void title_relese( void );
void title_keys( void );
void title_drow( void );
void title_init_save_data( void );
void title_kane_set( int x, int y );
void title_kane_disp( void );
void title_k_jmp( int i );
int replay_file_find( void );
int replay_file_find2( void );

enum
{
	EN_TITLE_NONE = 0,
	EN_TITLE_image1,
	EN_TITLE_image2,
	EN_TITLE_image3,
	EN_TITLE_image4,
	EN_TITLE_image5,
	EN_TITLE_image6,
};

static int scene_exit;

static int mode;	
static int a[2] = {0,0};
static int b[2] = {0,0};
static int kane[200];	
static int uracount = 0;

static int title_no = 0;
static char string[1024];

void title_main( void )
{
	int exit_code;
	
	exit_code = 0;
	
	title_init( );		
	
	while( scene_exit )
	{
		title_keys( );		
		title_drow( );
		
		RefreshScreen( g_screen );
		
		
		FPSWait( );	
		exit_code = system_keys( ); 
		if ( exit_code == 0 )
		{
			scene_exit = 0;
		}
	}
	
	title_relese( );
}

void title_init( void )
{
	scene_exit = 1;

	title_no = 0;
	mode = 0;		
	memset( kane, 0, sizeof( kane ) );
	uracount = 0;
	title_no = 0;

	LoadBitmap("image/title/title_natuki.bmp",1,true);

	if ( gameflag[100] == 1 )
	{
		LoadBitmap("image/title/title2_2.bmp",2,true);
	}
	else 
	{
		LoadBitmap("image/title/title2.bmp",2,true);
	}
	LoadBitmap("image/title/scl.bmp",3,true);
	LoadBitmap("image/bak/1.bmp",5, false );

	if ( gameflag[126] == 0 )
	{
		LoadBitmap("image/title/title5.bmp",7,true);
	}
	else 
	{
		LoadBitmap("image/title/title5_ura.bmp",7,true);
	}

	LoadBitmap("image/title/title3.bmp",10,true);
	LoadBitmap("image/sys/fonts2.bmp",18,true);
	/*LoadBitmap("image/sys/waku.bmp",109,true);*/

	a[0] = 100;
	a[1] = 200;
	title_kane_set( 0, 300 );
	
	if ( gameflag[123] != -1 )
	{
		gameflag[120] = gameflag[123];
	}
	
	if ( gameflag[120] > 50 )
	{
		gameflag[120] = 50;
	}
                                        
	soundPlayBgm( EN_BGM_GAME01 );

}

void title_relese( void )
{
	int i;
	
	for ( i = 0; i < BMPBUFF_MAX; i++ )
	{
		ReleaseBitmap( i );
	}
	soundStopBgm(EN_BGM_GAME01);

}


void title_keys( void )
{
	char path_item[512];
	char path_work[512];

#ifdef MINGW
	sprintf(path_work, "save/work.sav");
	sprintf(path_item, "save/item_wk.sav");
#elif defined(DREAMCAST)
	sprintf(path_work, "/ram/work.sav");
	sprintf(path_item, "/ram/item_wk.sav");
#elif defined(_TINSPIRE)
	sprintf(path_work, "./save/work.sav.tns");
	sprintf(path_item, "./save/item_wk.sav.tns");
#else
	sprintf(path_work, "%s/.ganbare/work.sav", getenv("HOME"));
	sprintf(path_item, "%s/.ganbare/item_wk.sav", getenv("HOME"));
#endif

	
	if ( IsPushKey( gameflag[0] ) )
	{
		soundPlaySe( EN_SE_SELECT );
		mode--;
		if ( gameflag[100] == 1 )
		{
			if ( mode == -2 )
			{
				mode = 4;
			}
		}
		else 
		{
			if ( mode == -1 )
			{
				mode = 4;
			}
		}
	}
	else if ( IsPushKey( gameflag[1] ) )
	{
		soundPlaySe( EN_SE_SELECT );
		mode++;
		if ( gameflag[100] == 1 )
		{
			if ( mode == 5 )
			{
				{
					mode = -1;
				}
			}
		}
		else 
		{
			if ( mode == 5 )
			{
				{
					mode = 0;
				}
			}
		}
	}
	if ( IsPushKey( gameflag[2] ) )
	{
		if ( mode == 0 )
		{
			soundPlaySe( EN_SE_SELECT );
			gameflag[120]--;
			if ( gameflag[120] < 1 )
			{
				gameflag[120] = gameflag[121];
				if ( gameflag[120] > 50 )
				{
					gameflag[120] = 50;
				}
			}
		}
		if ( mode == 1 )
		{
			 
			soundPlaySe( EN_SE_SELECT );
			gameflag[124]--;
			if ( gameflag[124] < 0 )
			{
				gameflag[124] = 1;
			}
		}
	}
	else if ( IsPushKey( gameflag[3] ) )
	{
		if ( mode == 0 )
		{
			soundPlaySe( EN_SE_SELECT );
			gameflag[120]++;
			if ( ( gameflag[120] > gameflag[121] ) || ( gameflag[120] > 50 ) )
			{
				gameflag[120] = 1;
			}
		}
		if ( mode == 1 )
		{
			 
			soundPlaySe( EN_SE_SELECT );
			gameflag[124]++;
			if ( gameflag[124] > 1 )
			{
				gameflag[124] = 0;
			}
		}
	}
	
	if ( IsPressKey( gameflag[2] ) )
	{
		b[0]++;
		if ( mode == 0 )
		{
			if ( b[0] >= 12 )
			{
				b[0] = 0;
				soundPlaySe( EN_SE_SELECT );
				gameflag[120]--;
				if ( gameflag[120] < 1 )
				{
					gameflag[120] = gameflag[121];
					if ( gameflag[120] > 50 )
					{
						gameflag[120] = 50;
					}
				}
			}
		}
	}
	else if ( IsPressKey( gameflag[3] ) )
	{
		b[0]++;
		if ( mode == 0 )
		{
			if ( b[0] >= 12 )
			{
				b[0] = 0;
				soundPlaySe( EN_SE_SELECT );
				gameflag[120]++;
				if ( ( gameflag[120] > gameflag[121] ) || ( gameflag[120] > 50 ) )
				{
					gameflag[120] = 1;
				}
			}
		}
	}
	else 
	{
		b[0] = 0;
	}

	if ( IsPushOKKey( ) )
	{
		if ( mode == 4 )	/* Exit */
		{
			gameflag[123] = -1;
			gameflag[40] = 10;
			g_scene = EN_SN_EXIT;
			scene_exit=0;
		}
		else if ( mode == 3 )	/* option */
		{
			gameflag[123] = -1;
			gameflag[40] = 3;
			g_scene = EN_SN_OPTION;
			scene_exit=0;
		}
		else if ( mode == 2 )	/* demo */
		{
			ResetGameFlag2( );
			
			title_init_save_data( );
			
			gameflag[125] = 0;	/* replay nomal */
			gameflag[123] = gameflag[120];
			gameflag[132] =  1;		
			gameflag2[3] = 1;
			gameflag2[2] = 0;
			gameflag[70] = 1;
			SaveGameFlag2(path_work);

			ResetGameFlag2( );
			SaveGameFlag2(path_item);

			gameflag[40] = 4;
			g_scene = EN_SN_ACT;
			scene_exit=0;
		}
		else if ( mode == 1 )	/* replay */
		{
			if ( replay_file_find( ) == 1 )
			{
				ResetGameFlag2( );
				
				title_init_save_data( );
				
				gameflag[127] = 0;
				gameflag[125] = 0;
				gameflag[123] = gameflag[120];
				gameflag[132] =  1;	
				gameflag[70] = 1;
				SaveGameFlag2(path_work);

				ResetGameFlag2( );
				SaveGameFlag2(path_item);

				gameflag[40] = 4;
				g_scene = EN_SN_ACT;
				scene_exit=0;
				return;
			}
			else 
			{
				soundPlaySe( EN_SE_MSG );
			}
		}
		else if ( mode == 0 )	/* new */
		{
			ResetGameFlag2( );
			
			title_init_save_data( );
			
			gameflag[127] = 0;
			gameflag[123] = -1;	
			gameflag[132] =  0;
			gameflag[70] = 1;
			SaveGameFlag2(path_work);

			ResetGameFlag2( );
			SaveGameFlag2(path_item);

			gameflag[40] = 4;
			g_scene = EN_SN_ACT;
			scene_exit=0;
			return;
		}
		else if ( mode == -1 )
		{
			ResetGameFlag2( );
			
			title_init_save_data( );
			
			gameflag[135] = 1000000;
			gameflag[136] = 0;			
			gameflag2[2] = 1;	
			gameflag2[3] = 1;	
			gameflag[123] = gameflag[120];	
			gameflag[127] = 1;	
			gameflag[123] = -1;	
			gameflag[132] =  0;
			gameflag[70] = 1;
			SaveGameFlag2(path_work);

			ResetGameFlag2( );
			SaveGameFlag2(path_item);

			gameflag[40] = 4;
			g_scene = EN_SN_ACT;
			scene_exit=0;
		}
	}

	if ( IsPushCancelKey( ) )
	{
		if ( mode == 0 )
		{
			if ( gameflag[100] == 1 )
			{
				uracount++;
				if ( uracount >= 16 )
				{
					if ( gameflag[126] == 0 )
					{
						gameflag[126] = 1;
						LoadBitmap("image/title/title5_ura.bmp",7,true);
					}
					else 
					{
						gameflag[126] = 0;
						LoadBitmap("image/title/title5.bmp",7,true);				
					}
					uracount = 0;
				}
			}
		}
		if ( mode == 1 )	/* replay jamp */
		{
			if ( replay_file_find2( ) == 1 )
			{
				ResetGameFlag2( );
				
				title_init_save_data( );
				
				gameflag[127] = 0;	
				gameflag[125] = 1;	
				gameflag[123] = gameflag[120];	
				gameflag[132] =  1;	
				gameflag[70] = 1;
				SaveGameFlag2(path_work);

				ResetGameFlag2( );
				SaveGameFlag2(path_item);

				gameflag[40] = 4;
				g_scene = EN_SN_ACT;
				scene_exit=0;
			}
			else 
			{
				soundPlaySe( EN_SE_MSG );
			}
		}
	}
	if ( IsPushKey( gameflag[6] ) )
	{
		gameflag[122]++;
		if ( gameflag[100] == 1 )
		{
			if ( gameflag[122] > 4 )
			{
				gameflag[122] = 0;
			}
		}
		else 
		{
			if ( gameflag[122] > 2 )
			{
				gameflag[122] = 0;
			}
		}
	}

}

void title_drow( void )
{
	int stage_hosei;
	int wk;
	
	stage_hosei = 0;
	if ( gameflag[126] == 1 )
	{
		stage_hosei = 50;
	}

	ClearSecondary();
	
	if ( title_no == 0 )
	{
		Blt( 5, 0, 0 );
		
		Blt( 1, 100 - a[0], 0 + a[1] );
		
		a[0] = a[0] - 10;
		if ( a[0] < 0 )
		{
			a[0] = 0;
		}
		a[1] = a[1] - 20;
		if ( a[1] < 0 )
		{
			a[1] = 0;
		}
		
		Blt( 2, 0, 0 );
		title_kane_disp(  );
		
		BltRect( 3, 96, 128 + ( mode * 16 ), 0, gameflag[122] * 32 , 32 , 32 );
		BltNumericImage2( gameflag[120], 2, 262, 148, 18, 0, 0, 10, 8 );
		BltRect( 18, 262, 164, 0, 56 + ( gameflag[124] * 8 ), 100, 8 );
		
		BltRect( 18, 5, 230 , 0, 24, 100, 8 );	/*  */
		BltRect( 18, 50, 230 , 0, 8, 100, 8 );	/*  */

		wk = get2keta( gameflag[200 + gameflag[120] + stage_hosei], 1 );
		BltNumericImage( wk, 2, 110, 230, 18, 0, 0, 10, 8 );	/*  */
		wk = get2keta( gameflag[200 + gameflag[120] + stage_hosei], 100 );
		BltNumericImage( wk, 2, 80, 230, 18, 0, 0, 10, 8 );	/*  */
		wk = get2keta( gameflag[200 + gameflag[120] + stage_hosei], 10000 );
		BltNumericImage( wk, 2, 50, 230, 18, 0, 0, 10, 8 );	/*  */
			
		BltRect( 18, 150, 230, 0, 80, 100, 8 );	/*  */
		BltNumericImage2( gameflag[300 + gameflag[120] + stage_hosei], 3, 185, 230, 18, 0, 0, 10, 8 );
	}

	/*if ( gameflag[61] == 0 )
	{
		Blt( 109 , -160, -120 );
	}*/
	
	KeyInput();				

}


void title_init_save_data( void )
{
	gameflag2[0]	= 8 * 32;	
	gameflag2[1]	= ( 2 * 32 ) - 16;	
	if ( gameflag[126] == 1 )
	{
		gameflag2[2]	= 2;
	}
	else 
	{
		gameflag2[2]	= 1;
	}
	gameflag2[3]	= gameflag[120];
	gameflag2[4]	= 0;	/* 向き */
	gameflag2[5]	= 3;	/* 現在ＨＰ */
	gameflag2[6]	= 3;	/* 最大ＨＰ */
	gameflag2[7]	= 0;	/* ハートのかけら所持個数 */
	gameflag2[8]	= 0;	/* 現在設定スキル */
	gameflag2[9]	= 0;	/* テレポータ使用不可、スクロール不可フラグ */
	gameflag2[10]	= 0;	/* テレポータ使用不可フラグ */

	gameflag2[20]	= 0; 	/* 時 */
	gameflag2[21]	= 0; 	/* 分 */
	gameflag2[22]	= 0; 	/* 秒 */
	
	gameflag2[30]	= 0;	/* テレポーター使用 */
	gameflag2[31]	= 0;	/* テレポーターステージ */
	gameflag2[32]	= 0;	/* テレポーター画面Ｎｏ */
	gameflag2[33]	= 0;	/* テレポーターＸ */
	gameflag2[34]	= 0;	/* テレポーターＹ */

	gameflag2[40]	= 0;	/* 取得スキル１ */
	gameflag2[41]	= 0;	/* 取得スキル２ */
	gameflag2[42]	= 0;	/* 取得スキル３ */
	gameflag2[43]	= 0;	/* 取得スキル４ */
}

/***************************************************************************/
// NAME      = kane_set
// FUNCTION  = タイトル文字の生成
// NOTES     = 
// DATE      = 
// AUTHER    = koizumi
// HISTORY   =
// PARAMETER = x：初期位置
//             y：初期位置
// RETURN    = なし
/***************************************************************************/
void title_kane_set( int x, int y )
{
	int i;
	
	for ( i = 0; i < 20; i++ )
	{
		if ( kane[0 + ( i * 10 )] == 0 )
		{
			kane[0 + ( i * 10 )] = 1;
			kane[1 + ( i * 10 )] = x;
			kane[2 + ( i * 10 )] = y;
			kane[4 + ( i * 10 )] = 0;
			kane[3 + ( i * 10 )] = 0;
			kane[5 + ( i * 10 )] = 0;
			kane[6 + ( i * 10 )] = 0;
			kane[7 + ( i * 10 )] = 0;
			kane[8 + ( i * 10 )] = 0;
			kane[9 + ( i * 10 )] = 6;
			break;
		}
	}
}


/***************************************************************************/
// NAME      = kane_disp
// FUNCTION  = タイトル文字の表示
// NOTES     = 
// DATE      = 
// AUTHER    = koizumi
// HISTORY   =
// PARAMETER = なし
// RETURN    = なし
/***************************************************************************/
void title_kane_disp( void )
{
	int i;
	
	for ( i = 0; i < 20; i++ )
	{
		if ( kane[0 + ( i * 10 )] == 1 )
		{
			BltRect( 7, kane[1 + ( i * 10 )], ( 0 - kane[2 + ( i * 10 )] ) + 240 - 96, 0, 64, 192, 96 );

			title_k_jmp( i );
		}
	}
}


/***************************************************************************/
// NAME      = k_jmp
// FUNCTION  = タイトル文字のY表示位置計算
// NOTES     = 
// DATE      = 
// AUTHER    = koizumi
// HISTORY   =
// PARAMETER = i：バッファ番号
// RETURN    = なし
/***************************************************************************/
void title_k_jmp( int i )
{
	int y1;



	if ( kane[6 + ( i * 10 )] == 10 ) 
	{
		if ( kane[9 + ( i * 10 )] > 0 )
		{ 
			kane[6 + ( i * 10 )] = kane[9 + ( i * 10 )];
			kane[9 + ( i * 10 )] = kane[9 + ( i * 10 )] - 1;
		}
		return;
	}

	kane[5 + ( i * 10 )]++;
	if ( kane[5 + ( i * 10 )] >= 10 )
	{
		kane[5 + ( i * 10 )] = 0;
	} 
	
	if ( kane[5 + ( i * 10 )] == 0 )
	{
		kane[6 + ( i * 10 )] = kane[6 + ( i * 10 )] - 3;
		if ( ( kane[6 + ( i * 10 )] <= 2 ) && ( kane[6 + ( i * 10 )] > -2 ) )
		{
			kane[6 + ( i * 10 )] = -3;
		}
		if ( kane[6 + ( i * 10 )] < -8 )
		{
			kane[6 + ( i * 10 )] = -8;
		}
		/* 地面判定 */
	}
	
	/* 今回の位置 */
	y1 = ( ( 0 - kane[6 + ( i * 10 )] ) * ( 0 - kane[6 + ( i * 10 )] ) * ( 0 - kane[6 + ( i * 10 )] ) );
	kane[2 + ( i * 10 )] = kane[2 + ( i * 10 )] - ( y1 / 25 );

	if ( kane[2 + ( i * 10 )] < 140 ) 
	{
		kane[6 + ( i * 10 )] = 10;
	}
}



/***************************************************************************/
// NAME      = replay_file_find
// FUNCTION  = リプレイファイルの検索
// NOTES     = 
// DATE      = 
// AUTHER    = koizumi
// HISTORY   =
// PARAMETER = なし
// RETURN    = ファイルの有無
/***************************************************************************/
int replay_file_find( void )
{
	long i;
	int file_j;
	int stage;
	
	stage = 1;
	if ( gameflag[126] == 1 )
	{
		stage = 2;	/* 裏ステージ */
	}
	else 
	{
	}
	
	file_j = 1;
	
	sprintf( string, "replay/%d/replay_data_%d.dat", ( int )stage, ( int )gameflag[120] );
	if ( LoadFile( string, &i, 1 ) )
	{
		file_j = 0;
	}
	
	return( file_j );
} 


/***************************************************************************/
// NAME      = replay_file_find2
// FUNCTION  = 最短ジャンプリプレイファイルの検索
// NOTES     = 
// DATE      = 
// AUTHER    = koizumi
// HISTORY   =
// PARAMETER = なし
// RETURN    = ファイルの有無
/***************************************************************************/
int replay_file_find2( void )
{
	long i;
	int file_j;
	int stage;
	
	stage = 1;
	if ( gameflag[126] == 1 )
	{
		stage = 2;	/* 裏ステージ */
	}
	else 
	{
	}
	
	file_j = 1;
	sprintf( string, "replay/%d/replay_data_j%d.dat", ( int )stage, ( int )gameflag[120] );
	if ( LoadFile( string, &i, 1 ) )
	{
		file_j = 0;
	}
	
	return( file_j );
} 




