#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#include <SDL/SDL.h>
#include "define.h"
#include "function.h"
#include "util_snd.h"
#include "extern.h"
#include "act.h" 
#include "include/dconv.h" 
#include "include/general.h" 

#include "refresh.h"

void act_main( void );
void act_init( void );
void act_relese( void );
void act_keys( void );
void fram_set( );
void jmp_st( );
void jmp( );
void idou_l( long ankr_flag );
void idou_r( long ankr_flag );
void cleaer_ankr( long ankr_flag );
void act_drow( void );
void save_file_w( );
void save_file( );
void clear_save_file( );
void playerdisp( );
void bakdisp1();
void bakdisp2( );
void bak_make( );
void wait_scl( long type, long d_num );
void wait_scl2( long d_num ,long init_x, long init_y );
void wait_scl3( );
void snack_counter_disp();
void minimap_disp( void );
void mapdisp();
void bgm_midi( long BgmWavNo );
void stage_start( );
void stage_endj( );
void stage_claer( );
void stage_end( );
void p_shot_set( long type , long muki, long houkou );
void p_shot_disp( );
void p_shot_f_set( long x, long y , long type );
void p_shot_f_disp( );
void p_shot_f2_set( long x, long y , long muki );
void p_shot_f2_disp( );
void item_set( long type, long x, long y, long iti, long g_num );
long item_sonzai( long map1, long map2 );
void item_disp( );
long item_up( );
long item_up2( );
long item_event( );
void init_event_set_enm( long event_no , long b_num );
void init_event_set( long event_no , long b_num );
void event_set( long event_no , long b_num );
void event_set_enm( long event_no , long b_num );
void event_disp( );
void event_save( );
void load_save_file( );
void event_kaihuku( );
void event_return( );
void event_warp( );
void enm_set( long type, long x, long y );
void enm_set2( long type, long x, long y ,long muki );
long e_hp( long type );
void enm_disp();
void e_move( long i );
void boss_dead( long x, long y );
void e_shot_set( long type , long i , long x, long y );
void e_shot_disp( );
void enmjmp( long i );
long map1_item_j( long x, long y );
void kane_set( long x, long y );
void kane_disp(  );
void k_jmp( long i );
void p_jump_f_set( long x, long y );
void p_jump_f_disp( );
void screen_BackupPlane( );
void window_disp( );
void window_keys( );
long labs( long para );
void p_mgc_set( long muki , long p3 );
void p_mgc_disp( );
void BlendExBltRectW( long plane_no, long x, long y, long sx, long sy, long hx, long hy, long func, long blend );
void BlendExBltW( long plane_no, long x, long y, long func, long blend );
void BlendExBltFastW( long plane_no, long x, long y, long func, long blend );
void replay_input_save( void );
void replay_input_load( void );
void replay_file_save_w( void );
void replay_file_save( long type );
long replay_file_load( void );
long GetKyori( long i1, long i2 );
void debugdisp( void );

char string[1024];

static long scene_exit;
static long sn = 0;									/* 場面用変数これを変えることによって描画関数内の処理を変える		*/

static long enm1_parameter[10 * 20] = 
{
/*   HP */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 0未使用 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 1 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 2 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 3 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 4 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 5 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 6 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 7 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 8 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 9 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 10 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 11 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 12 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0 	/* 20未使用 */
};

static long enm2_parameter[10 * 20] = 
{
/*   HP */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 0未使用 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 1 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 2 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 3 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 4 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 5 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 6 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 7 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 8 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 9 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0 	/* 20未使用 */
};

static long enm3_parameter[10 * 20] = 
{
/*   HP   金 攻撃 属性 相性 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 0未使用 */
	  1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 1 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 2 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 3 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 4 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 5 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 6 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 7 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 8 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 9 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0 	/* 20未使用 */
};

static long enm4_parameter[10 * 20] = 
{
/*   HP */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 0未使用 */
	  7,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 1ボス */
	  7,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 2 */
	 30,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 3 */
	 27,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 4 */
     42,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 5 */
     30,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 6 */
      1,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 7 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 8 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 	/* 9 */
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0 	/* 20未使用 */
};

static long item_data[3] = { 32, 500, 10 };			/* アイテムの情報　サイズ、最大配置個数、1データバッファ数 */
static long enm_data[3] = { 0, 500, 20 };			/* 敵の情報　予備、最大配置個数、1データバッファ数 */
static long MapInfo[10] = 
{
	20 * 1									/* 0:マップのxサイズ */
,	15 * 1									/* 1:マップのyサイズ */
,	( 20 * 1 ) * 32							/* 2:マップのx表示サイズ */
,	( 15 * 1 ) * 32							/* 3:マップのy表示サイズ */
,   20										/* 4:ヘッダデータ部のデータ数 */
};

static long player[50];									/* プレイヤー用バッファ */
											/* player[0] X */
											/* player[1] Y */
											/* player[2] 状態 */
											/* player[3] 向き */
											/* player[4] BMP内の表示位置X */
											/* player[5] BMP内の表示位置Y */
											/* player[6] 掴み中の移動補正X */
											/* player[7] 掴み中の移動補正Y */
											/* player[8] 現在HP */
											/* player[9] 敵を掴んでいる */
											/* player[10] アニメーションフレームカウンタ */
											/* player[11] アニメーションカウンタ */
											/* player[12] アニメーションフレーム指定 */
											/* player[13] ジャンプフレームカウンタ */
											/* player[14] ジャンプカウンタ */
											/* player[15] ミスの場合のX */
											/* player[16] ミスの場合のY */
											/* player[17] 無敵時間 */
											/* player[18] 着地モーション用カウンタ */
											/* player[19] 掴んだ時点での距離 */
											/* player[20] 腕の方向 */
											/* player[21] 腕の距離 */
											/* player[22] 回転方向 */
											/* player[23] 掴むフラグ */
											/* player[24] アンカー状態からジャンプした */
											/* player[25] ボタンをおしてジャンプしたフラグ */
											/* player[26] 長時間落下したフラグ（着地フェクト用） */
											/* player[27] 掴んだときに入力されていたキー */
											/* player[28] 移動床モーション */
											/* player[29] 横ばね吹き飛び中 */
											/* player[30] アンカーX */
											/* player[31] アンカーY */
											/* player[32] アンカー掴み位置X */
											/* player[33] アンカー掴み位置Y */
											/* player[34] アンカー吹っ飛びX */
											/* player[35] アンカー吹っ飛びY */
											/* player[36] アンカー吹っ飛び方向 */
											/* player[37] アイテムを掴んでいる */
											/* player[38] アンカーを伸ばした(効果音用) */
static long dp_x;									/* プレイヤーの表示位置（画面内の相対座標） */
static long dp_y;									/* プレイヤーの表示位置（画面内の相対座標） */
static long demo[10];
static long map1[320];								/* 背景用バッファ レイヤー1 */
static long map2[320];								/* 背景用バッファ レイヤー2 */
static long map_data[20];
static long bak_cnt = 0;							/* 背景アニメーション用カウンタ */
static long d_x = 0;								/* 背景表示位置 */
static long d_y = 0;								/* 背景表示位置 */
//static long disp_data[60];							/* セーブ一覧 */
static long next_px = 0;							/* 画面切り替え時のキャラ移動対策 */
static long next_py = 0;

static long atk_rr1 =22;						/* 腕の回転速度 */
static long atk_rr2 = 1;						/* 腕の回転速度 */
//long atk_rr = 360 / 32;						/* 腕の回転速度 */

static long stage = 0;								/* ステージ番号 */

static long pshot[10 * 10];							/* プレイヤー攻撃用情報 */
static long pshotf[10 * 40];						/* プレイヤー攻撃用情報 */
static long pshotf2[10 * 80];						/* プレイヤー攻撃用情報 */
static long pmshot[20 * 40];						/* プレイヤー攻撃用情報 */
static long pjumpf[100];							/* プレイヤー攻撃用情報 */
static long item[10 * 50];							/* アイテム用バッファ */
static long item_wk[5*100];							/* 地面 その他のマップ情報 */
static long enemy[40 * 20];								/* 敵情報															*/
static long mgc_e[10 * 20] ;						/* MGC表示情報														*/
static long p_ef[1000] ;							/* 汎用バッファ（足跡等）														*/
//static long life_a[5];								/* HPアニメーション用 */
static long kane[200];								/* アイテム用バッファ */

static long Jump_counts = 0;
static long save_data[512];							/* 保存情報 */
static long test[5];
static long rayer[5];								/* 背景スクロール量 */
static long play_time[5];
/* リプレイ２０分 */
static long replay[60 * 60 * 10];	/* フレーム＊秒＊分 */
static long replay_time = 1;
static long replay_time_MAX = 60 * 60 * 10;	/* 最大時間 */
static long replay_load_key[10];
static long replay_file_name = 0;
static long replay_save_flag = 0;				/* 保存を行うか 1:しない */
static long rrkey[5];								/* 入力キー方向 */

//static long event_buff[100];
/* event_buff[0]	文字用ｳｨﾝﾄﾞｳ表示 */
/* event_buff[1]	顔表示 */
/* event_buff[2]	ｷｰ入力待ち */
/* event_buff[3]	wait */
/* event_buff[4]	実行イベントバッファポインタ */
/* event_buff[5]	顔表示 */
/* event_buff[7]	i */
/* event_buff[8]	player[2] */
/* event_buff[9]	player[3] */
/* event_buff[10]	画面更新 */
/* event_buff[11]	画面演出 */

/* TextLayer( 10 )はｲﾍﾞﾝﾄで使用（固定） */
/* プレーン80～90はｲﾍﾞﾝﾄで使用 */
/* wav30はｲﾍﾞﾝﾄで使用 */
//static long file_handle = 0	;
static long one_event = 0;							/* 重なって発生するイベントの前回重なっていたかのフラグ */
//static long start_plyer_disp = 1;					/* 開始時に表示するかフラグ */
//static long start_effct_disp = 0;					/* 開始時に効果を表示するかフラグ */

#define DEBUG_DISP (0)

//----------------------------------------------------------------------
// メイン関数
void act_main( void )
{
	long exit_code;
	
	exit_code = 0;

	act_init( );		// 初期化
	
	while( scene_exit )
	{
//		KeyInput( );	// キー入力
		
//		act_keys( );		// キー処理
		act_drow( );		// 描画
		
		FPSWait( );		// 待ち

		exit_code = system_keys( );  //
		if ( exit_code == 0 )
		{
			scene_exit = 0;
		}
	}
	
	act_relese( );		// 終了
}

void act_init( void )
{
	long i;
	long x;
	long y;
	long wk;
	long d_num;
	
	char path_item[512];
	char path_work[512];

#ifdef MINGW
	sprintf(path_work, "save/work.sav");
	sprintf(path_item, "save/item_wk.sav");
#elif defined(DREAMCAST)
	sprintf(path_work, "/ram/work.sav");
	sprintf(path_item, "/ram/item_wk.sav");
#else	
	sprintf(path_work, "%s/.ganbare/work.sav", getenv("HOME"));
	sprintf(path_item, "%s/.ganbare/item_wk.sav", getenv("HOME"));
#endif

	scene_exit = 1;
	sn = 0;
	bak_cnt = 0;							/* 背景アニメーション用カウンタ */
	d_x = 0;								/* 背景表示位置 */
	d_y = 0;								/* 背景表示位置 */
	next_px = 0;							/* 画面切り替え時のキャラ移動対策 */
	next_py = 0;
	atk_rr1 =22;						/* 腕の回転速度 */
	atk_rr2 = 1;						/* 腕の回転速度 */
	stage = 0;								/* ステージ番号 */
	Jump_counts = 0;
	replay_time = 1;
	replay_time_MAX = 60 * 60 * 20;	/* 最大時間 */
	replay_save_flag = 0;				/* 保存を行うか 1:しない */
	one_event = 0;							/* 重なって発生するイベントの前回重なっていたかのフラグ */

//#if false
	memset( player, 0, sizeof( player ) );									/* プレイヤー用バッファ */
	memset( demo, 0, sizeof( demo ) );
	memset( map1, 0, sizeof( map1 ) );								/* 背景用バッファ レイヤー1 */
	memset( map2, 0, sizeof( map2 ) );								/* 背景用バッファ レイヤー2 */
	memset( map_data, 0, sizeof( map_data ) );
	memset( pshot, 0, sizeof( pshot ) );							/* プレイヤー攻撃用情報 */
	memset( pshotf, 0, sizeof( pshotf ) );						/* プレイヤー攻撃用情報 */
	memset( pshotf2, 0, sizeof( pshotf2 ) );						/* プレイヤー攻撃用情報 */
	memset( pmshot, 0, sizeof( pmshot ) );						/* プレイヤー攻撃用情報 */
	memset( pjumpf, 0, sizeof( pjumpf ) );							/* プレイヤー攻撃用情報 */
	memset( item, 0, sizeof( item ) );							/* アイテム用バッファ */
	memset( item_wk, 0, sizeof( item_wk ) );							/* 地面 その他のマップ情報 */
	memset( enemy, 0, sizeof( enemy ) );								/* 敵情報															*/
	memset( mgc_e, 0, sizeof( mgc_e ) ) ;						/* MGC表示情報														*/
	memset( p_ef, 0, sizeof( p_ef ) ) ;							/* 汎用バッファ（足跡等）														*/
	memset( kane, 0, sizeof( kane ) );								/* アイテム用バッファ */
	memset( save_data, 0, sizeof( save_data ) );							/* 保存情報 */
	memset( test, 0, sizeof( test ) );
	memset( rayer, 0, sizeof( rayer ) );								/* 背景スクロール量 */
	memset( play_time, 0, sizeof( play_time ) );
	memset( replay, 0, sizeof( replay ) );	/* フレーム＊秒＊分 */
	memset( replay_load_key, 0, sizeof( replay_load_key ) );
	memset( rrkey, 0, sizeof( rrkey ) );								/* 入力キー方向 */
	memset( string, 0, sizeof( string ) );								/* 入力キー方向 */

	ResetGameFlag2();				/* ファイルなし */
	for ( i = 0; i <= 500; i++ )
	{
		item_wk[i] = gameflag2[i];
	}
	
	ResetGameFlag2( );
	LoadGameFlag2(path_work);
	for ( i = 0; i < 512; i++ )
	{
		save_data[i] = gameflag2[i];
	}
	player[8] = 1;	/* HP */
	stage = save_data[2];
	d_num = save_data[3]; 	/* 画面 */
	gameflag[120] = d_num;


	player[20] = 0;
	player[21] = 32;
	player[22] = 1;



	ResetGameFlag2( );
	sprintf(string,"data/%d/%d_%d.map", ( int )stage , ( int )1, ( int )d_num );
	if ( LoadGameFlag2(string) == 0 )
	{
		for ( i = 0; i <= 319; i++ )
		{
			map1[i] = gameflag2[i];
		}
	}
	else 
	{
		if ( gameflag[132] == 1 )
		{
			g_scene = EN_SN_TITLE;
		}
		else 
		{
			g_scene = EN_SN_ENDING;	
		}
		sn = 10;
	}
	for ( i = 0 ; i <= 20 ; i++ )
	{
		map_data[i] = gameflag2[i + 320];
	}
	
	for ( i = 0 ; i < 5 ; i++ )
	{
		if ( map_data[11 + i] != 0 )
		{
			sprintf(string,"image/bak/%d.bmp", ( int )map_data[11 + i] );
			if ( LoadBitmap(string,110 + i , 0 ) != 0 )	
			{
				ReleaseBitmap(110 + i);
			}
		}
	}

	player[0] = map_data[9] * 32;
	player[1] = ( map_data[10] * 32 ) - 16;
	
	ResetGameFlag2( );
	sprintf(string,"data/%d/%d_%d.map", ( int )stage , ( int )2, ( int )d_num );
	if ( LoadGameFlag2(string) == 0 )
	{
		for ( i = 0; i <= 319; i++ )
		{
			map2[i] = gameflag2[i];
		}
	}

	ResetGameFlag2( );
	sprintf(string,"data/%d/i%d_%d.map", ( int )stage , ( int )1, ( int )d_num );
	if ( LoadGameFlag2(string) == 0 )
	{
		for ( x = 0; x <= 19 ; x++ )
		{
			for ( y = 0; y <= 16 ; y++ )
			{
				if ( gameflag2[x + ( y * 20 )] != 0 )
				{
					item_set( gameflag2[x + ( y * 20 )], x * 32, ( ( 14 - y ) * 32 ) + 16, x + ( y * 20 ), d_num );
				}
			}
		}
	}


	
	ResetGameFlag2();				/* ファイルなし */
	sprintf( string,"data/%d/e%d_%d.map", ( int )stage , ( int )1, ( int )d_num );
	if ( LoadGameFlag2( string ) == 0 )	/* 敵配置 */
	{
		/* 敵配置 */
		for ( x = 0; x <= 19 ; x++ )
		{
			for ( y = 0; y <= 16 ; y++ )
			{
				if ( gameflag2[x + ( y * 20 )] != 0 )
				{
					enm_set( gameflag2[x + ( y * 20 )], x * 32, ( ( 14 - y ) * 32 ) + 16 );
				}
			}
		}
	}
	
	player[14] = 10;
	player[15] = player[0];
	player[16] = player[1];
	player[2] = 0;

	if ( gameflag[132] == 1 )
	{
		if ( 1 != replay_file_load( ) )
		{
			g_scene = EN_SN_TITLE;	/* タイトルへ */
			sn = 10;
			return;
		}
	}
	else 
	{
	}


	if ( gameflag[70] == 1 )
	{
		if (
		     ( ( save_data[3] >  0 ) && ( save_data[3] <=  5 ) ) 
		  || ( ( save_data[3] > 20 ) && ( save_data[3] <= 25 ) )
		  || ( ( save_data[3] > 40 ) && ( save_data[3] <= 45 ) )
		   )
		{
			soundPlayBgm( EN_BGM_GAME02 );
		}
		if (
		     ( ( save_data[3] > 45 ) && ( save_data[3] <= 50 ) )
		   )
		{
			if ( stage == 2 )
			{
				soundPlayBgm( EN_BGM_GAME07 );
			}
			else 
			{
				soundPlayBgm( EN_BGM_GAME06 );
			}
		}
		if (
		     ( ( save_data[3] >  5 ) && ( save_data[3] <= 10 ) ) 
		  || ( ( save_data[3] > 25 ) && ( save_data[3] <= 30 ) )
		  || ( ( save_data[3] > 35 ) && ( save_data[3] <= 40 ) )
		   )
		{
			soundPlayBgm( EN_BGM_GAME03 );
		}
		if (
		     ( ( save_data[3] > 10 ) && ( save_data[3] <= 15 ) ) 
		  || ( ( save_data[3] > 30 ) && ( save_data[3] <= 35 ) )
		   )
		{
			soundPlayBgm( EN_BGM_GAME04 );
		}
		if (
		     ( ( save_data[3] > 15 ) && ( save_data[3] <= 20 ) ) 
		  || ( ( save_data[3] > 35 ) && ( save_data[3] <= 40 ) )
		   )
		{
			soundPlayBgm( EN_BGM_GAME05 );
		}
	}
	else 
	{
		if ( gameflag[71] == 0 )
		{
			if ( (save_data[3] ==  1) || (save_data[3] == 21) || (save_data[3] == 41 ))
			{
				soundPlayBgm( EN_BGM_GAME02 );
			}
			if (save_data[3] == 46)
			{
				if ( stage == 2 )
				{
					soundPlayBgm( EN_BGM_GAME07 );
				}
				else 
				{
					soundPlayBgm( EN_BGM_GAME06 );
				}
			}
			if (
			     ( ( save_data[3] ==  6 ) ) 
			  || ( ( save_data[3] == 26 ) )
			   )
			{
				soundPlayBgm( EN_BGM_GAME03 );
			}
			if (
			     ( ( save_data[3] == 11 ) ) 
			  || ( ( save_data[3] == 31 ) )
			   )
			{
				soundPlayBgm( EN_BGM_GAME04 );
			}
			if (
			     ( ( save_data[3] == 16 ) ) 
			  || ( ( save_data[3] == 36 ) )
			   )
			{
				soundPlayBgm( EN_BGM_GAME05 );
			}
		}
	}
	
	
	gameflag[71] = 0;
	gameflag[70] = 0;

	if ( gameflag[127] == 1 )	//トータルアタック
	{
		wk = get2keta( gameflag[135] , 1 );
		play_time[0] = play_time[0] + wk;
		if ( play_time[0] >= 60 )
		{
			play_time[0] = play_time[0] - 60;
			play_time[1]++; 
		}
		wk = get2keta( gameflag[135] , 100 );
		play_time[1] = play_time[1] + wk;
		if ( play_time[1] >= 60 )
		{
			play_time[1] = play_time[1] - 60;
			play_time[2]++; 
		}
		
		wk = get2keta( gameflag[135] , 10000 );
		play_time[2] = play_time[2] + wk;
		if ( play_time[2] >= 999 )
		{
			play_time[2] = 999;
			play_time[1] = 59;
			play_time[0] = 59;
		}
		
		if ( gameflag[40] == 5 )
		{
			if ( gameflag[137] > gameflag[135] )
			{
				gameflag[137] = gameflag[135];
			}
			if ( gameflag[138] > gameflag[136] )
			{
				gameflag[138] = gameflag[136];
			}
		}
	}

	sprintf(string,"image/player/ri%d.bmp", ( int )gameflag[122] );
	LoadBitmap(string,1,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/player/mgc1.bmp",2,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/player/atk1.bmp",3,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/player/atk2.bmp",4,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む

	LoadBitmap("image/player/atkf.bmp",5,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む

	sprintf(string,"image/map/map%d_1.bmp", ( int )stage );
	
	printf("Map = %s \n", string);
	
	LoadBitmap(string,10,true);				
	sprintf(string,"image/map/map%d_2.bmp", ( int )stage );
	LoadBitmap(string,11,true);
	sprintf(string,"image/map/item%d.bmp", ( int )stage );
	LoadBitmap(string,12,true);


	LoadBitmap("image/sys/map_f0.bmp",15,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/map_f1.bmp",16,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/map_f2.bmp",17,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/fonts2.bmp",18,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/sc.bmp",19,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む

	LoadBitmap("image/sys/p_data.bmp",20,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/skill.bmp",21,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/hp.bmp",22,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/hp2.bmp",23,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	
	LoadBitmap("image/sys/pause.bmp",24,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	
	LoadBitmap("image/enm/e32.bmp",40,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/enm/e48.bmp",41,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/enm/e64.bmp",42,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	
	LoadBitmap("image/enm/e_shot.bmp",50,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む

	LoadBitmap("image/player/font_d.bmp",51,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/player/hp.bmp",52,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	

	LoadBitmap("image/sys/menu.bmp",60,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/title/scl.bmp",61,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/window_item.bmp",62,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む

	LoadBitmap("image/player/arm.bmp",65,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/player/arm2.bmp",66,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/input.bmp",67,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/new.bmp",68,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/new2.bmp",69,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/sys/mini_map.bmp",70,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む
	LoadBitmap("image/bak/0.bmp",72,true);					//プレーンナンバー２にシステム用ＢＭＰを読み込む

	/*LoadBitmap("image/sys/waku.bmp",109,true);*/				
	LoadBitmap("image/color.bmp",127,false);



}

void act_relese( void )
{
	long i;
	
	save_file_w( );

	for ( i = 0; i < BMPBUFF_MAX; i++ )
	{
		ReleaseBitmap( i );
	}
//	soundStopBgm(EN_BGM_GAME01);

}


void act_keys( void )
{
	long i;
	long old_x;
	long muki;
	long muki2;
	long px;
	long py;
	long kaiten;
	long rr;
	long rg;
	long ii;
	
	if ( sn == 1 )
	{
		play_time[0] = play_time[0] + 1;
		if ( play_time[0] >= 60 )
		{
			play_time[0] = 0;
			play_time[1] = play_time[1] + 1;
			if ( play_time[1] >= 60 )
			{
				play_time[1] = 0;
				play_time[2] = play_time[2] + 1;
				if ( play_time[2] >= 60 )
				{
					if ( play_time[2] >= 10 )
					{
						play_time[2] = 10;
					}
				}
			}
		}
	}
	if ( gameflag[132] == 0 )
	{
		replay_input_save( );		/* リプレイデータ保存 */
	}
	else 
	{
		replay_input_load( );		/* リプレイデータ再生 */
	}
	replay_time++;
	if ( replay_time > replay_time_MAX )
	{
		replay_time = replay_time_MAX;	/* 最大時間を越えた、保存しない */
		replay_save_flag = 1;			/* 保存しない */
	}

	for ( i = 0; i < 5; i++ )
	{
		rrkey[i] = 0;						/* 入力キー方向 */
	}


	player[17]--;			/* 無敵時間 */
	if ( player[17] <= 0 )
	{
		player[17] = 0;
	}
	if ( player[9] == 0 )
	{
		jmp( );
	}
	
	
	if ( ( item_up( ) != -1 ) && ( player[14] == 10 ) )	/* アイテムの上に乗っている */
	{
		player[1] = item_up( ); 
	}
	

	if ( player[2] == 31 )
	{
		if ( player[3] == 1 ) 
		{
//			player[0] = player[0] + 3;
//			idou_r( );
		}
		else 
		{
//			player[0] = player[0] - 3;
//			idou_l( );
		}
	}
	else 
	{
		old_x = player[0];
		
		if ( player[9] == 1 )
		{			
			muki = -1;
			if ( ( ( IsPressKey( gameflag[0] ) ) && ( gameflag[132] == 0 ) )		// 上
		  || ( ( replay_load_key[2] == 1 ) && ( gameflag[132] == 1 ) ) )
			{
				muki = 180;
			}
			else if ( ( ( IsPressKey( gameflag[1] ) ) && ( gameflag[132] == 0 ) )	// 下
		  || ( ( replay_load_key[3] == 1 ) && ( gameflag[132] == 1 ) ) )
			{
				muki = 0;
			}
			
			if ( ( ( IsPressKey( gameflag[2] ) ) && ( gameflag[132] == 0 ) )		// 左
		  || ( ( replay_load_key[0] == 1 ) && ( gameflag[132] == 1 ) ) )
			{
				if ( muki == 0 )
				{
					muki = 45;
				}
				else if ( muki == 180 )
				{
					muki = 135;
				}
				else 
				{
					muki = 90;
				}
			}
			else if ( ( ( IsPressKey( gameflag[3] ) ) && ( gameflag[132] == 0 ) )	// 右
		  || ( ( replay_load_key[1] == 1 ) && ( gameflag[132] == 1 ) ) )
			{
				if ( muki == 0 )
				{
					muki = 315;
				}
				else if ( muki == 180 )
				{
					muki = 225;
				}
				else 
				{
					muki = 270;
				}
			}
			
			if ( player[27] == muki )	/* 掴んだときの入力方向を押していた */
			{
				muki = -1;	/* 移動しない */
			}
			else 
			{
				player[27] = -1;
			}
			
			player[0] = player[32] - ( ( funcSin( player[20] ) * ( player[21] ) ) >> 16 );
			player[1] = player[33] - ( ( funcCos( player[20] ) * ( player[21] ) ) >> 16 );
		}
		else
		{
			if ( ( ( IsPressKey( gameflag[2] ) ) && ( gameflag[132] == 0 ) )		// 左
		  || ( ( replay_load_key[0] == 1 ) && ( gameflag[132] == 1 ) ) )
			{
				
				player[3] = 1;	/* 向き */
				if ( player[2] == 0 )
				{
					player[10] = 0;	/* アニメーション間隔フレーム設定 */
					player[11] = 0;	/* アニメーションカウンタ */
					player[2] = 1;	/* 歩き */
				}
				else if ( player[2] == 1 )
				{
					player[0] = player[0] - 2;	/* キャラクター移動 */
				}
				else if ( player[2] == 21 )
				{
					player[0] = player[0] - 2;	/* キャラクター移動 */
				}

			}
			else if ( ( ( IsPressKey( gameflag[3] ) ) && ( gameflag[132] == 0 ) )	// 右
		  || ( ( replay_load_key[1] == 1 ) && ( gameflag[132] == 1 ) ) )
			{
				player[3] = 0;
				if ( player[2] == 0 )
				{
					player[10] = 0;
					player[11] = 0;
					player[2] = 1;
				}
				else if ( player[2] == 1 )
				{
					player[0] = player[0] + 2;
				}
				else if ( player[2] == 21 )
				{
					player[0] = player[0] + 2;
				}
			}
			else 
			{
				if ( player[2] == 1 )
				{
					player[10] = 0;
					player[11] = 0;
					player[2] = 0;
				}
			}
			
			muki = -1;
			if ( ( ( IsPressKey( gameflag[0] ) ) && ( gameflag[132] == 0 ) )		// 上
		  || ( ( replay_load_key[2] == 1 ) && ( gameflag[132] == 1 ) ) )
			{
				muki = 0;
			}
			else if ( ( ( IsPressKey( gameflag[1] ) ) && ( gameflag[132] == 0 ) )	// 下
		  || ( ( replay_load_key[3] == 1 ) && ( gameflag[132] == 1 ) ) )
			{
				muki = 180;
			}
			
			if ( ( ( IsPressKey( gameflag[2] ) ) && ( gameflag[132] == 0 ) )		// 左
		  || ( ( replay_load_key[0] == 1 ) && ( gameflag[132] == 1 ) ) )
			{
				if ( muki == 0 )
				{
					muki = 315;
				}
				else if ( muki == 180 )
				{
					muki = 225;
				}
				else 
				{
					muki = 270;
				}
			}
			else if ( ( ( IsPressKey( gameflag[3] ) ) && ( gameflag[132] == 0 ) )	// 右
		  || ( ( replay_load_key[1] == 1 ) && ( gameflag[132] == 1 ) ) )
			{
				if ( muki == 0 )
				{
					muki = 45;
				}
				else if ( muki == 180 )
				{
					muki = 135;
				}
				else 
				{
					muki = 90;
				}
			}
			if ( old_x > player[0] )
			{
				idou_l( 0 );
			}
			else if ( old_x < player[0] )
			{
				idou_r( 0 );
			}
		}
		
		muki2 = muki;
		
		if ( muki != -1 )	/* キーが入力されている */
		{
			muki = muki - player[20];
			if ( muki < 0 )
			{
				muki = muki + 360;
			}
			
			if ( muki > 180 )
			{
				player[22] = -1;
			}
			else if ( ( muki < 180 ) && ( muki > 0 ) )
			{
				player[22] = 1;
			}
			
			
			
			if ( muki != 0 )
			{
				if ( player[9] == 0 )
				{
					for ( i = 0; i <= atk_rr1; i++ )
					{
						if ( player[20] == muki2 )
						{
							break;
						}
						player[20] = player[20] + ( player[22] * 1 );	/* アンカーの移動 */
						if ( player[20] >= 360 )
						{
							player[20] = player[20] - 360;
						}
						if ( player[20] < 0 )
						{
							player[20] = player[20] + 360;
						}
					}
				}
				else
				{
					for ( i = 0; i <= atk_rr2; i++ )
					{
						if ( player[20] == muki2 )
						{
							break;
						}
						kaiten = player[20] + ( player[22] * 1 );	/* アンカーの移動 */
						if ( kaiten >= 360 )
						{
							kaiten = kaiten - 360;
						}
						if ( kaiten < 0 )
						{
							kaiten = kaiten + 360;
						}
						px = ( ( funcSin( kaiten ) * ( player[21] * 1 ) ) >> 16 );	/* アンカー位置Ｘ */
						py = ( ( funcCos( kaiten ) * ( player[21] * 1 ) ) >> 16 );	/* アンカー位置Ｙ */
						if (
						   ( ( ( map1_item_j( player[0] + px + 16, player[1] + py + 16  ) >= 100 )
						    && ( map1_item_j( player[0] + px + 16, player[1] + py + 16  ) <= 119 ) )
						  || ( ( map1_item_j( player[0] + px + 16, player[1] + py + 16  ) >= 200 )
						    && ( map1_item_j( player[0] + px + 16, player[1] + py + 16  ) <= 219 ) ) )
						   )
						{
//							break;
						}
						player[20] = player[20] + ( player[22] * 1 );	/* アンカーの移動 */
						if ( player[20] >= 360 )
						{
							player[20] = player[20] - 360;
						}
						if ( player[20] < 0 )
						{
							player[20] = player[20] + 360;
						}
					}
				}
				
	//			if ( ( player[22] == -1 ) && ( player[20] < muki2 ) )
	//			{
	//				player[20] = muki2;
	//			}
	//			else if ( ( player[22] == 0 ) && ( player[20] > muki2 ) )
	//			{
	//				player[20] = muki2;
	//			}
				
			}
		}
		
		player[30] = ( ( funcSin( player[20] ) * ( player[21] ) ) >> 16 );	/* アンカー位置Ｘ */
		player[31] = ( ( funcCos( player[20] ) * ( player[21] ) ) >> 16 );	/* アンカー位置Ｙ */
		

		
		
		if ( player[28] == 1 )	/* 移動床 */
		{
			player[0] = player[0] + 2;
		}
		else if ( player[28] == 2 )
		{
			player[0] = player[0] - 2;
		}
		
		if ( player[29] > 0 )	/* 移動床 */
		{
			player[0] = player[0] + ( player[29] * 1 );
			player[29] = player[29] - 1;
		}
		else if ( player[29] < 0 )
		{
			player[0] = player[0] + ( player[29] * 1 );
			player[29] = player[29] + 1;
		}
		/* アンカー吹き飛び */
/*		
		if ( player[34] > 0 )
		{
			player[0] = player[0] + ( ( funcSin( player[36] ) * ( player[34] ) ) >> 16 );
			player[34] = player[34] - 8;
			if ( player[34] < 0 )
			{
				player[34] = 0;
			}
		}
		if ( player[35] > 0 )
		{
			player[1] = player[1] + ( ( funcCos( ( 32768 / 360 ) * player[36] ) * ( player[35] ) ) >> 16 );
			player[35] = player[35] - 8;
			if ( player[35] < 0 )
			{
				player[35] = 0;
			}
		}
*/		
		
		
		if ( ( ( IsPressKey( gameflag[0] ) ) && ( gameflag[132] == 0 ) )
		  || ( ( replay_load_key[2] == 1 ) && ( gameflag[132] == 1 ) ) )
		{
			/* 立ち、歩き */
			if ( ( player[2] == 0 ) || ( player[2] == 1 ) )
			{
				item_event( );
			}
		}
		else if ( ( ( IsPressKey( gameflag[1] ) ) && ( gameflag[132] == 0 ) )
		  || ( ( replay_load_key[3] == 1 ) && ( gameflag[132] == 1 ) ) )
		{
		}
		
		//	スペースキーが押される
		if ( ( ( IsPressKey( gameflag[4] ) )  && ( gameflag[132] == 0 ) ) 		/* 移動速度低下 */
		  || ( ( replay_load_key[4] == 1 ) && ( gameflag[132] == 1 ) ) )
		{
			if ( player[24] == 1 )	/* アンカー状態からジャンプした or 地上 */
			{
			}
			else 
			{
				if ( player[38] == 0 )
				{
					soundPlaySe( EN_SE_PAWA );
				}
				player[38] = 1;
				
				for ( i = 0; i < 20; i++ )
				{
					player[21] = player [21] + 1;
					player[23] = 1;
					if ( player[21] > 100 )
					{
						player[21] = 100;
						player[23] = 0;
					}
					if ( player[9] == 1 )
					{
						if ( player[21] > player[19] )
						{
							player[21] = player[19];
						}
					}

					player[30] = ( ( funcSin( player[20] ) * ( player[21] ) ) >> 16 );	/* アンカー位置Ｘ */
					player[31] = ( ( funcCos( player[20] ) * ( player[21] ) ) >> 16 );	/* アンカー位置Ｙ */
/*
					if (
					   ( ( ( map1_item_j( player[0] + player[30] + 0, player[1] + player[31] + 0  ) >= 100 )
					    && ( map1_item_j( player[0] + player[30] + 0, player[1] + player[31] + 0  ) <= 119 ) )
					  || ( ( map1_item_j( player[0] + player[30] + 0, player[1] + player[31] + 0  ) >= 200 )
					    && ( map1_item_j( player[0] + player[30] + 0, player[1] + player[31] + 0  ) <= 219 ) ) )
					|| ( ( ( map1_item_j( player[0] + player[30] + 32, player[1] + player[31] + 32  ) >= 100 )
					    && ( map1_item_j( player[0] + player[30] + 32, player[1] + player[31] + 32  ) <= 119 ) )
					  || ( ( map1_item_j( player[0] + player[30] + 32, player[1] + player[31] + 32  ) >= 200 )
					    && ( map1_item_j( player[0] + player[30] + 32, player[1] + player[31] + 32  ) <= 219 ) ) )
					   )
*/
					if (
					     ( ( player[0] + player[30] + 10 ) < -15 ) 
					  || ( ( player[0] + player[30] + 22 ) > 640 )
//					  || ( ( player[1] + player[31] + 0 ) < 0 )
//					  || ( ( player[1] + player[31] + 32 ) > 480 )
					   )
					{
					}
					else 
					{
						if (
						   ( ( ( map1_item_j( player[0] + player[30] + 16, player[1] + player[31] + 16  ) >= 100 )
						    && ( map1_item_j( player[0] + player[30] + 16, player[1] + player[31] + 16  ) <= 118 ) )
						  || ( ( map1_item_j( player[0] + player[30] + 16, player[1] + player[31] + 16  ) >= 200 )
						    && ( map1_item_j( player[0] + player[30] + 16, player[1] + player[31] + 16  ) <= 218 ) ) )
						   )
						{
							if   ( player[23] == 1 )	/* 伸ばしている */
							{
								if ( player[9] == 0 )
								{
									player[10] = 0;
									player[11] = 0;
									player[2] = 21;		/* 空中 */
									player[13] = 0;
									player[14] = -1;
									px = ( player[0] + player[30] + 16 ) / 32;
									py = ( ( ( ( player[1] + player[31] + 16 ) + 16 ) / 32 ) );
	//								player[32] = player[0] + player[30];
	//								player[33] = player[1] + player[31];
									player[32] = px * 32;
									player[33] = ( py * 32 ) - 16;
									
									px = player[0] - player[32];
									py = player[1] - player[33];
									rg = funcTan2( px, py ) ;
									rr = ( rg * 360 ) / 65535 ;
									player[20] = rr + 180;
									
									
									rr = GetKyori(
													  labs( player[0] - player[32] ) ,
													  labs( player[1] - player[33] )
													); 
									player[21] = rr;
									if ( player[21] < 37 )
									{
										player[21] = 37;
									}
									player[19] = player[21];
									
									
									player[27] = -1;

									if ( ( ( IsPressKey( gameflag[0] ) ) && ( gameflag[132] == 0 ) )
								  || ( ( replay_load_key[2] == 1 ) && ( gameflag[132] == 1 ) ) )
									{
										player[27] = 180;
									}
									else if ( ( ( IsPressKey( gameflag[1] ) ) && ( gameflag[132] == 0 ) )
								  || ( ( replay_load_key[3] == 1 ) && ( gameflag[132] == 1 ) ) )
									{
										player[27] = 0;
									}
									
									if ( ( ( IsPressKey( gameflag[2] ) ) && ( gameflag[132] == 0 ) )
								  || ( ( replay_load_key[0] == 1 ) && ( gameflag[132] == 1 ) ) )
									{
										if ( player[27] == 0 )
										{
											player[27] = 45;
										}
										else if ( player[27] == 180 )
										{
											player[27] = 135;
										}
										else 
										{
											player[27] = 90;
										}
									}
									else if ( ( ( IsPressKey( gameflag[3] ) ) && ( gameflag[132] == 0 ) )
								  || ( ( replay_load_key[1] == 1 ) && ( gameflag[132] == 1 ) ) )
									{
										if ( player[27] == 0 )
										{
											player[27] = 315;
										}
										else if ( player[27] == 180 )
										{
											player[27] = 225;
										}
										else 
										{
											player[27] = 270;
										}
									}

//									muki2 = player[27];
									soundPlaySe( EN_SE_ATK1 );
									p_shot_f2_set( player[32]  + 14, player[33]  + 14, 0 );
								}
								player[9] = 1;	/* 掴んでいる */
								break;
							}
						}
						else 
						{
							for ( ii = 0; ii < 50; ii++ )
							{
								if ( item[( ii * 10 ) + 0 ] == 1 )
								{
									/* プレイヤーとの当たり判定 */
									if ( ( ( (item[1 + ( ii * 10 )] + 2 ) < ( player[0] + player[30] + 16 ) )	/* 自機とのあたり判定 */
										&& ( ( player[0] + player[30] + 16 ) < (item[1 + ( ii * 10 )] + 30 ) )
										&& ( (item[2 + ( ii * 10 )]  + 2 ) < ( player[1] + player[31] + 16 ) )
										&& ( ( player[1] + player[31] + 16 ) < ( item[2 + ( ii * 10 )] + 30 ) ) )
										&& ( sn == 1 ) )
									{
										if ( 
										     ( item[( ii * 10 ) + 3 ] == 703 )
										  || ( item[( ii * 10 ) + 3 ] == 704 )
										   )
										{
											if   ( player[23] == 1 )	/* 伸ばしている */
											{
												if ( player[9] == 0 )
												{
													player[37] = ii;		/* アイテムを掴んでいる */
													player[10] = 0;
													player[11] = 0;
													player[2] = 21;		/* 空中 */
													player[13] = 0;
													player[14] = -1;
													px = ( player[0] + player[30] + 16 ) / 32;
													py = ( ( ( ( player[1] + player[31] + 16 ) + 16 ) / 32 ) );
					//								player[32] = player[0] + player[30];
					//								player[33] = player[1] + player[31];
													player[32] = item[1 + ( ii * 10 )];
													player[33] = item[2 + ( ii * 10 )];
													
													px = player[0] - player[32];
													py = player[1] - player[33];
													rg = funcTan2( px, py ) ;
													rr = ( rg * 360 ) / 65535 ;
													player[20] = rr + 180;
													
													
													rr = GetKyori(
																	  labs( player[0] - player[32] ) ,
																	  labs( player[1] - player[33] )
																	); 
													player[21] = rr;
													if ( player[21] < 37 )
													{
														player[21] = 37;
													}
													player[19] = player[21];
													
													
													player[27] = -1;
													if ( ( ( IsPressKey( gameflag[0] ) ) && ( gameflag[132] == 0 ) )
												  || ( ( replay_load_key[2] == 1 ) && ( gameflag[132] == 1 ) ) )
													{
														player[27] = 180;
													}
													else if ( ( ( IsPressKey( gameflag[1] ) ) && ( gameflag[132] == 0 ) )
												  || ( ( replay_load_key[3] == 1 ) && ( gameflag[132] == 1 ) ) )
													{
														player[27] = 0;
													}
													
													if ( ( ( IsPressKey( gameflag[2] ) ) && ( gameflag[132] == 0 ) )
												  || ( ( replay_load_key[0] == 1 ) && ( gameflag[132] == 1 ) ) )
													{
														if ( player[27] == 0 )
														{
															player[27] = 45;
														}
														else if ( player[27] == 180 )
														{
															player[27] = 135;
														}
														else 
														{
															player[27] = 90;
														}
													}
													else if ( ( ( IsPressKey( gameflag[3] ) ) && ( gameflag[132] == 0 ) )
												  || ( ( replay_load_key[1] == 1 ) && ( gameflag[132] == 1 ) ) )
													{
														if ( player[27] == 0 )
														{
															player[27] = 315;
														}
														else if ( muki == 180 )
														{
															player[27] = 225;
														}
														else 
														{
															player[27] = 270;
														}
													}
													
												soundPlaySe( EN_SE_ATK1 );
													p_shot_f2_set( player[32] + 14, player[33] + 14, 0 );
												}
												player[9] = 1;	/* 掴んでいる */
												break;
											}
										}
									}
								}
							}
						}
	//						else 
	//						{
	//				//				player[9] = 0;	/* 掴んでいる */
	//						}
					}

				}
				if ( player[9] == 1 )
				{
					idou_r( 1 );
					idou_l( 1 );
				}
			}
		}
		else 
		{
			player[21] = player [21] - 10;
			if ( player[21] < 32 )
			{
				player[21] = 32;
			}
			player[23] = 0;
			
			if ( player[9] == 1 )	/* 敵を掴んでいる場合 */
			{
				player[14] = -1;	/*  */
				player[13] = 0;
//				player[36] = player[20];
//				player[34] = player[21] - 32; /* アンカー吹っ飛びX */
//				player[35] = player[21] - 32; /* アンカー吹っ飛びY */
			}
			player[9] = 0;	/*  */
			player[37] = -1;
			player[24] = 0;
			player[38] = 0;
		}

		
		
		//Bが押される
		if ( ( ( ( IsPushKey( gameflag[5] ) ) || IsPushKey( gameflag[5] ) ) && ( gameflag[132] == 0 ) )
		  || ( ( replay_load_key[6] == 1 ) && ( gameflag[132] == 1 ) ) )
		{
			if ( ( player[2] != 11 ) && ( player[2] != 31 ) && ( player[14] == 10 ) )	/* 地上で攻撃、ダメージじゃない */
			{
				Jump_counts++;
				soundPlaySe( EN_SE_JUMP );
				/* アイテムに乗っていない場合はそこがリトライ位置 */
				if ( item_up( ) == -1 )
				{
					player[15] = player[0];
					player[16] = player[1];
				}
				player[10] = 0;
				player[11] = 0;
				player[2] = 21;	/* 空中 */
				player[14] = 5;
				player[13] = 0;
				player[25] = 1;
				jmp_st( );
				player[24] = 0;
			}
			if ( player[9] == 1 )
			{
				Jump_counts++;
				soundPlaySe( EN_SE_JUMP );
				/* アイテムに乗っていない場合はそこがリトライ位置 */
				if ( item_up( ) == -1 )
				{
					player[15] = player[0];
					player[16] = player[1];
				}
				player[10] = 0;
				player[11] = 0;
				player[2] = 21;	/* 空中 */
				player[14] = 5;
				player[13] = 0;
				player[25] = 1;
				jmp_st( );
				player[9] = 0;
				player[21] = 32;
				player[23] = 0;
				player[24] = 1;
			}
		}
		if ( ( ( IsPressKey( gameflag[5] ) ) && ( gameflag[132] == 0 ) )
		  || ( ( replay_load_key[5] == 1 ) && ( gameflag[132] == 1 ) ) )
		{
		}
		else 
		{
			if ( player[14] == 10 )
			{
			}
			else 
			{
				if ( ( player[14] > 0 ) && ( player[25] == 1 ) )	/* 途中で落下 */
				{
//					player[14] = -1;
				}
			}
			player[25] = 0;
		}
		
		
	}
	if ( Jump_counts > 99 )
	{
		Jump_counts = 99;
	}
	
	/* 12ボタン　か　Ｃが押された */
	if ( IsPushKey( gameflag[6] ) )
	{
		sn = 4;
		demo[0] = 0;
	}
	
	/* スキルショートカットウィンドウ */
	if ( IsPushKey( gameflag[7] ) )
	{
	}



	if ( player[0] < 0 )
	{
		player[0] = 0;
//		cleaer_ankr( );
		if ( player[9] == 1 ) 
		{
//			player[32] = px * 32;
//			player[33] = ( py * 32 ) - 16;
//			
//			px = player[0] - player[32];
//			py = player[1] - player[33];
//			rg = funcTan2( px, py ) ;
//			rr = ( rg * 360 ) / 65535 ;
//			player[20] = rr + 179;
			
			
			rr = GetKyori(
							  labs( player[0] - player[32] ) ,
							  labs( player[1] - player[33] )
							); 
			player[21] = rr;
			if ( player[21] < 37 )
			{
				player[21] = 37;
			}
			player[19] = player[21];
		}
		if ( save_data[9] == 1 )	/* テレポータ使用不可 */
		{
		}
		else if ( ( item_up( ) != -1 ) )
		{
			/* アイテムに乗っている場合はスクロールしない */
		}
		else 
		{
			if ( player[1] <= 460 )
			{
				if ( map_data[4] == 1 )
				{
					wait_scl( 2, map_data[5] );
				}
			}
		}
	}
	if ( player[0] > ( 640 - 32 ) )
	{
		player[0] = 640 - 32;
//		cleaer_ankr( );
		if ( player[9] == 1 ) 
		{
//			player[32] = px * 32;
//			player[33] = ( py * 32 ) - 16;
//			
//			px = player[0] - player[32];
//			py = player[1] - player[33];
//			rg = funcTan2( px, py ) ;
//			rr = ( rg * 360 ) / 65535 ;
//			player[20] = rr + 179;
			
			
			rr = GetKyori(
							  labs( player[0] - player[32] ) ,
							  labs( player[1] - player[33] )
							); 
			player[21] = rr;
			if ( player[21] < 37 )
			{
				player[21] = 37;
			}
			player[19] = player[21];
		}
		if ( save_data[9] == 1 )	/* テレポータ使用不可 */
		{
		}
		else if ( ( item_up( ) != -1 ) )
		{
		}
		else 
		{
			if ( player[1] <= 460 )
			{
				if ( map_data[6] == 1 )
				{
					wait_scl( 3, map_data[7] );
				}
			}
		}
	}
	if ( player[1] < -30 )	/* 画面下 */
	{
		cleaer_ankr( 1 );	/*  */
	}
	if ( player[1] <= 0 )	/* 画面下 */
	{
		if ( save_data[9] == 1 )	/* テレポータ使用不可 */
		{
		}
		else 
		{
			if ( map_data[2] == 1 )
			{
				wait_scl( 1, map_data[3] );
			}
		}
	}
	if ( player[1] >= 450 )	/* 画面上 */
	{
		if ( save_data[9] == 1 )	/* テレポータ使用不可 */
		{
		}
		else 
		{
			if ( map_data[0] == 1 )
			{
				wait_scl( 0 , map_data[1] );
			}
		}
	}

}

void fram_set( )
{
	if ( player[2] == 0 )	/* ニュートラル */
	{
		player[12] = 20;
	}
	if ( player[2] == 1 )	/* 歩き */
	{
		player[12] = 5;
	}
	if ( player[2] == 11 )	/* 攻撃 */
	{
		player[12] = 2;
	}
	if ( player[2] == 21 )	/* 空中 */
	{
		player[12] = 5;
	}
	if ( player[2] == 31 )	/* やられ */
	{
		player[12] = 3;
	}
	if ( player[2] == 33 )	/* クリア */
	{
		player[12] = 10;
	}
	if ( player[2] == 61 )	/* 上向き */
	{
		player[12] = 10;
	}
}


/* すい中の場合はジャンプ力増加 */
void jmp_st( )
{
	long px1 ;
	long px2 ;
	long py ;
	long py2 ;

	px1 = ( player[0] + 10 ) / 32;
	px2 = ( player[0] + 22 ) / 32;
	py = ( 16 - ( ( player[1] + 28 ) / 32 ) );
	py2 = ( 16 - ( ( player[1] + 32 + 28 ) / 32 ) );
//	py = ( 16 - ( ( player[1] ) / 32 ) );
		/* 水中判定 */
	if ( ( map1[ px1 + ( ( py2 ) * 20 ) ] == 319 ) 
	  || ( map1[ px2 + ( ( py2 ) * 20 ) ] == 419 )  
	  || ( map1[ px1 + ( ( py2 ) * 20 ) ] == 318 )  
	  || ( map1[ px2 + ( ( py2 ) * 20 ) ] == 418 )  
	  || ( map1[ px1 + ( ( py2 ) * 20 ) ] == 719 )
	  || ( map1[ px1 + ( ( py2 ) * 20 ) ] == 718 ) )
	{
		player[14] = player[14] + 1; 		/* ジャンプフラグ */
	}
	else 
	{
	}
}

/* ジャンプＹ位置計算 */
void jmp( )
{
	long y1;
	long px ;
	long px1 ;
	long px2 ;
	long py ;
	long py3 ;
	

	player[13]++;
	if ( player[13] >= 11 )
	{
		player[13] = 0;
	}
	
	if ( player[14] == 10 ) 
	{
		/* 今回の位置 */
		px = ( player[0] + 16 ) / 32;
		px1 = ( player[0] + 10 ) / 32;
		px2 = ( player[0] + 21 ) / 32;
		py = ( 16 - ( ( player[1] + 28 ) / 32 ) );
		/* 100 ～ 200は地面 */
		if (
		   ( ( !( ( map1[ px1 + ( ( py ) * 20 ) ] >= 100 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 119 ) ) )
		  && ( !( ( map1[ px2 + ( ( py ) * 20 ) ] >= 100 ) && ( map1[ px2 + ( ( py ) * 20 ) ] <= 119 ) ) ) 
		  && ( !( ( map1[ px1 + ( ( py ) * 20 ) ] >= 200 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 219 ) ) ) 
		  && ( !( ( map1[ px2 + ( ( py ) * 20 ) ] >= 200 ) && ( map1[ px2 + ( ( py ) * 20 ) ] <= 219 ) ) )
		   )
		  || ( ( ( player[1] - 16 ) % 32 ) != 0 ) )
			{
			if ( item_up( ) != -1 )	/* アイテムの上に乗っている */
			{
				player[1] = item_up( ); 
				return;
			}
			
			if ( ( player[2] == 0 )
			  || ( player[2] == 1 ) )
			{
				player[10] = 0;
				player[11] = 0;
				player[2] = 21;		/* 空中 */
			}
			player[14] = -1;
			player[13] = 0;
			
			if ( player[2] == 31 )	/* ダメージ中 */
			{
			}
		}
		else
		{
			return;
		}
	}

	if ( player[13] == 0 )
	{
		player[14] = player[14] - 2;
//		if ( player[14] == 0 )
//		{
//			player[14] = -3;
//		}

		if ( ( player[14] <= 1 ) && ( player[14] > -2 ) )
		{
			player[14] = -1;
		}

		if ( player[14] < -4 )
		{
			player[14] = -4;
			player[26]++;		/* 着地フェクト用 */
		}
		else 
		{
			player[26] = 0;
		}
		
		/* 地面判定 */
	}
	
	/* 今回の位置 */
	y1 = ( ( 0 - player[14] ) * ( 0 - player[14] ) * ( 0 - player[14] ) );
//	player[1] = player[1] - ( y1 / 30 );

	px1 = ( player[0] + 10 ) / 32;
	px2 = ( player[0] + 21 ) / 32;
	py = ( 16 - ( ( player[1] + 28 ) / 32 ) );
//	py = ( 16 - ( ( player[1] ) / 32 ) );
		/* 水中判定 */
	if ( ( map1[ px1 + ( ( py ) * 20 ) ] == 319 ) 
	  || ( map1[ px2 + ( ( py ) * 20 ) ] == 419 )  
	  || ( map1[ px1 + ( ( py ) * 20 ) ] == 318 )  
	  || ( map1[ px2 + ( ( py ) * 20 ) ] == 418 )  
	  || ( map1[ px1 + ( ( py ) * 20 ) ] == 719 )
	  || ( map1[ px1 + ( ( py ) * 20 ) ] == 718 ) )
	{
		if ( player[14] > 0 ) /* 上昇中 */
		{
/*			player[1] = player[1] - ( y1 / 6 ); */
			player[1] = player[1] - ( y1 / 18 );
		}
		else 
		{
			player[1] = player[1] - ( y1 / 40 );
		}
	}
	else 
	{
		player[1] = player[1] - ( y1 / 20 );
	}

	if ( player[1] > 480 ) 
	{
		return;
	}
	py3 = ( 16 - ( ( player[1] + 32 + 32 + 16 ) / 32 ) );

	if ( player[14] <= 0 )													/* 上り中は判定しない */
	{
		/* 地面判定 */
		if ( ( ( map1[ px1 + ( ( py ) * 20 ) ] >= 100 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 119 ) )
		  || ( ( map1[ px1 + ( ( py ) * 20 ) ] >= 200 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 219 ) ) 
		  || ( ( map1[ px2 + ( ( py ) * 20 ) ] >= 100 ) && ( map1[ px2 + ( ( py ) * 20 ) ] <= 119 ) ) 
		  || ( ( map1[ px2 + ( ( py ) * 20 ) ] >= 200 ) && ( map1[ px2 + ( ( py ) * 20 ) ] <= 219 ) ) )
		{
			if ( player[8] > 0 )
			{ 
				soundPlaySe( EN_SE_LANDING );
			}
			if ( ( player[2] != 31 ) && ( player[2] != 32 ) )	/* ダメージ中じゃない */
			{
				player[2] = 0;												/* 着地 */
				player[11] = 0;												/* 硬直解除 */
				player[18] = 10;
			}
			player[1] = ( ( 16 - py ) * 32 ) - 16; 
			/* 着地時の煙を生成 */
			if ( player[26] >= 2 )
			{
				p_jump_f_set( player[0], player[1] );
			}
			player[14] = 10;
		}
		else 
		{
			if ( item_up( ) != -1 )	/* アイテムの上に乗っている */
			{
				soundPlaySe( EN_SE_LANDING );
				if ( ( player[2] != 31 ) && ( player[2] != 32 ) )	/* ダメージ中じゃない */
				{
					player[2] = 0;												/* 着地 */
					player[11] = 0;												/* 硬直解除 */
				}
				player[1] = item_up( ); 
				player[14] = 10;
				player[18] = 10;
			}
		}
	}
	else		/* 上昇中 */
	{
		/* 地面判定 */
		if ( ( ( map1[ px1 + ( ( py3 ) * 20 ) ] >= 100 ) && ( map1[ px1 + ( ( py3 ) * 20 ) ] <= 119 ) )
		  || ( ( map1[ px1 + ( ( py3 ) * 20 ) ] >= 200 ) && ( map1[ px1 + ( ( py3 ) * 20 ) ] <= 219 ) )
		  || ( ( map1[ px2 + ( ( py3 ) * 20 ) ] >= 100 ) && ( map1[ px2 + ( ( py3 ) * 20 ) ] <= 119 ) )
		  || ( ( map1[ px2 + ( ( py3 ) * 20 ) ] >= 200 ) && ( map1[ px2 + ( ( py3 ) * 20 ) ] <= 219 ) ) )
		{
			player[13] = 0;
			player[14] = -3;
			player[26] = 0;
		}
		else 
		{
			if ( item_up2( ) != -1 )	/* アイテムに頭がぶつかる */
			{
				player[1] = item_up2( ); 
				player[13] = 0;
				player[14] = -3;
				player[26] = 0;
			}
		}
	}
	if ( player[1] <= -32 )
	{
		if ( map_data[2] == 0 )	/* スクロールしない場合はダメージ */
		{
			if ( player[8] > 0 )
			{ 
				if ( player[17] == 0 )
				{
					player[8] = player[8] - 1;
				}
				if ( player[17] == 0 )
				{
					soundPlaySe( EN_SE_DAMEGE );
				}
				if ( player[8] > 0 )
				{ 
					player[0] = player[15];
					player[1] = player[16];
				}
				else
				{
					
					cleaer_ankr( 1 );
					player[1] = -32 ;
				} 
			}
		}
	}

}

void idou_l( long ankr_flag )
{
	long px ;
	long py1 ;
	long py2 ;

	long i;


	/* 今回の位置 */
	px = ( player[0] + 10 + player[29] ) / 32;
	py1 = ( 16 - ( ( player[1] + 49 + 16  ) / 32 ) );
	py2 = ( 16 - ( ( player[1] + 52 ) / 32 ) );
	if ( py2 <= 0 )	/* 画面外は一番上で判定 */
	{
		py2 = 1;
	}

	/* 502はしご上　501はしご中 */
	if ( ( ( map1[ px + ( ( py1 ) * 20 ) ] >= 100 ) && ( map1[ px + ( ( py1 ) * 20 ) ] <= 299 ) )
	  || ( ( map1[ px + ( ( py2 ) * 20 ) ] >= 100 ) && ( map1[ px + ( ( py2 ) * 20 ) ] <= 299 ) ) )
	{
		cleaer_ankr( ankr_flag );
		
		px = ( player[0] + 16 ) / 32;
		player[0] = px * 32 - 10;
		player[29] = 0;

	}
	else 
	{
	
	}
	for ( i = 0; i < 50; i++ )
	{
		if ( item[( i * 10 ) + 0 ] == 1 )
		{
			/* プレイヤーとの当たり判定 */
			if ( ( ( (item[1 + ( i * 10 )]  ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
				&& ( (player[0] + 10 + player[29] ) < (item[1 + ( i * 10 )] + 32 ) )
				&& ( (item[2 + ( i * 10 )] ) < (player[1] + 32 ) )
				&& ( (player[1] + 5 ) < ( item[2 + ( i * 10 )] + 32 ) ) ) )
			{
				if ( ( ( item[( i * 10 ) + 3 ] >= 500 ) && ( item[( i * 10 ) + 3 ] <= 620 ) ) /* 移動不可アイテム */
				    || ( item[( i * 10 ) + 3 ] == 7 ) )
				{
					cleaer_ankr( ankr_flag );
		
					px = ( player[0] + 16 ) / 32;
					player[0] = px * 32 - 10;
					player[29] = 0;
				}
			}
		}
	}

}
/* 自分の左右に移動可能か？ */
void idou_r( long ankr_flag )
{
	long px ;
	long py1 ;
	long py2 ;

	long i;

	/* 今回の位置 */
	px = ( player[0] + 21 + player[29]  ) / 32;
	py1 = ( 16 - ( ( player[1] + 49 + 16 ) / 32 ) );
	py2 = ( 16 - ( ( player[1] + 52 ) / 32 ) );
	if ( py2 <= 0 )	/* 画面外は一番上で判定 */
	{
		py2 = 1;
	}

	/* 502はしご上　501はしご中 */
	if ( ( ( map1[ px + ( ( py1 ) * 20 ) ] >= 100 ) && ( map1[ px + ( ( py1 ) * 20 ) ] <= 299 ) )
	  || ( ( map1[ px + ( ( py2 ) * 20 ) ] >= 100 ) && ( map1[ px + ( ( py2 ) * 20 ) ] <= 299 ) ) )
	{
		cleaer_ankr( ankr_flag );
		
		px = ( player[0] + 16 ) / 32;
		player[0] = px * 32 + 10;
		player[29] = 0;
	}
	else 
	{
	}
	for ( i = 0; i < 50; i++ )
	{
		if ( item[( i * 10 ) + 0 ] == 1 )
		{
			/* プレイヤーとの当たり判定 */
			if ( ( ( (item[1 + ( i * 10 )]  ) < (player[0] + 22 + player[29] ) )	/* 自機とのあたり判定 */
				&& ( (player[0] + 10 ) < (item[1 + ( i * 10 )] + 32 ) )
				&& ( (item[2 + ( i * 10 )] ) < (player[1] + 32 ) )
				&& ( (player[1] + 5 ) < ( item[2 + ( i * 10 )] + 32 ) ) ) )
			{
				if ( ( ( item[( i * 10 ) + 3 ] >= 500 ) && ( item[( i * 10 ) + 3 ] <= 620 ) ) /* 移動不可アイテム */
				    || ( item[( i * 10 ) + 3 ] == 7 ) )
				{
					cleaer_ankr( ankr_flag );
			
					px = ( player[0] + 16 ) / 32;
					player[0] = px * 32 + 10;
					player[29] = 0;
				}
			}
		}

	}

}

void cleaer_ankr( long ankr_flag )
{
	if ( ankr_flag != 0 )
	{
		player[24] = 1;
	}
	player[9] = 0;
	player[21] = 32;
	player[23] = 0;
	player[37] = -1;
}

void act_drow( void )
{
	//-------------------------------------------------------------
	//変数宣言
	//-------------------------------------------------------------

	//背景クリア
	ClearSecondary();

	switch( sn )							/* snによって描画するものを変える */
	{
	case 0:		//開始デモ
		bakdisp1( );
		playerdisp( );
		enm_disp( );
		bakdisp2( );
		stage_start( );
		debugdisp( );
		//-----------------------------------------------------------------------------------
		break;
	case 1 :		//アクション
		act_keys( );
		bakdisp1( );
		item_disp( );
		kane_disp( );
		enm_disp( );
		e_shot_disp( );
		playerdisp( );
		p_jump_f_disp( );
		p_shot_disp( );
		p_mgc_disp( );
		bakdisp2( );
		p_shot_f2_disp( );
		p_shot_f_disp( );
		minimap_disp( );
		snack_counter_disp( ); // おやつカウンタ
		mapdisp( );
		debugdisp( );		/* デバッグ情報 */
		stage_endj( );
		//-----------------------------------------------------------------------------------
		break;
	case 2 :		//クリアデモ
		bakdisp1( );
		item_disp( );
//			enm_disp( );
//			e_shot_disp( );
//			jmp( );
		playerdisp( );
//			p_shot_disp( );
//			p_mgc_disp( );
		bakdisp2( );
		p_shot_f2_disp( );
		stage_claer( );
		minimap_disp( );
		snack_counter_disp( ); // おやつカウンタ
		mapdisp( );
		debugdisp( );
		//-----------------------------------------------------------------------------------
		break;
	case 3 :		//ゲームオーバー
		bakdisp1( );
		item_disp( );
		enm_disp( );
		e_shot_disp( );
		jmp( );
		playerdisp( );
		p_shot_disp( );
		p_mgc_disp( );
		bakdisp2( );
		p_shot_f2_disp( );
		minimap_disp( );
		snack_counter_disp( ); // おやつカウンタ
		mapdisp( );
		stage_end( );
		debugdisp( );
		//-----------------------------------------------------------------------------------
		break;
	case 4 :		//アイテム
		window_keys( );
		bakdisp1( );
		item_disp( );
		kane_disp( );
		enm_disp( );
		e_shot_disp( );
		playerdisp( );
		p_jump_f_disp( );
		p_shot_disp( );
		p_mgc_disp( );
		bakdisp2( );
		p_shot_f2_disp( );
		p_shot_f_disp( );
		minimap_disp( );
		snack_counter_disp( ); // おやつカウンタ
		mapdisp( );
		window_disp( );
		debugdisp( );
		//-----------------------------------------------------------------------------------
		break;
	case 5 :		//イベント
		//-----------------------------------------------------------------------------------
		break;
	default :	//	どの条件にも満たない場合
		
		/* 画面切り替え時の表示対策 */
		player[0] = next_px;
		player[1] = next_py;
		
		scene_exit = 0;
		break;
	}

	RefreshScreen( g_screen );	
	KeyInput();				
}

/* 一時ファイル保存 */
void save_file_w( )
{
	long i;
	char path_config[512];
	char path_item[512];
	char path_work[512];
	char path_folder[512];
	
#ifdef MINGW
	sprintf(path_config, "save/config");
	sprintf(path_folder, "save");

	sprintf(path_work, "save/work.sav");
	sprintf(path_item, "save/item_wk.sav");
#elif defined(DREAMCAST)

	sprintf(path_config, "/ram/config");
	sprintf(path_folder, "/ram");

	sprintf(path_work, "/ram/work.sav");
	sprintf(path_item, "/ram/item_wk.sav");
	
#elif defined(_TINSPIRE)
	sprintf(path_config, "./save/config.tns");
	sprintf(path_folder, "./save");

	sprintf(path_work, "./save/work.sav.tns");
	sprintf(path_item, "./save/item_wk.sav.tns");
#else		
	sprintf(path_config, "%s/.ganbare/config", getenv("HOME"));
	sprintf(path_folder, "%s/.ganbare", getenv("HOME"));

	sprintf(path_work, "%s/.ganbare/work.sav", getenv("HOME"));
	sprintf(path_item, "%s/.ganbare/item_wk.sav", getenv("HOME"));
#endif
	
#ifdef MINGW
	mkdir(path_folder);
#elif defined(_TINSPIRE)
	mkdir(path_folder, 0755);	
#elif defined(DREAMCAST)

#else	
	mkdir(path_folder, 0755);
#endif
	
	printf("SaveGameFlag(path_config); save_file_w\n");
	SaveGameFlag(path_config);
	
	ResetGameFlag2();
	save_data[5] = player[8];
	save_data[2] = stage;
	
	save_data[0] = player[0];	
	save_data[1] = player[1];
	save_data[4] = player[3];
	for ( i = 0; i < 512; i++ )
	{
		gameflag2[i] = save_data[i];
	}
	SaveGameFlag2(path_work);
	
	ResetGameFlag2();				
	for ( i = 0; i < 500; i++ )
	{
		gameflag2[i] = item_wk[i];
	}
	SaveGameFlag2(path_item);
}


/**********************************/
/* プレイヤーの状態決定処理       */
/**********************************/

void playerdisp( )
{
	long i;
	long arm_muki;
	
	arm_muki = 0;
	
	player[10]++;
	if ( player[10] >= player[12] )
	{
		player[10] = 0;
		
		player[11]++;
		if ( player[11] >= 8 )
		{
			player[11] = 0;
			if ( player[2] == 11 )	/* 攻撃終了 */
			{
				player[2] = 0;
				if ( player[14] != 10 )	/* 空中 */
				{
					player[2] = 21;
				}
			}
			if ( player[2] == 31 )	/* ダメージ */
			{
				player[2] = 0;
				if ( player[14] != 10 )	/* 空中 */
				{
					player[2] = 21;
				}
			}
			if ( player[2] == 32 )	/* ダウンゲームオーバー */
			{
				player[2] = 32;
				player[11] = 7;
			}
			if ( player[2] == 33 )	/* クリア */
			{
				player[2] = 33;
				player[11] = 7;
			}
		}
	}
	fram_set( );
	
	if ( player[0] < 160 )
	{
		dp_x = player[0];
	}
	else if ( player[0] >= 480 )
	{
		dp_x = player[0] - 320;
	}
	else 
	{
		dp_x = 160;
	}
	
	if ( player[1] < 120 )
	{
		dp_y = player[1];
	}
	else if ( player[1] >= 360 - 16 )
	{
		dp_y = player[1] - 240 + 16;
	}
	else 
	{
		dp_y = 120;
	}
	
	
	if ( player[2] == 0 )
	{
		player[4] = 0;
		player[5] = 0;
		if ( player[18] > 0 )
		{
			player[4] = 0;
			player[5] = 7;
			player[11] = 8;
			if ( player[28] == 1 )	/* 移動床 */
			{
				player[3] = 0;
				player[4] = 1;
				player[5] = 2;
				player[11] = 0;
			}
			if ( player[28] == 2 )	/* 移動床 */
			{
				player[3] = 1;
				player[4] = 1;
				player[5] = 2;
				player[11] = 0;
			}
		}
		else 
		{
			if ( player[11] == 8 )
			{
				player[11] = 0;
			}
		}
		if ( player[28] == 1 )	/* 移動床 */
		{
			player[3] = 0;
			player[4] = 1;
			player[5] = 2;
		}
		if ( player[28] == 2 )	/* 移動床 */
		{
			player[3] = 1;
			player[4] = 1;
			player[5] = 2;
		}
	}
	if ( player[18] > 0 )	/* 着地モーション表示 */
	{
		player[18]--;
	}
	if ( player[2] != 0 )
	{
		player[18] = 0;
	}
	else 
	{
	}

	
	if ( player[2] == 1 )		/* 歩き */
	{
		player[4] = 0;
		player[5] = 1;
		if ( player[28] == 1 )	/* 移動床 */
		{
			player[3] = 0;
			player[4] = 1;
			player[5] = 2;
		}
		if ( player[28] == 2 )	/* 移動床 */
		{
			player[3] = 1;
			player[4] = 1;
			player[5] = 2;
		}
	}
	if ( player[2] == 11 )
	{
		player[4] = 0;
		player[5] = 3;
	}
	if ( player[2] == 21 )
	{
		player[4] = 0;
		player[5] = 2;
		if ( player[14] < 0 )	/* 落ちてる */
		{
			player[5] = 7;
		}
	}
	if ( player[2] == 31 )	/* ダメージ */
	{
		player[4] = 0;
		player[5] = 4;

	}
	if ( player[2] == 32 )	/* ゲームオーバー */
	{
		player[4] = 1;
		player[5] = 0;

	}
	if ( player[2] == 33 )	/* クリア */
	{
		player[4] = 0;
		player[5] = 8;

	}
	if ( player[2] == 61 )	/* 上向き */
	{
		player[4] = 0;
		player[5] = 0;
		player[11] = 8;
		player[10] = 0;
	}

	if ( player[29] != 0 )
	{
		player[4] = 0;
		player[5] = 2;
	}
	
	/* アンカー */
/*
	player[20]
	player[21]
	player[22]
*/
//	csl_x[i] = ( dp_x + ( char_size[0] / 2 ) ) + ( ( funcSin( ( 32768 / 360 ) * ( player[36] + ( 90 * i ) ) ) * find_dot ) >> 16 );
//	csl_y[i] = ( dp_y - ( char_size[1] / 2 ) ) + ( ( funcCos( ( 32768 / 360 ) * ( player[36] + ( 90 * i ) ) ) * find_dot ) >> 16 );

//	BltRect( 4,
//	     ( dp_x ) + ( ( funcSin( ( 32768 / 360 ) * player[20] ) * player[21] ) >> 16 ),
//	     ( 240 - dp_y - 32 ) + ( ( funcCos( ( 32768 / 360 ) * player[20] ) * player[21] ) >> 16 ) ,
//	     0,
//	     0,
//	     32,
//	     32 );
	/* 角度から表示パターンを取得 */
	arm_muki = ( ( player[20] + 11 ) * 100 ) / ( ( 360 * 100 ) / 16 ) ;
	if ( arm_muki >= 16 )
	{
		arm_muki = 0;
	} 

	for ( i = 1; i < 5; i++ )
	{ 
		if ( ( player[21] == 32 ) && ( i > 1 ) ) 
		{
		}
		else 
		{
			BltRect( 4,
			     ( dp_x ) + ( ( player[30] / 5 ) * i ),
			     ( 240 - ( dp_y + ( ( player[31] / 5 ) * i ) ) ) - 32,
			     arm_muki * 32,
			     96 + ( gameflag[133] * 32 ),
			     32,
			     32 );
		}
	}
//		BltTrans( 65,
//		     ( dp_x ) + ( ( ( ( funcSin( ( 32768 / 360 ) * player[20] ) * player[21] ) >> 16 ) / 5 ) * i ),
//		     ( 240 - dp_y - 32 ) + ( ( ( ( funcCos( ( 32768 / 360 ) * player[20] ) * player[21] ) >> 16 ) / 5 ) * i ) ,
//		     125,
//		     player[20] );

		arm_muki = ( ( player[20] + 11 ) * 100 ) / ( ( 360 * 100 ) / 16 ) ;
		if ( arm_muki >= 16 )
		{
			arm_muki = 0;
		} 
		BltRect( 4,
		     ( dp_x ) + player[30],
		     ( 240 - ( dp_y + player[31] ) ) - 32,
		     arm_muki * 32,
		     32 + ( player[23] * 32 ),
		     32,
		     32 );
/*
		if ( player[23] == 0 )
		{
			BltTrans( 65,
			     ( dp_x ) + ( ( ( ( funcSin( ( 32768 / 360 ) * player[20] ) * player[21] ) >> 16 ) / 5 ) * i ),
			     ( 240 - dp_y - 32 ) + ( ( ( ( funcCos( ( 32768 / 360 ) * player[20] ) * player[21] ) >> 16 ) / 5 ) * i ) ,
			     149,
			     arm_muki );
		}
		else 
		{
			BltTrans( 66,
			     ( dp_x ) + ( ( ( ( funcSin( ( 32768 / 360 ) * player[20] ) * player[21] ) >> 16 ) / 5 ) * i ),
			     ( 240 - dp_y - 32 ) + ( ( ( ( funcCos( ( 32768 / 360 ) * player[20] ) * player[21] ) >> 16 ) / 5 ) * i ) ,
			     149,
			     arm_muki );
		}
*/	
	
	if ( ( player[17] % 2 ) == 0 )
	{
		BltRect( 1, dp_x, 240 - dp_y - 32 , ( player[11] * 32 ) + ( player[4] * 320 ), ( player[5] * 64 ) + ( player[3] * 32 ), 32, 32 );
	}
	
}
//-------------------------------------------------------------
//背景表示
void bakdisp1()
{
	int x;
	int y;
	int b_x;
	int b_y;
	int i;

	x = 0;
	y = 0;
	b_x = 0;
	b_y = 0;
	i = 0;
	
	if ( gameflag[67] == 1 )
	{
		if ( stage == 11 )
		{
			rayer[0] = rayer[0] + 1;
			if ( rayer[0] > 480 )
			{
				rayer[0] = rayer[0] - 480;
			}
			rayer[1] = rayer[1] + 2;
			if ( rayer[1] > 480 )
			{
				rayer[1] = rayer[1] - 480;
			}
			rayer[2] = rayer[2] + 4;
			if ( rayer[2] > 480 )
			{
				rayer[2] = rayer[2] - 480;
			}
		}
		else 
		{
		
			rayer[0] = rayer[0] + 1;
			if ( rayer[0] > 640 )
			{
				rayer[0] = rayer[0] - 640;
			}
			rayer[1] = rayer[1] + 2;
			if ( rayer[1] > 640 )
			{
				rayer[1] = rayer[1] - 640;
			}
			rayer[2] = rayer[2] + 4;
			if ( rayer[2] > 640 )
			{
				rayer[2] = rayer[2] - 640;
			}
		}
	}
	
	//画面背景を画面全体に転送
	bak_cnt = bak_cnt + 1;
	if ( bak_cnt >= 40 )
	{
		bak_cnt = 0;
	}

	if ( player[0] < 160 )
	{
		d_x = 0;
	}
	else if ( player[0] >= 480 )
	{
		d_x = 320;
	}
	else 
	{
		d_x = player[0] - 160;
	}

	if ( player[1] < ( 120 ) )
	{
		d_y = 240;
	}
	else if ( player[1] >= ( 360 - 16 ) )
	{
		d_y = 16;
	}
	else 
	{
		d_y = 360 - player[1];
	}
	/* stage13,14,17は多重スクロール */
/*
	if ( stage == 13 )
	{
		BltFastRect( 110, 0, 0, ( d_x / 8 ), ( d_y / 8 ) - 2, 320, 240 );
		BltRect( 111, 0, 0, ( d_x / 4 ), ( d_y / 4 ) - 4, 320, 240 );
		BltRect( 112, 0, 0, ( d_x / 2 ), ( d_y / 2 ) - 8, 320, 240 );
		BltRect( 125, 0, 0 , d_x , d_y - 16, 320, 240 );
	}
	else 
	{
*/
	/* 背景アニメーションON */
	Blt( 113, 0, 0 );						/* 背景スクロール演出 */
//	Blt( 110, 0 - rayer[0] - d_x, 0 - d_y );				/* 背景スクロール演出 */
//	Blt( 110, 640 - rayer[0] - d_x, 0 - d_y );				/* 背景スクロール演出 */
//	Blt( 111, 0 - rayer[1] - d_x, 0 - d_y );				/* 背景スクロール演出 */
//	Blt( 111, 640 - rayer[1] - d_x, 0 - d_y );				/* 背景スクロール演出 */
//	Blt( 112, 0 - rayer[2] - d_x, 0 - d_y );				/* 背景スクロール演出 */
//	Blt( 112, 640 - rayer[2] - d_x, 0 - d_y );				/* 背景スクロール演出 */
	/* 地面表示 */
	i = bak_cnt / 10;
	for ( x = 0; x < 20; x++ )
	{
		for ( y = 0; y <= 16; y++ )
		{
			if ( ( ( d_x - 32 ) <= ( x * 32 ) ) 
			  && ( ( d_y - 32 ) <= ( y * 32 ) )
			  && ( ( d_x + 320 + 32 ) >= ( x * 32 ) )
			  && ( ( d_y + 240 + 32 ) >= ( y * 32 ) ) )
			{
				b_y = map1[ x + ( y * 20 ) ] / 100;
				b_x = map1[ x + ( y * 20 ) ] % 100;
				if ( ( map1[ x + ( y * 20 ) ] >= 100 ) && ( map1[ x + ( y * 20 ) ] <= 219 ) )
				{
					BltRect( 10, ( x * 32 ) - d_x , ( y * 32 ) - d_y - 32 + 16 , 0 * 32 , 0 * 32 ,32, 32 );
				}
				else 
				{	

					if ( b_y == 7 )			/* 背景アニメーション */
					{
						BltRect( 10, ( x * 32 ) - d_x, ( y * 32 ) - d_y - 32 + 16 , b_x * 32 , ( b_y + i ) * 32 ,32, 32 );
					}
					else 
					{
						BltRect( 10, ( x * 32 )  - d_x , ( y * 32 )  - d_y - 32 + 16 , b_x * 32 , b_y * 32 ,32, 32 );
					}
				}
			}
		}
	}

}
void bakdisp2( )
{
	int i;
	int x;
	int y;
	int b_x;
	int b_y;
	
	i = bak_cnt / 10;
	
	for ( x = 0; x < 20; x++ )
	{
		for ( y = 0; y <= 16; y++ )
		{
			if ( ( ( d_x - 32 ) <= ( x * 32 ) ) 
			  && ( ( d_y - 32 ) <= ( y * 32 ) )
			  && ( ( d_x + 320 + 32 ) >= ( x * 32 ) )
			  && ( ( d_y + 240 + 32 ) >= ( y * 32 ) ) )
			{
				if ( ( map1[ x + ( y * 20 ) ] >= 100 ) && ( map1[ x + ( y * 20 ) ] <= 219 ) )
				{
					b_y = map1[ x + ( y * 20 ) ] / 100;
					b_x = map1[ x + ( y * 20 ) ] % 100;
					
					if ( b_y == 7 )				/* 背景アニメーション */
					{
						BltRect( 10, ( x * 32 ) - d_x , ( y * 32 ) - d_y - 32 + 16, b_x * 32 , ( b_y + i ) * 32 ,32, 32 );
					}
					else 
					{
						BltRect( 10, ( x * 32 ) - d_x , ( y * 32 ) - d_y - 32 + 16 , b_x * 32 , b_y * 32 ,32, 32 );
					}
					
					b_y = map2[ x + ( y * 20 ) ] / 100;
					b_x = map2[ x + ( y * 20 ) ] % 100;
					if ( b_y == 7 )				/* 背景アニメーション */
					{
						BltRect( 11, ( x * 32 ) - d_x , ( y * 32 ) - d_y - 32 + 16 , b_x * 32 , ( b_y + i ) * 32 ,32, 32 );
					}
					else 
					{
						BltRect( 11, ( x * 32 ) - d_x , ( y * 32 ) - d_y - 32 + 16 , b_x * 32 , b_y * 32 ,32, 32 );
					}
				}
				else  
				{
					b_y = map2[ x + ( y * 20 ) ] / 100;
					b_x = map2[ x + ( y * 20 ) ] % 100;
					if ( b_y == 7 )				/* 背景アニメーション */
					{
						BltRect( 11, ( x * 32 ) - d_x , ( y * 32 ) - d_y - 32 + 16 , b_x * 32 , ( b_y + i ) * 32 ,32, 32 );
					}
					else 
					{
	//					BltRect( 11, ( x * 32 ) - d_x , ( y * 32 ) - d_y - 32 + 16 , b_x * 32 , b_y * 32 ,32, 32 );
					}
				}
			}
		}
	}
}

void bak_make( )
{

}



/* ＷＡＩＴつきスクロール */
void wait_scl( long type, long d_num )
{
/*  
	0:上	1:下	2:左	3:右
	よみこむファイルＮＯ
*/
	long i;
	long x;
	long y;

	screen_BackupPlane( );

	/* バッファクリア */
	for ( i = 0; i < 100; i++ )
	{
		p_ef[i * 5] = 0;						/* 存在 */
	}
	for ( i = 0; i < 40; i++ )
	{
		pmshot[i * 20 + 2] = 0;						/* 存在 */
	}
	for ( i = 0; i < 10; i++ )
	{
		pshot[i * 10 + 0] = 0 ;	
	}
	for ( i = 0; i < 40; i++ )
	{
		pshotf[i * 10 + 0] = 0 ;	
	}
	/* バッファクリア */
	for ( i = 0; i < 50; i++ )
	{
		item[i * 10 + 0] = 0 ;	
	}
	for ( i = 0; i < 20; i++ )
	{
		enemy[i * 20 + 0] = 0 ;					/* 存在してない */
	}
	for ( i = 0; i < 20; i++ )
	{
		mgc_e[i * 10 + 0] = 0 ;					/* 存在してない */
	}
	for ( i = 0; i < 20; i++ )
	{
		kane[i * 10 + 0] = 0 ;					/* 存在してない */
	}

	ResetGameFlag2( );
	sprintf(string,"data/%d/%d_%d.map", (int)stage , (int)1, (int)d_num );
	LoadGameFlag2(string);
	{
		for ( i = 0; i <= 319; i++ )
		{
			map1[i] = gameflag2[i];
		}
	}
	for ( i = 0 ; i <= 20 ; i++ )
	{
		map_data[i] = gameflag2[i + 320];
	}
	
	for ( i = 0 ; i < 5 ; i++ )
	{
		sprintf(string,"image/bak/%d.bmp", (int)map_data[11 + i] );
		
		if ( ( i == 0 ) || ( map_data[11 + i] != 0 ) )
		{
			LoadBitmap(string,110 + i , 0 );
		}
		else 
		{
			ReleaseBitmap(110 + i);
		}
	}

	ResetGameFlag2( );
	sprintf(string,"data/%d/%d_%d.map", (int)stage , (int)2, (int)d_num );
	if ( LoadGameFlag2(string) == 0 )
	{
		for ( i = 0; i <= 319; i++ )
		{
			map2[i] = gameflag2[i];
		}
	}
	bak_make();


	ResetGameFlag2( );
	sprintf(string,"data/%d/i%d_%d.map", (int)stage , (int)1, (int)d_num );
	if ( LoadGameFlag2(string) == 0 )
	{
		for ( x = 0; x <= 19 ; x++ )
		{
			for ( y = 0; y <= 16 ; y++ )
			{
				if ( gameflag2[x + ( y * 20 )] != 0 )
				{
					item_set( gameflag2[x + ( y * 20 )], x * 32, ( ( 14 - y ) * 32 ) + 16, x + ( y * 20 ), d_num );
				}
			}
		}
	}

	ResetGameFlag2();
	sprintf( string,"data/%d/e%d_%d.map", (int)stage , (int)1, (int)d_num );
	LoadGameFlag2( string );
	for ( x = 0; x <= 19 ; x++ )
	{
		for ( y = 0; y <= 16 ; y++ )
		{
			if ( gameflag2[x + ( y * 20 )] != 0 )
			{
				enm_set( gameflag2[x + ( y * 20 )], x * 32, ( ( 14 - y ) * 32 ) + 16 );
			}
		}
	}
	
	save_data[3] = d_num;
	
	if ( type == 0 )
	{
		player[1] = 0;
	}
	else if ( type == 1 )
	{
		player[1] = ( 480 - 32 - 16 );
	}
	else if ( type == 2 )
	{
		player[0] = ( 640 - 32 );
	}
	else if ( type == 3 )
	{
		player[0] = 0;
	}

	player[15] = player[0];
	player[16] = player[1];

	player[9] = 0;	
}


void wait_scl2( long d_num ,long init_x, long init_y )
{
	long i;
	long x;
	long y;

	for ( i = 0; i < 100; i++ )
	{
		p_ef[i * 5] = 0;						/* 存在 */
	}
	for ( i = 0; i < 40; i++ )
	{
		pmshot[i * 20 + 2] = 0;						/* 存在 */
	}
	for ( i = 0; i < 10; i++ )
	{
		pshot[i * 10 + 0] = 0 ;	
	}
	for ( i = 0; i < 40; i++ )
	{
		pshotf[i * 10 + 0] = 0 ;	
	}

	for ( i = 0; i < 50; i++ )
	{
		item[i * 10 + 0] = 0 ;	
	}
	for ( i = 0; i < 20; i++ )
	{
		enemy[i * 20 + 0] = 0 ;					/* 存在してない */
	}
	for ( i = 0; i < 20; i++ )
	{
		mgc_e[i * 10 + 0] = 0 ;
	}
	for ( i = 0; i < 20; i++ )
	{
		kane[i * 10 + 0] = 0 ;				
	}

	ResetGameFlag2( );
	sprintf(string,"data/%d/%d_%d.map", (int)stage , (int)1, (int)d_num );
	LoadGameFlag2(string);
	if ( LoadGameFlag2(string) == 0 )
	{
		for ( i = 0; i <= 319; i++ )
		{
			map1[i] = gameflag2[i];
		}
	}
	for ( i = 0 ; i <= 20 ; i++ )
	{
		map_data[i] = gameflag2[i + 320];
	}
	
	for ( i = 0 ; i < 5 ; i++ )
	{
		sprintf(string,"image/bak/%d.bmp", (int)map_data[11 + i] );
		
		if ( ( i == 0 ) || ( map_data[11 + i] != 0 ) )
		{
			LoadBitmap(string,110 + i , 0 );
		}
		else 
		{
			ReleaseBitmap(110 + i);
		}
	}
	
	ResetGameFlag2( );
	sprintf(string,"data/%d/%d_%d.map", (int)stage , (int)2, (int)d_num );
	if ( LoadGameFlag2(string) == 0 )
	{
		for ( i = 0; i <= 319; i++ )
		{
			map2[i] = gameflag2[i];
		}
	}
	bak_make();


	ResetGameFlag2( );
	sprintf(string,"data/%d/i%d_%d.map", (int)stage , (int)1, (int)d_num );
	if ( LoadGameFlag2(string) == 0 )
	{
		for ( x = 0; x <= 19 ; x++ )
		{
			for ( y = 0; y <= 16 ; y++ )
			{
				if ( gameflag2[x + ( y * 20 )] != 0 )
				{
					item_set( gameflag2[x + ( y * 20 )], x * 32, ( ( 14 - y ) * 32 ) + 16, x + ( y * 20 ), d_num );
				}
			}
		}
	}

	ResetGameFlag2();
	sprintf( string,"data/%d/e%d_%d.map", (int)stage , (int)1, (int)d_num );
	LoadGameFlag2( string );

	for ( x = 0; x <= 19 ; x++ )
	{
		for ( y = 0; y <= 16 ; y++ )
		{
			if ( gameflag2[x + ( y * 20 )] != 0 )
			{
				enm_set( gameflag2[x + ( y * 20 )], x * 32, ( ( 14 - y ) * 32 ) + 16 );
			}
		}
	}
	
	save_data[3] = d_num;
	
	player[0] = init_x;
	player[1] = init_y;


	player[15] = player[0];
	player[16] = player[1];
}


void wait_scl3( )
{
	long i;
	long x;
	long y;
	long d_num;
	
	d_num = save_data[3];

	for ( i = 0; i < 100; i++ )
	{
		p_ef[i * 10] = 0;						/* 存在 */
	}
	for ( i = 0; i < 40; i++ )
	{
		pmshot[i * 20 + 2] = 0;						/* 存在 */
	}
	for ( i = 0; i < 10; i++ )
	{
		pshot[i * 10 + 0] = 0 ;	
	}
	for ( i = 0; i < 40; i++ )
	{
		pshotf[i * 10 + 0] = 0 ;	
	}

	for ( i = 0; i < 50; i++ )
	{
		item[i * 10 + 0] = 0 ;	
	}
	for ( i = 0; i < 20; i++ )
	{
		if ( enemy[0 + ( i * 20 )] == 1 )				
		{
			enemy[1 + ( i * 20 )] = enemy[14 + ( i * 20 )] ;
			enemy[2 + ( i * 20 )] = enemy[15 + ( i * 20 )] ;
		}
	}
	for ( i = 0; i < 20; i++ )
	{
		mgc_e[i * 10 + 0] = 0 ;		
	}

	ResetGameFlag2( );
	sprintf(string,"data/%d/%d_%d.map", (int)stage , (int)1, (int)d_num );
	LoadGameFlag2(string);
	{
		for ( i = 0; i <= 319; i++ )
		{
			map1[i] = gameflag2[i];
		}
	}
	for ( i = 0 ; i <= 20 ; i++ )
	{
		map_data[i] = gameflag2[i + 320];
	}
	
	for ( i = 0 ; i < 5 ; i++ )
	{
		sprintf(string,"image/bak/%d.bmp", (int)map_data[11 + i] );
		
		if ( ( i == 0 ) || ( map_data[11 + i] != 0 ) )
		{
			LoadBitmap(string,110 + i , 0 );
		}
		else 
		{
			ReleaseBitmap(110 + i);
		}
	}
	
	ResetGameFlag2( );
	sprintf(string,"data/%d/%d_%d.map", (int)stage , (int)2, (int)d_num );
	if ( LoadGameFlag2(string) == 0 )
	{
		for ( i = 0; i <= 319; i++ )
		{
			map2[i] = gameflag2[i];
		}
	}
	bak_make();

	ResetGameFlag2( );
	sprintf(string,"data/%d/i%d_%d.map", (int)stage , (int)1, (int)d_num );
	if ( LoadGameFlag2(string) == 0 )
	{
		for ( x = 0; x <= 19 ; x++ )
		{
			for ( y = 0; y <= 16 ; y++ )
			{
				if ( gameflag2[x + ( y * 20 )] != 0 )
				{
					item_set( gameflag2[x + ( y * 20 )], x * 32, ( ( 14 - y ) * 32 ) + 16, x + ( y * 20 ), d_num );
				}
			}
		}
	}

	ResetGameFlag2();
	sprintf( string,"data/%d/e%d_%d.map", (int)stage , (int)1, (int)d_num );
	LoadGameFlag2( string );
	
	player[15] = player[0];
	player[16] = player[1];


}

void snack_counter_disp()
{
	long px;
	long py;
	long x;
	long dx;
	long dy;
	long count;
	long i;

	count = 0;
	for(i = 0; i < 20; i++)
	{
		if ( enemy[0 +( i * 20 )] == 1 )
		{
			count++;
		}
	}
	
	px = 10;
	py = 18;
	dx = 10;
	dy = 8;
	x = 0;
	BltRect( 18, 100, 2, 0, 72, 100, 8 );	/* replay */
	
	BltNumericImage2( count, 2, 140, 2, 18, 0, 0, 10, 8 );
}


void minimap_disp( void )
{
	long px;
	long py;
	long size;
	long width;
	long height;
	long i;
	long x;
	long y;

	px   = 2;
	py   = 12;
	size = 3;
	width  = size*20;
	height = size*15;

	if ( gameflag[132] == 0 )
	{
		return;
	}

	Blt( 70, px, py );
	x = px + 0      + ((player[0] / 32 + 0) * size);
	y = py + height - ((player[1] / 32 + 1) * size);
	BltRect( 127, x, y, 127, 191, size, size );

	for( i = 0; i < 20; i++)
	{
		if ( enemy[0 +( i * 20 )] == 1 )
		{
			x = px + 0      + ((enemy[1 +( i * 20)] / 32 + 0) * size);
			y = py + height - ((enemy[2 +( i * 20)] / 32 + 1) * size);
			BltRect( 127, x, y, 127, 127, size, size );
		}

	}
}


void mapdisp()
{
	int stage_hosei;
	
	stage_hosei = 0;
	if ( stage == 2 )
	{
		stage_hosei = 50;
	}

	long wk;

	if ( gameflag[132] == 1 )
	{
		if ( stage != 0 )
		{
			BltRect( 18, 260, 2, 0, 32, 100, 8 );	/* replay */
		}

		if ( replay_load_key[0] == 1 )  /* l */
		{
			if ( replay_load_key[2] == 1 )  /* up */
			{
				BltRect( 67, 10, 150, 256, 0, 64, 64 );	/* l */
			}
			else if ( replay_load_key[3] == 1 )  /* down */
			{
				BltRect( 67, 10, 150, 192, 64, 64, 64 );	/* l */
			}
			else 
			{
				BltRect( 67, 10, 150, 128, 64, 64, 64 );	/* l */
			}
		}
		else if ( replay_load_key[1] == 1 )  /* r */
		{
			if ( replay_load_key[2] == 1 )  /* up */
			{
				BltRect( 67, 10, 150, 192, 0, 64, 64 );	/* r */
			}
			else if ( replay_load_key[3] == 1 )  /* down */
			{
				BltRect( 67, 10, 150, 256, 64, 64, 64 );	/* r */
			}
			else 
			{
				BltRect( 67, 10, 150, 64, 64, 64, 64 );	/* r */
			}
		}
		else 
		{
			if ( replay_load_key[2] == 1 )  /* up */
			{
				BltRect( 67, 10, 150, 64, 0, 64, 64 );	/* r */
			}
			else if ( replay_load_key[3] == 1 )  /* down */
			{
				BltRect( 67, 10, 150, 128, 0, 64, 64 );	/* r */
			}
			else 
			{
				BltRect( 67, 10, 150, 0, 0, 64, 64 );	/* r */
			}
		}
		if ( replay_load_key[4] == 1 )  /* 1 */
		{
			BltRect( 67, 90, 190, 32, 128, 32, 32 );	/* off */
		}
		else 
		{
			BltRect( 67, 90, 190, 0, 128, 32, 32 );	/* on */
		}
		if ( replay_load_key[5] == 1 )  /* 2 */
		{
			BltRect( 67, 130, 190, 32, 128, 32, 32 );	/* off */
		}
		else 
		{
			BltRect( 67, 130, 190, 0, 128, 32, 32 );	/* on */
		}
	}

	if ( stage != 0 )
	{	
		if ( gameflag[127] == 1 )	//トータルアタック
		{
			BltRect( 18, 5, 220, 0, 128, 100, 8 );	/* rank */
		}
		else if ( stage == 2 )
		{
			BltRect( 18, 5, 220, 0, 112, 100, 8 );	/* rank */
		}
		else 
		{
			if ( ( save_data[3] >= 1 ) && ( save_data[3] <= 15 ) )
			{
				BltRect( 18, 5, 220, 0, 88, 100, 8 );	/* rank */
			}
			if ( ( save_data[3] >= 16 ) && ( save_data[3] <= 35 ) )
			{
				BltRect( 18, 5, 220, 0, 96, 100, 8 );	/* rank */
			}
			if ( ( save_data[3] >= 36 ) && ( save_data[3] <= 50 ) )
			{
				BltRect( 18, 5, 220, 0, 104, 100, 8 );	/* rank */
			}
		}

		BltRect( 18, 10, 2, 0, 40, 100, 8 );	/*  */
		BltNumericImage2( save_data[3], 2, 60, 2, 18, 0, 0, 10, 8 );	/* d_num */

		if ( gameflag[127] == 1 )	
		{
			BltRect( 18, 180, 2, 0, 120, 100, 8 );	/*  */
			BltNumericImage2( gameflag[136], 3, 220, 2, 18, 0, 0, 10, 8 );	/* miss count */
		}
		else 
		{
			BltRect( 18, 180, 2, 0, 80, 100, 8 );	/*  */
			BltNumericImage2( Jump_counts, 2, 230, 2, 18, 0, 0, 10, 8 );	/* Jump_counts */
		}
		
		BltRect( 18, 50, 230, 0, 8, 100, 8 );	/*  */
		BltRect( 18, 5, 230, 0, 16, 100, 8 );	/*  */
		
		BltRect( 18, 200, 230 , 0, 8, 100, 8 );	/*  */
		BltRect( 18, 155, 230 , 0, 24, 100, 8 );	/*  */

		BltNumericImage( play_time[0], 2, 110, 230, 18, 0, 0, 10, 8 );	/* 時間 */
		BltNumericImage( play_time[1], 2, 80, 230, 18, 0, 0, 10, 8 );	/* 時間 */
		BltNumericImage( play_time[2], 2, 50, 230, 18, 0, 0, 10, 8 );	/* 時間 */

		if ( gameflag[127] == 1 )	
		{
			wk = get2keta( gameflag[137] , 1 );
			BltNumericImage( wk, 2, 260, 230, 18, 0, 0, 10, 8 );	
			wk = get2keta( gameflag[137] , 100 );
			BltNumericImage( wk, 2, 230, 230, 18, 0, 0, 10, 8 );	
			wk = get2keta( gameflag[137] , 10000 );
			BltNumericImage( wk, 2, 200, 230, 18, 0, 0, 10, 8 );	
		}
		else 
		{
			wk = get2keta( gameflag[200 + save_data[3] + stage_hosei] , 1 );
			BltNumericImage( wk, 2, 260, 230, 18, 0, 0, 10, 8 );	
			wk = get2keta( gameflag[200 + save_data[3] + stage_hosei] , 100 );
			BltNumericImage( wk, 2, 230, 230, 18, 0, 0, 10, 8 );	
			wk = get2keta( gameflag[200 + save_data[3] + stage_hosei] , 10000 );
			BltNumericImage( wk, 2, 200, 230, 18, 0, 0, 10, 8 );	
		}
	}
	else 
	{
		BltRect( 18, 180, 2, 0, 80, 100, 8 );	/*  */
		BltNumericImage2( Jump_counts, 3, 210, 2, 18, 0, 0, 10, 8 );	/* Jump_counts */
	}

}


void stage_start( )
{
	int x = 0;
	int y = 0;
	int rec = 0;
	int rec2 = 0;

	KeyInput( );
	demo[0]++;
	rec = 32 - ( demo[0] * 2 );
	if ( rec < 0 )
	{
		rec = 0;
	}
	rec2 = demo[0];
	if ( rec2 > 16 )
	{
		rec2 = 16;
	}
	
	for ( x = 0; x < 10; x++ )
	{
		for ( y = 0; y < 8; y++ )
		{
			BltRect( 72, ( x * 32 ) + rec2, ( y * 32 ) + rec2, 0, 0, rec, rec );
		}
	}

	if ( demo[0] >= 16 )
	{
		sn = 1;
	}

}

void stage_endj( )
{
	if ( player[8] <= 0 )
	{
		player[2] = 32;
		player[10] = 0;
		player[11] = 0;
		sn = 3;
		demo[0] = 0;
	}
}

void stage_claer( )
{
	int stage_hosei;
	int rec = 0;
	int rec2 = 0;
	int fead = 0;
	int x = 0;
	int y = 0;
	
	stage_hosei = 0;
	if ( stage == 2 )
	{
		stage_hosei = 50;
	}
	
	demo[0]++;
	
	play_time[4] = 1000000 + ( play_time[2] * 10000 ) + ( play_time[1] * 100 ) + ( play_time[0] );

	if ( gameflag[127] == 0 )
	{
		if ( ( gameflag[300 + save_data[3] + stage_hosei] > Jump_counts ) )
		{
			if ( gameflag[132] == 0 )
			{
				Blt( 69, 0, 0 );
			}
		}
		if ( ( gameflag[200 + save_data[3] + stage_hosei] > play_time[4] ) )
		{
			if ( gameflag[132] == 0 )
			{
				Blt( 68, 0, 0 );
			}
		}
	}

	fead = demo[0] - 140;
	if ( fead < 0 )
	{
		fead = 0;
	}
	rec = ( fead * 2 );
	if ( rec > 32 )
	{
		rec = 32;
	}
	rec2 = 16 - fead;
	if ( rec2 < 0 )
	{
		rec2 = 0;
	}
	
	for ( x = 0; x < 10; x++ )
	{
		for ( y = 0; y < 8; y++ )
		{
			BltRect( 72, ( x * 32 ) + rec2, ( y * 32 ) + rec2, 0, 0, rec, rec );
		}
	}

	if ( demo[0] > 180 )
	{
		if ( gameflag[132] == 0 )
		{
			if ( gameflag[127] == 1 )	//トータルアタック
			{
				gameflag[135] = play_time[4];
			}
			else 
			{
				replay_file_save_w( );
				if ( gameflag[200 + save_data[3] + stage_hosei] > play_time[4] )
				{
					gameflag[200 + save_data[3] + stage_hosei] = play_time[4];	/* スコア保存 */
				
					replay_file_save( 0 );
				}

				if ( gameflag[300 + save_data[3] + stage_hosei] > Jump_counts )
				{
					gameflag[300 + save_data[3] + stage_hosei] = Jump_counts;	/* スコア保存 */
				
					replay_file_save( 1 );
				}
			}
				
				save_data[3]++;
				if ( gameflag[121] < save_data[3] )
				{
					gameflag[121] = save_data[3];
				}
		}
		else 
		{
			replay_file_save_w( );
			save_data[3]++;
			if ( ( gameflag[124] == 1 ) || ( stage == 0 ) )	/* リプレイＯＮＥ */
			{
				gameflag[40] = 1;
				g_scene = EN_SN_TITLE;
			}
		}
		sn = 10;
	}

		if ( player[2] != 33 )
		{
			player[10] = 0 ;
			player[11] = 0 ;
			player[2] = 33 ;
		}
	
}

void stage_end( )
{
	int rec = 0;
	int rec2 = 0;
	int fead = 0;
	int x = 0;
	int y = 0;

	demo[0]++;

	fead = demo[0] - 140;
	if ( fead < 0 )
	{
		fead = 0;
	}
	rec = ( fead * 2 );
	if ( rec > 32 )
	{
		rec = 32;
	}
	rec2 = 16 - fead;
	if ( rec2 < 0 )
	{
		rec2 = 0;
	}
	
	for ( x = 0; x < 10; x++ )
	{
		for ( y = 0; y < 8; y++ )
		{
			BltRect( 72, ( x * 32 ) + rec2, ( y * 32 ) + rec2, 0, 0, rec, rec );
		}
	}

	if ( demo[0] > 180 )
	{
		if ( player[8] <= 0 )		/* ライフ０ */
		{
			gameflag[136]++;
			if ( gameflag[136] > 999 )	/* ミスカウント */
			{
				gameflag[136] = 999;
			}
		}
		if ( gameflag[127] == 1 )	//トータルアタック
		{
			play_time[4] = 1000000 + ( play_time[2] * 10000 ) + ( play_time[1] * 100 ) + ( play_time[0] );
			gameflag[135] = play_time[4];
		}
		gameflag[71] = 1;
		sn = 10;
	}
}


void p_shot_set( long type , long muki, long houkou )
{
	long i;
		
	for ( i = 0; i < 10; i++ )
	{
		if ( pshot[( i * 10 ) + 0 ] == 0 )
		{
			pshot[( i * 10 ) + 0 ] = 1;
			pshot[( i * 10 ) + 1 ] = player[0];
			pshot[( i * 10 ) + 2 ] = player[1];
			pshot[( i * 10 ) + 3 ] = houkou;
			pshot[( i * 10 ) + 4 ] = muki;
			pshot[( i * 10 ) + 5 ] = 0;
			pshot[( i * 10 ) + 6 ] = 0;
			pshot[( i * 10 ) + 7 ] = type;
			pshot[( i * 10 ) + 8 ] = 0;		
			pshot[( i * 10 ) + 9 ] = 0;
			
			break;
		}
	}
}

void p_shot_disp( )
{
	long i;
	
	for ( i = 0; i < 10; i++ )
	{
		if ( pshot[( i * 10 ) + 0 ] == 1 )
		{
			pshot[( i * 10 ) + 1 ] = player[0];
			pshot[( i * 10 ) + 2 ] = player[1];
			
			if ( pshot[( i * 10 ) + 3 ] == 0 )
			{
				if ( pshot[( i * 10 ) + 4 ] == 0 )
				{
					pshot[( i * 10 ) + 1 ] = player[0] - 16;
					pshot[( i * 10 ) + 2 ] = player[1] + 16;
					pshot[( i * 10 ) + 8 ] = 0;	
					pshot[( i * 10 ) + 9 ] = 0;	
				}
				if ( pshot[( i * 10 ) + 4 ] == 1 )
				{
					pshot[( i * 10 ) + 1 ] = player[0] + 5;
					pshot[( i * 10 ) + 2 ] = player[1] + 10;
					pshot[( i * 10 ) + 8 ] = 0;		/* あたり判定座標X */
					pshot[( i * 10 ) + 9 ] = 0;		/* あたり判定座標X */
				}
				if ( pshot[( i * 10 ) + 4 ] == 2 )
				{
					pshot[( i * 10 ) + 1 ] = player[0] + 5;
					pshot[( i * 10 ) + 2 ] = player[1] - 16;
					pshot[( i * 10 ) + 8 ] = 0;		/* あたり判定座標X */
					pshot[( i * 10 ) + 9 ] = 0;		/* あたり判定座標X */
				}
				if ( pshot[( i * 10 ) + 4 ] == 3 )
				{
					pshot[( i * 10 ) + 1 ] = player[0] + 5;
					pshot[( i * 10 ) + 2 ] = player[1] - 43;
					pshot[( i * 10 ) + 8 ] = 0;		/* あたり判定座標X */
					pshot[( i * 10 ) + 9 ] = 0;		/* あたり判定座標X */
				}
				if ( pshot[( i * 10 ) + 4 ] == 4 )
				{
					pshot[( i * 10 ) + 1 ] = player[0] - 16;
					pshot[( i * 10 ) + 2 ] = player[1] - 43;
					pshot[( i * 10 ) + 8 ] = 0;		/* あたり判定座標X */
					pshot[( i * 10 ) + 9 ] = 0;		/* あたり判定座標X */
				}
			}
			else if ( pshot[( i * 10 ) + 3 ] == 1 )
			{
				if ( pshot[( i * 10 ) + 4 ] == 0 )
				{
					pshot[( i * 10 ) + 1 ] = player[0] - 16;
					pshot[( i * 10 ) + 2 ] = player[1] + 16;
				}
				if ( pshot[( i * 10 ) + 4 ] == 1 )
				{
					pshot[( i * 10 ) + 1 ] = player[0] - 37;
					pshot[( i * 10 ) + 2 ] = player[1] + 10;
				}
				if ( pshot[( i * 10 ) + 4 ] == 2 )
				{
					pshot[( i * 10 ) + 1 ] = player[0] - 37;
					pshot[( i * 10 ) + 2 ] = player[1] - 16;
				}
				if ( pshot[( i * 10 ) + 4 ] == 3 )
				{
					pshot[( i * 10 ) + 1 ] = player[0] - 37;
					pshot[( i * 10 ) + 2 ] = player[1] - 43;
				}
				if ( pshot[( i * 10 ) + 4 ] == 4 )
				{
					pshot[( i * 10 ) + 1 ] = player[0] - 16;
					pshot[( i * 10 ) + 2 ] = player[1] - 43;
				}
			}


			BltRect( 3, pshot[( i * 10 ) + 1 ] - d_x, 480 - pshot[( i * 10 ) + 2 ] - 64 - d_y , pshot[( i * 10 ) + 6 ] * 64, pshot[( i * 10 ) + 3 ] * 256, 64, 64 );
			
			pshot[( i * 10 ) + 5 ]++;
			if ( pshot[( i * 10 ) + 5 ] >= 4 )
			{
				pshot[( i * 10 ) + 5 ] = 0;
				pshot[( i * 10 ) + 6 ]++;
				if ( pshot[( i * 10 ) + 6 ] >= 4 )
				{
					pshot[( i * 10 ) + 6 ] = 0;
					pshot[( i * 10 ) + 0 ] = 0;
				}
			}
		}
	}
}

void p_shot_f_set( long x, long y , long type )
{
	long i;
	
	for ( i = 0; i < 40; i++ )
	{
		if ( pshotf[( i * 10 ) + 0 ] == 0 )
		{
			pshotf[( i * 10 ) + 0 ] = 1;
			pshotf[( i * 10 ) + 1 ] = x;
			pshotf[( i * 10 ) + 2 ] = y;
			pshotf[( i * 10 ) + 5 ] = 0;
			pshotf[( i * 10 ) + 6 ] = 0;
			pshotf[( i * 10 ) + 7 ] = type;
			break;
		}
	}
}
void p_shot_f_disp( )
{
	long i;
	
	for ( i = 0; i < 40; i++ )
	{
		if ( pshotf[( i * 10 ) + 0 ] == 1 )
		{
			BltRect( 5, pshotf[( i * 10 ) + 1 ] - d_x, 480 - pshotf[( i * 10 ) + 2 ] - 32 - d_y , pshotf[( i * 10 ) + 6 ] * 32, pshotf[( i * 10 ) + 7 ] * 32, 32, 32 );

			pshotf[( i * 10 ) + 5 ]++;
			if ( pshotf[( i * 10 ) + 5 ] >= 5 )
			{
				pshotf[( i * 10 ) + 5 ] = 0;
				pshotf[( i * 10 ) + 6 ]++;
				if ( pshotf[( i * 10 ) + 6 ] >= 4 )
				{
					pshotf[( i * 10 ) + 0 ] = 0;
					pshotf[( i * 10 ) + 6 ] = 0;
				}
			}
		}
	}
}
void p_shot_f2_set( long x, long y , long muki )
{
	long i;
	long k; 
	long ii;
	long rg;
	long max;
	

	max = 10;
	
	for ( i = 0; i < max; i++ )
	{
		rg = ( ( rand( ) % 360 ) );
		
		k = ( rand( ) % 5 ) + 1;

		for ( ii = 0; ii < 80; ii++ )
		{
			if ( pshotf2[ii * 10 + 2] == 0 )
			{
				pshotf2[ii * 10 + 0] = funcSin( rg ) * k;		/* 弾速x */
				pshotf2[ii * 10 + 1] = funcCos( rg ) * k;		/* 弾速y */
				pshotf2[ii * 10 + 2] = 1;				/* 存在 */									
				pshotf2[ii * 10 + 3] = 1;				/* 方向 */
				pshotf2[ii * 10 + 4] = 0 ;			/* 消えるまでのフレーム数 */
				pshotf2[ii * 10 + 5] = ( x ) << 16 ;	/* 初期位置の設定 */
				pshotf2[ii * 10 + 6] = ( y ) << 16 ;
				pshotf2[ii * 10 + 7] = 0 ;			/* 消えるまでのフレーム数 */
				pshotf2[ii * 10 + 8] = 2 ; 			/* 消えるまでのフレーム数 */
				break;
			}
		}
	}
}
void p_shot_f2_disp( )
{
	long ii;

	for ( ii = 0; ii < 80; ii++ )
	{
		if ( pshotf2[ii * 10 + 2] == 1 )
		{
			BltRect( 5, ( pshotf2[ii * 10 + 5] >> 16 ) - d_x, 480 - ( pshotf2[ii * 10 + 6] >> 16 ) - 4 - d_y, pshotf2[ii * 10 + 4] * 4, 32, 4, 4 );
			pshotf2[ii * 10 + 5] = pshotf2[ii * 10 + 0] + pshotf2[ii * 10 + 5];		/* 弾速x */
			pshotf2[ii * 10 + 6] = pshotf2[ii * 10 + 1] + pshotf2[ii * 10 + 6];		/* 弾速x */
			pshotf2[ii * 10 + 3] = 1;				/* 方向 */
			pshotf2[ii * 10 + 7]++ ;			/* 消えるまでのフレーム数 */
			if ( pshotf2[ii * 10 + 7] >= pshotf2[ii * 10 + 8] )
			{
				pshotf2[ii * 10 + 7] = 0;
				pshotf2[ii * 10 + 4] = pshotf2[ii * 10 + 4] + 1 ;			/* 消えるまでのフレーム数 */
				if ( pshotf2[ii * 10 + 4] >= 10 )
				{
					pshotf2[ii * 10 + 2] = 0;				/* 存在無し */									
				}
			}
		}
	}

}


void item_set( long type, long x, long y, long iti, long g_num )
{
	long i;
	
	if ( iti != 0 )
	{
		if ( 0 == item_sonzai( iti, g_num ) )
		{
			return;
		} 
	}	
	if ( iti < 0 )
	{
		return;
	}
		
	for ( i = 0; i < 50; i++ )
	{
		if ( item[( i * 10 ) + 0 ] == 0 )
		{
			item[( i * 10 ) + 0 ] = 1;
			item[( i * 10 ) + 1 ] = x;
			item[( i * 10 ) + 2 ] = y;
			item[( i * 10 ) + 3 ] = type;
			item[( i * 10 ) + 4 ] = iti;
			item[( i * 10 ) + 5 ] = g_num;
			item[( i * 10 ) + 6 ] = stage;	/* ステージ数 */
			item[( i * 10 ) + 7 ] = 0;	/* イベント無効フラグ（1:表示のみ） */
			
			/* ワープのグラフィック変更 */
			if ( ( save_data[30] == 1 ) && ( type == 5 ) )	/* ワープ可能 */
			{
				item[( i * 10 ) + 3 ] = 6;					/* ワープ可能アイテムに変更 */
			}
			
			/* 620までは配置時イベントを行う */
			if ( ( type >= 100 ) && ( type <= 620 ) )
			{
//				init_event_set( item[( i * 10 ) + 3 ] , i );	/* 配置条件 */
			}
			break;
		}
	}
}
long item_sonzai( long map1, long map2 )
{
	long i;
	long rc;

	rc = 1;
	for ( i = 0; i < 100; i++ )
	{
		if ( item_wk[0 + ( i * 5 )] == 1 )
		{
			if ( ( item_wk[1 + ( i * 5 )] == map1 )
			  && ( item_wk[2 + ( i * 5 )] == map2 )
			  && ( item_wk[3 + ( i * 5 )] == stage ) )
			{
				rc = 0;
				break;
			}
		}
	}
	return( rc );

}


void item_disp( )
{
	long i;
	long k;
	long bx;
	long by;
	long k_f;
	long wbx;
	long disp_x_j;
	long disp_y_j;
	
	wbx = 0;
	k_f = 0;
	disp_x_j = 0;
	disp_y_j = 0;
	
	for ( i = 0; i < 50; i++ )
	{
		if ( item[( i * 10 ) + 0 ] == 1 )
		{
			bx = item[3 + ( i * 10 )] % 100 ;
			by = item[3 + ( i * 10 )] / 100 ;
			
			disp_x_j = item[1 + ( i * item_data[2] )];
			disp_y_j = ( 0 - item[2 + ( i * item_data[2] )] ) + MapInfo[3];
			if ( ( ( d_x - item_data[0] ) <= ( disp_x_j ) ) 
			  && ( ( d_y - item_data[0] ) <= ( disp_y_j  ) )
			  && ( ( d_x + 320 + item_data[0] ) >= ( disp_x_j ) )
			  && ( ( d_y + 240 + item_data[0] ) >= ( disp_y_j ) ) )
			{

				BltRect( 12 , item[1 + ( i * 10 )] - d_x, ( 0 - item[2 + ( i * 10 )] ) + 480 - 32 - d_y, 32 * bx, by * 32, 32, 32 );
			}
			/* プレイヤーとの当たり判定 */
			if ( ( ( (item[1 + ( i * 10 )] + 2 ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
				&& ( (player[0] + 10 ) < (item[1 + ( i * 10 )] + 30 ) )
				&& ( (item[2 + ( i * 10 )]  + 2 ) < (player[1] + 22 ) )
				&& ( (player[1] + 5 ) < ( item[2 + ( i * 10 )] + 30 ) ) )
				&& ( sn == 1 ) )
			{
				if ( player[14] == 10 )
				{
					if ( ( item[( i * 10 ) + 3 ] >= 300 ) && ( item[( i * 10 ) + 3 ] <= 420 ) ) /* 重なって発生するイベント */
					{
						if ( item[( i * 10 ) + 7 ] == 0 )
						{
							k_f = 1;
							if ( one_event == 0 )
							{
								one_event = 1;
//								event_set( item[( i * 10 ) + 3 ] , i );
							}
						}
					}
				}
				if ( 
				     ( item[( i * 10 ) + 3 ] == 1 ) /* ダメージ床*/
				  || ( item[( i * 10 ) + 3 ] == 2 ) /* ダメージ床 */
				  || ( item[( i * 10 ) + 3 ] == 3 ) /* ダメージ床 */
				  || ( item[( i * 10 ) + 3 ] == 4 ) /* ダメージ床 */
				  || ( item[( i * 10 ) + 3 ] == 703 ) /* ダメージ床 */
				  || ( item[( i * 10 ) + 3 ] == 704 ) /* ダメージ床 */
				   )
				{
					player[8] = player[8] - 1;
					soundPlaySe( EN_SE_DAMEGE );
				}
			}

			if ( sn == 1 )
			{
				if ( item[( i * 10 ) + 3 ] == 701 ) 	/* 移動床、縦 */
				{
					if ( item[8 + ( i * 10 )] == 0 )
					{
						if ( item[2 + ( i * 10 )] > 400 )
						{
							item[8 + ( i * 10 )] = 1;
						}
						else 
						{
							if ( ( map1_item_j( item[1 + ( i * 10 )] + 16 , item[2 + ( i * 10 )] + 63 ) >= 100 )
							  && ( map1_item_j( item[1 + ( i * 10 )] + 16 , item[2 + ( i * 10 )] + 63 ) <= 219 ) )
							{
								item[8 + ( i * 10 )] = 1;
							}
							else 
							{
								item[2 + ( i * 10 )]++;
							}
						}
					}
					else 
					{
						if ( item[2 + ( i * 10 )] < 16 )
						{
							item[8 + ( i * 10 )] = 0;
						}
						else 
						{
							if ( ( map1_item_j( item[1 + ( i * 10 )] + 16 , item[2 + ( i * 10 )] - 0 ) >= 100 )
							  && ( map1_item_j( item[1 + ( i * 10 )] + 16 , item[2 + ( i * 10 )] - 0 ) <= 219 ) )
							{
								item[8 + ( i * 10 )] = 0;
							}
							else 
							{
								item[2 + ( i * 10 )]--;
							}
						}
					}
				}
				if ( item[( i * 10 ) + 3 ] == 702 ) 	/* 移動床、横 */
				{
					if ( item[8 + ( i * 10 )] == 0 )
					{
						if ( item[1 + ( i * 10 )] > 602 )
						{
							item[8 + ( i * 10 )] = 1;
						}
						else 
						{
							if ( ( map1_item_j( item[1 + ( i * 10 )] + 32 , item[2 + ( i * 10 )] + 16 ) >= 100 )
							  && ( map1_item_j( item[1 + ( i * 10 )] + 32 , item[2 + ( i * 10 )] + 16 ) <= 219 ) )
							{
								item[8 + ( i * 10 )] = 1;
							}
							else 
							{
								item[1 + ( i * 10 )]++;
								if ( ( ( (item[1 + ( i * 10 )]  ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
									&& ( (player[0] + 10 ) < (item[1 + ( i * 10 )] + 32 ) )
									&& ( (item[2 + ( i * 10 )] + 27 ) < (player[1] + 10 ) )
									&& ( (player[1] - 1 ) < ( item[2 + ( i * 10 )] + 32 ) ) ) )
								{
									player[0]++;
									idou_r( 1 );
								}
							}
						}
					}
					else 
					{
						if ( item[1 + ( i * 10 )] < 0 )
						{
							item[8 + ( i * 10 )] = 0;
						}
						else 
						{
							if ( ( map1_item_j( item[1 + ( i * 10 )] , item[2 + ( i * 10 )] + 16 ) >= 100 )
							  && ( map1_item_j( item[1 + ( i * 10 )] , item[2 + ( i * 10 )] + 16 ) <= 219 ) )
							{
								item[8 + ( i * 10 )] = 0;
							}
							else 
							{
								item[1 + ( i * 10 )]--;
								if ( ( ( (item[1 + ( i * 10 )]  ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
									&& ( (player[0] + 10 ) < (item[1 + ( i * 10 )] + 32 ) )
									&& ( (item[2 + ( i * 10 )] + 27 ) < (player[1] + 10 ) )
									&& ( (player[1] - 1 ) < ( item[2 + ( i * 10 )] + 32 ) ) ) )
								{
									player[0]--;
									idou_l( 1 );
								}
							}
						}
					}
				}
				if ( item[( i * 10 ) + 3 ] == 703 ) 	/* 移動床、縦 */
				{
					if ( item[8 + ( i * 10 )] == 0 )
					{
						if ( item[2 + ( i * 10 )] > 432 )
						{
							item[8 + ( i * 10 )] = 1;
						}
						else 
						{
							if ( ( map1_item_j( item[1 + ( i * 10 )] + 16 , item[2 + ( i * 10 )] + 31 ) >= 100 )
							  && ( map1_item_j( item[1 + ( i * 10 )] + 16 , item[2 + ( i * 10 )] + 31 ) <= 219 ) )
							{
								item[8 + ( i * 10 )] = 1;
							}
							else 
							{
								item[2 + ( i * 10 )]++;
								if ( player[37] == i )
								{
									player[33]++;
									idou_l( 1 );
									idou_r( 1 );
								}
							}
						}
					}
					else 
					{
						if ( item[2 + ( i * 10 )] < 16 )
						{
							item[8 + ( i * 10 )] = 0;
						}
						else 
						{
							if ( ( map1_item_j( item[1 + ( i * 10 )] + 16 , item[2 + ( i * 10 )] - 1 ) >= 100 )
							  && ( map1_item_j( item[1 + ( i * 10 )] + 16 , item[2 + ( i * 10 )] - 1 ) <= 219 ) )
							{
								item[8 + ( i * 10 )] = 0;
							}
							else 
							{
								item[2 + ( i * 10 )]--;
								if ( player[37] == i )
								{
									player[33]--;
									idou_l( 1 );
									idou_r( 1 );
								}
							}
						}
					}
				}
				if ( item[( i * 10 ) + 3 ] == 704 ) 	/* 移動床、横 */
				{
					if ( item[8 + ( i * 10 )] == 0 )
					{
						if ( item[1 + ( i * 10 )] > 602 )
						{
							item[8 + ( i * 10 )] = 1;
						}
						else 
						{
							if ( ( map1_item_j( item[1 + ( i * 10 )] + 32 , item[2 + ( i * 10 )] + 16 ) >= 100 )
							  && ( map1_item_j( item[1 + ( i * 10 )] + 32 , item[2 + ( i * 10 )] + 16 ) <= 219 ) )
							{
								item[8 + ( i * 10 )] = 1;
							}
							else 
							{
								item[1 + ( i * 10 )]++;
								if ( player[37] == i )
								{
									player[32]++;
								}
							}
						}
					}
					else 
					{
						if ( item[1 + ( i * 10 )] < 0 )
						{
							item[8 + ( i * 10 )] = 0;
						}
						else 
						{
							if ( ( map1_item_j( item[1 + ( i * 10 )] , item[2 + ( i * 10 )] + 16 ) >= 100 )
							  && ( map1_item_j( item[1 + ( i * 10 )] , item[2 + ( i * 10 )] + 16 ) <= 219 ) )
							{
								item[8 + ( i * 10 )] = 0;
							}
							else 
							{
								item[1 + ( i * 10 )]--;
								if ( player[37] == i )
								{
									player[32]--;
								}
							}
						}
					}
				}

				
				for ( k = 0; k < 40 ; k++ )
				{
					/* プレイヤーの弾との当たり判定 */
					if ( ( ( ( item[1 + i * 10] ) < ( ( pmshot[5 + k * 20] >> 16 ) + 32 ) )
						&& ( ( ( pmshot[5 + k * 20] >> 16 ) ) < ( item[1 + i * 10] + 32 ) )
						&& ( ( item[2 + i * 10] ) < ( ( pmshot[6 + k * 20] >> 16 ) + 32 ) )
						&& ( ( ( pmshot[6 + k * 20] >> 16 ) ) < ( item[2 + i * 10] + 32 ) ) ) 
						&& ( pmshot[2 + k * 20] == 1 )
						&& ( sn == 1 ) )
					{
	//					if ( item[( i * 10 ) + 3 ] == 1 ) /* イベント */
	//					{
	//					}
					}
				}
			}
		}
	}
	if ( ( k_f == 0 ) && ( sn == 1 ) )
	{
		one_event = 0;
	}
}

long item_up( )
{
	long i;
	long bx;
	long by;
	
	for ( i = 0; i < 50; i++ )
	{
		if ( item[( i * 10 ) + 0 ] == 1 )
		{
			bx = item[3 + ( i * 10 )] % 100 ;
			by = item[3 + ( i * 10 )] / 100 ;
			
			if ( ( by == 5 ) 
			  || ( by == 6 ) /* 台 */
			  || ( ( by == 0 ) && ( bx == 7 ) ) )
			{
				if ( ( ( (item[1 + ( i * 10 )]  ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
					&& ( (player[0] + 10 ) < (item[1 + ( i * 10 )] + 32 ) )
					&& ( (item[2 + ( i * 10 )] + 27 ) < (player[1] + 10 ) )
					&& ( (player[1] - 1 ) < ( item[2 + ( i * 10 )] + 32 ) ) ) )
				{
					return( item[2 + ( i * 10 )] + 32 );
				}
			}
			if ( ( by == 7 ) && ( bx == 1 ) )	/* 	移動床、縦 */
			{
				if ( ( ( (item[1 + ( i * 10 )]  ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
					&& ( (player[0] + 10 ) < (item[1 + ( i * 10 )] + 32 ) )
					&& ( (item[2 + ( i * 10 )] + 27 ) < (player[1] + 10 ) )
					&& ( (player[1] - 1 ) < ( item[2 + ( i * 10 )] + 32 ) ) ) )
				{
					return( item[2 + ( i * 10 )] + 31 );
				}
			}
			if ( ( by == 7 ) && ( bx == 2 ) )	/* 移動床、横 */
			{
				if ( ( ( (item[1 + ( i * 10 )]  ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
					&& ( (player[0] + 10 ) < (item[1 + ( i * 10 )] + 32 ) )
					&& ( (item[2 + ( i * 10 )] + 27 ) < (player[1] + 10 ) )
					&& ( (player[1] - 1 ) < ( item[2 + ( i * 10 )] + 32 ) ) ) )
				{
					return( item[2 + ( i * 10 )] + 31 );
				}
			}
		}
	}
	return( -1 );
}
long item_up2( )
{
	long i;
	long bx;
	long by;
	
	for ( i = 0; i < 50; i++ )
	{
		if ( item[( i * 10 ) + 0 ] == 1 )
		{
			bx = item[3 + ( i * 10 )] % 100 ;
			by = item[3 + ( i * 10 )] / 100 ;
			
			if ( ( by == 5 ) 
			  || ( by == 6 ) /* 台 */
			  || ( ( by == 0 ) && ( bx == 7 ) ) )
			{
				if ( ( ( (item[1 + ( i * 10 )]  ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
					&& ( (player[0] + 10 ) < (item[1 + ( i * 10 )] + 32 ) )
					&& ( (item[2 + ( i * 10 )] + 0 ) < (player[1] + 33 ) )
					&& ( (player[1] + 20 ) < ( item[2 + ( i * 10 )] + 32 ) ) ) )
				{
					return( item[2 + ( i * 10 )] - 32 );
				}
			}
		}
	}
	return( -1 );
}

/* アイテムに重なって上が押された時のイベント */
long item_event( )
{
	long i;
	long rc;
	long r_f;
	
	rc = 0;
	
	for ( i = 0; i < 50; i++ )
	{
		if ( item[( i * 10 ) + 0 ] == 1 )
		{
			r_f = 0;
			
			/* プレイヤーとの当たり判定 */
			if ( ( ( (item[1 + ( i * 10 )]  ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
				&& ( (player[0] + 10 ) < (item[1 + ( i * 10 )] + 32 ) )
				&& ( (item[2 + ( i * 10 )] ) < (player[1] + 32 ) )
				&& ( (player[1] + 5 ) < ( item[2 + ( i * 10 )] + 32 ) ) )
				&& ( player[14] == 10 ) )
			{
				r_f = 1;
				player[18] = 0;	/* 着地モーションとほったらかしアニメーションの解除 */

			}

			if ( ( r_f == 1 ) && ( item[( i * 10 ) + 7 ] == 0 ) )
			{
				if ( ( item[( i * 10 ) + 3 ] >= 100 ) && ( item[( i * 10 ) + 3 ] <= 220 ) ) /* 看板 */
				{
					rc = 1;
//					event_set( item[( i * 10 ) + 3 ] , i );
				}
				break;
			}
			
		}
	}
	return( rc );
}


//敵作成
void enm_set( long type, long x, long y )
{	
	long i;
	long size;
	long syu;
	long j;

	for ( i = 0; i < 20; i++ )
	{
		if ( enemy[0 + ( i * 20 )] == 0 )
		{
			for ( j = 0; j < 20 ; j++ )
			{
				enemy[j + ( i * 20 )] = 0;
			}
			
			
			enemy[0 + ( i * 20 )] = 1;
			enemy[1 + ( i * 20 )] = x;
			enemy[2 + ( i * 20 )] = y;
			size = type / 100;
			syu = type % 100;
			enemy[3 + ( i * 20 )] = size;
			if ( size >= 4 )	/* ボス */
			{
				sprintf(string,"image/enm/boss%d.bmp", (int)syu );
				LoadBitmap(string,43,true);								//プレーンナンバー２にシステム用ＢＭＰを読み込む
				syu = 1;
			}
			enemy[4 + ( i * 20 )] = syu;
			enemy[5 + ( i * 20 )] = e_hp( type ) ;
			enemy[6 + ( i * 20 )] = 0;				/* やられカウンタ */
			enemy[7 + ( i * 20 )] = 0;				/* 位置補正 */
			enemy[8 + ( i * 20 )] = 0;				/* 表示パターン */
			enemy[9 + ( i * 20 )] = 0;				/* 向き */
			enemy[10+ ( i * 20 )] = 0;				/* 表示Ｙ */
			enemy[11+ ( i * 20 )] = type;			/* 種類 */
			enemy[12+ ( i * 20 )] = 0;				/* ジャンプフレームカウンタ */
			enemy[13+ ( i * 20 )] = 0;				/* ジャンプカウンタ */
			enemy[14+ ( i * 20 )] = x;				/* 再配置情報X */
			enemy[15+ ( i * 20 )] = y;				/* 再配置情報Y */
			enemy[16+ ( i * 20 )] = 0;				/* 無敵時間 */
			enemy[17+ ( i * 20 )] = 0;
			enemy[18+ ( i * 20 )] = 0;
			enemy[19+ ( i * 20 )] = 0;				/*  */

			if ( size >= 4 )	/* ボス */
			{
//				init_event_set_enm( enemy[11+ ( i * 20 )] , i );
				enemy[17+ ( i * 20 )] = 1;
			}
			break;
		}
	}
}

//敵作成
void enm_set2( long type, long x, long y ,long muki )
{	
	long i;
	long size;
	long syu;
	long j;

	for ( i = 0; i < 20; i++ )
	{
		if ( enemy[0 + ( i * 20 )] == 0 )
		{
			for ( j = 0; j < 20 ; j++ )
			{
				enemy[j + ( i * 20 )] = 0;
			}
			
			
			enemy[0 + ( i * 20 )] = 1;
			enemy[1 + ( i * 20 )] = x;
			enemy[2 + ( i * 20 )] = y;
			size = type / 100;
			syu = type % 100;
			enemy[3 + ( i * 20 )] = size;
			if ( size >= 4 )	/* ボス */
			{
				sprintf(string,"image/enm/boss%d.bmp", (int)syu );
				LoadBitmap(string,43,true);								//プレーンナンバー２にシステム用ＢＭＰを読み込む
				syu = 1;
			}
			enemy[4 + ( i * 20 )] = syu;
			enemy[5 + ( i * 20 )] = e_hp( type ) ;
			enemy[6 + ( i * 20 )] = 0;				/* やられカウンタ */
			enemy[7 + ( i * 20 )] = 0;				/* 位置補正 */
			enemy[8 + ( i * 20 )] = 0;				/* 表示パターン */
			enemy[9 + ( i * 20 )] = muki;			/* 向き */
			enemy[10+ ( i * 20 )] = 0;				/* 表示Ｙ */
			enemy[11+ ( i * 20 )] = type;			/* 種類 */
			enemy[12+ ( i * 20 )] = 0;				/* ジャンプフレームカウンタ */
			enemy[13+ ( i * 20 )] = 0;				/* ジャンプカウンタ */
			enemy[14+ ( i * 20 )] = x;				/* 再配置情報X */
			enemy[15+ ( i * 20 )] = y;				/* 再配置情報Y */
			enemy[16+ ( i * 20 )] = 0;				/* 無敵時間 */
			enemy[17+ ( i * 20 )] = 0;
			enemy[18+ ( i * 20 )] = 0;
			enemy[19+ ( i * 20 )] = 0;				/*  */

			if ( size >= 4 )	/* ボス */
			{
//				init_event_set_enm( enemy[11+ ( i * 20 )] , i );
				enemy[17+ ( i * 20 )] = 1;
			}
			break;
		}
	}
}


/* 敵のＨＰを設定（e_setで使われる） */
long e_hp( long type )
{
	long rc;
	rc = 1;
	if ( ( type / 100 ) == 1 )	/* 32 */
	{
		rc = enm1_parameter[ 10 * ( type - 100 ) + 0 ];
	}
	if ( ( type / 100 ) == 2 )	/* 48 */
	{
		rc = enm2_parameter[ 10 * ( type - 200 ) + 0 ];
	}
	if ( ( type / 100 ) == 3 )	/* 64 */
	{
		rc = enm3_parameter[ 10 * ( type - 300 ) + 0 ];
	}
	if ( ( type / 100 ) == 4 )	/* 128 */
	{
		rc = enm4_parameter[ 10 * ( type - 400 ) + 0 ];
	}
	return ( rc );
}

void enm_disp()
{	
	long i;
	long size ;
	long p_h;
	long all;
	long size_hosei_x;
	long size_hosei_y;
	long isclaer;
	long is_tukami;
	long disp_x_j;
	long disp_y_j;

	disp_x_j = 0;
	disp_y_j = 0;
	
	is_tukami = 0;
	isclaer = 0; 
	size_hosei_x = 0;
	size_hosei_y = 0;
	p_h = 32;	/* プレイヤーの当たり判定 */
	all = 1;
	size = 0;
	
	for ( i = 0; i < 20; i++ )
	{
		if ( enemy[0 +( i * 20 )] == 1 )
		{
			isclaer = 1;
			all = 0;
			/* 無敵時間が存在する場合はカウント */
			if ( enemy[16 + ( i * 20 )] > 0 )
			{
				enemy[16 + ( i * 20 )]--;
			}

			if ( enemy[3 + ( i * 20 )] == 1 )
			{
				size = 32;
			}
			if ( enemy[3 + ( i * 20 )] == 2 )
			{
				size = 48;
			}
			if ( enemy[3 + ( i * 20 )] == 3 )
			{
				size = 64;
			}
			if ( enemy[3 + ( i * 20 )] == 4 )	/* 5ボス */
			{
				size = 128;
			}
			if ( enemy[3 + ( i * 20 )] == 5 )	/* 5ボス */
			{
				size = 128;
			}
			if ( enemy[3 + ( i * 20 )] == 6 )	/* 6ボス */
			{
				size = 128;
			}
			/* ボスのサイズ補正 */

			if ( enemy[11+ ( i * 20 )] == 401 )	
			{
//				size_hosei_x = 32;
				size_hosei_y = 20;
			} 
			else if ( enemy[11+ ( i * 20 )] == 403 )	
			{
				size_hosei_x = 32;
				size_hosei_y = 20;
			} 
			else if ( enemy[11+ ( i * 20 )] == 404 )	
			{
				size_hosei_x = 5;
				size_hosei_y = 20;
			} 
			else 
			{
				size_hosei_x = 0;
				size_hosei_y = 0;
			}

			/* 画面内の敵だけ描画する */
			disp_x_j = enemy[1 + ( i * enm_data[2] )];
			disp_y_j = ( 0 - enemy[2 + ( i * enm_data[2] )] ) + MapInfo[3];

			if ( ( ( d_x - size ) <= ( disp_x_j ) ) 
			  && ( ( d_y - size ) <= ( disp_y_j  ) )
			  && ( ( d_x + 320 + size ) >= ( disp_x_j ) )
			  && ( ( d_y + 240 + size ) >= ( disp_y_j ) ) )
			{
				if ( enemy[6 + ( i * 20 )] > 0 )	/* やられ */
				{
					if ( ( ( enemy[16 + ( i * 20 )] % 2 ) == 0 ) || ( sn != 1 ) )
					{
						if ( enemy[3 + ( i * 20 )] >= 4 )	/* ボスのときは表示方法が違う */
						{
							BltRect( 39 + enemy[3 + ( i * 20 )] , enemy[1 + ( i * 20 )]  - d_x, ( 0 - enemy[2 + ( i * 20 )] ) + 480 - size  - d_y, ( size * 4 ) , size + ( size * enemy[9 + ( i * 20 )] ), size, size );
						}
						else 
						{
							BltRect( 39 + enemy[3 + ( i * 20 )] , enemy[1 + ( i * 20 )]  - d_x, ( 0 - enemy[2 + ( i * 20 )] ) + 480 - size  - d_y, ( size * 8 ) + ( enemy[9 + ( i * 20 )] *  size ) , size * ( enemy[4 + ( i * 20 )] + enemy[10+ ( i * 20 )] ), size, size );
						}
					}
				}
				else 
				{
						
					if ( ( ( enemy[16 + ( i * 20 )] % 2 ) == 0 ) || ( sn != 1 ) )
					{
						if ( enemy[3 + ( i * 20 )] >= 4 )	/* ボスのときは表示方法が違う */
						{
							BltRect( 39 + enemy[3 + ( i * 20 )] , enemy[1 + ( i * 20 )]  - d_x, ( 0 - enemy[2 + ( i * 20 )] ) + 480 - size  - d_y, ( size * enemy[8 + ( i * 20 )] ) , size * ( enemy[4 + ( i * 20 )] + enemy[10+ ( i * 20 )] ) + ( enemy[9 + ( i * 20 )] * size ), size, size );
						}
						else 
						{
							if ( sn == 5 )
							{
								test[0] = 1;
							}
							BltRect( 39 + enemy[3 + ( i * 20 )] , enemy[1 + ( i * 20 )]  - d_x, ( 0 - enemy[2 + ( i * 20 )] ) + 480 - size  - d_y, ( size * enemy[8 + ( i * 20 )] ) + ( enemy[9 + ( i * 20 )] * ( size * 4 ) ) , size * ( enemy[4 + ( i * 20 )] + enemy[10+ ( i * 20 )] ), size, size );
						}
					}
				}
			}
			
			e_move( i );	/* 敵キャラ種類別動作 */
				

			if ( enemy[3 + ( i * 20 )] >= 4 )	/* 5ボス */
			{
				if ( enemy[i * 20 + 1] > ( 640 - 128 ) )
				{
					enemy[i * 20 + 1] = ( 640 - 128 );
				}
				if ( enemy[i * 20 + 1] < 1 )
				{
					enemy[i * 20 + 1] = 1;
				}
			}
			
			if ( ( enemy[i * 20 + 1] >= 665 ) || ( enemy[i * 20 + 1] <= -100 ) || ( enemy[i * 20 + 2] <= -100 ) || ( enemy[i * 20 + 2] >= 500 ))
			{
				enemy[i * 20 + 0] = 0;
			}
/*
			if ( sn != 1 )
			{
				return;
			}
*/
			/* プレイヤーとの当たり判定 */
			if ( ( ( (enemy[1 + i * 20] + size_hosei_x ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
				&& ( (player[0] + 10 ) < (enemy[1 + i * 20] + size - size_hosei_x ) )
				&& ( (enemy[2 + i * 20] + size_hosei_y ) < (player[1] + p_h) )
				&& ( (player[1] + 5 ) < ( enemy[2 + i * 20] + size - size_hosei_y ) ) ) 
				&& ( player[17] <= 0 ) 
				&& ( enemy[6 + i * 20] == 0 ) )
			{
				enemy[0 + ( i * 20 )] = 0;		/* 消去 */
				soundPlaySe( EN_SE_L1 );
			}
#if false
			if ( ( ( (enemy[1 + i * 20] + size_hosei_x ) < (player[0] + player[30] + 32 ) )	/* アンカーとのあたり判定 */
				&& ( (player[0] + player[30] ) < (enemy[1 + i * 20] + size - size_hosei_x ) )
				&& ( (enemy[2 + i * 20] + size_hosei_y ) < (player[1] + player[31] + 32 ) )
				&& ( (player[1] + player[31] ) < ( enemy[2 + i * 20] + size - size_hosei_y ) ) ) 
				&& ( enemy[6 + i * 20] == 0 )
				&& ( player[23] == 1 )	/* 伸ばしている */
				&& ( player[24] == 0 )	/* 伸ばしている */
				&& ( is_tukami == 0 ) )	/* 伸ばしている */
			{
				if ( player[9] == 0 )
				{
					player[13] = 0;
					player[14] = 0;
					player[32] = enemy[1 + i * 20];
					player[33] = enemy[2 + i * 20];
					player[19] = player[21];
				}
				player[9] = 1;	/* 掴んでいる */
				is_tukami = 1;
			}
#endif			
			/* 倒せない敵（針等） */
//			if ( ( enemy[11 + i * 20]  == 105 )		/* 電撃 */
//			  || ( enemy[11 + i * 20]  == 107 ) )	/* 発生 */
//			{
//			}
//			else 
#if false
			{
				
				for ( k = 0; k < 10 ; k++ )
				{
					size2 = 32;
					/* プレイヤーの弾との当たり判定 */
					if ( ( ( (enemy[1 + i * 20]  ) < (pshot[1 + k * 10] + size2 ) )
						&& ( (pshot[1 + k * 10] ) < (enemy[1 + i * 20] + size ) )
						&& ( (enemy[2 + i * 20] ) < (pshot[2 + k * 10] + size2) )
						&& ( (pshot[2 + k * 10] ) < ( enemy[2 + i * 20] + size ) ) ) 
						&& ( pshot[0 + k * 10] == 1 ) )	/* アニメーションが３番 */
					{
						enemy[0 + ( i * 20 )] = 0;		/* 消去 */
					}
					
					size2 = 32;
					/* プレイヤーの弾との当たり判定 */
					/* プレイヤーの弾との当たり判定 */
					if ( ( ( ( enemy[1 + i * 20] + size_hosei_x ) < ( ( pmshot[5 + k * 20] >> 16 )  + size2 ) )
						&& ( ( ( pmshot[5 + k * 20] >> 16 ) ) < ( enemy[1 + i * 20] + size - size_hosei_x  ) )
						&& ( ( enemy[2 + i * 20] + size_hosei_y ) < ( ( pmshot[6 + k * 20] >> 16 ) + size2 ) )
						&& ( ( ( pmshot[6 + k * 20] >> 16 ) ) < ( enemy[2 + i * 20] + size - size_hosei_y  ) ) ) 
						&& ( pmshot[2 + k * 20] == 1 ) )
					{
						/* 無敵時間が存在しない */
						if ( enemy[16 + ( i * 20 )] == 0 )
						{
							/* 攻撃が効かない */
							if ( ( enemy[11 + ( i * 20 )] == 402 ) && ( pmshot[7 + k * 20] != 2 ) )
							{
								pmshot[2 + k * 20] = 0;		/* 弾消去 */
								p_shot_f_set( ( pmshot[5 + k * 20] >> 16 ) , ( pmshot[6 + k * 20] >> 16 ), 0 );
								p_shot_f2_set( ( pmshot[5 + k * 20] >> 16 ) + 15, ( pmshot[6 + k * 20] >> 16 ) + 15, 0 );
							}
							else 
							{
								
								p_shot_f2_set( ( pmshot[5 + k * 20] >> 16 ) + 15, ( pmshot[6 + k * 20] >> 16 ) + 15, 0 );
								
								if ( ( pmshot[3 + k * 20] == 2 )	/* スキルクラッシュショット */
								  || ( pmshot[3 + k * 20] == 4 ) )	/* スキルクラッシュショット */
								{
									p_shot_f_set( ( pmshot[5 + k * 20] >> 16 ) + ( rand( ) % 32 ) - 16, ( pmshot[6 + k * 20] >> 16 ) + ( rand( ) % 32 ) - 16, 0 );
									p_shot_f_set( ( pmshot[5 + k * 20] >> 16 ) + ( rand( ) % 32 ) - 16, ( pmshot[6 + k * 20] >> 16 ) + ( rand( ) % 32 ) - 16, 0 );
									p_shot_f_set( ( pmshot[5 + k * 20] >> 16 ) + ( rand( ) % 32 ) - 16, ( pmshot[6 + k * 20] >> 16 ) + ( rand( ) % 32 )- 16, 0 );
									enemy[5 + ( i * 20 )] = enemy[5 + ( i * 20 )] - 3;
								}
								else 
								{
									p_shot_f_set( ( pmshot[5 + k * 20] >> 16 ) , ( pmshot[6 + k * 20] >> 16 ), 0 );
									enemy[5 + ( i * 20 )] = enemy[5 + ( i * 20 )] - 1;
								}

								pmshot[2 + k * 20] = 0;		/* 弾消去 */
								if ( enemy[5 + ( i * 20 )] <= 0 )	/* 倒した */
								{

									if ( enemy[3 + ( i * 20 )] >= 4 )	/* 4ボス */
									{
										boss_dead( enemy[1 + ( i * 20 )], enemy[2 + ( i * 20 )] );
										event_set_enm( enemy[11 + ( i * 20 )] , i );
										for ( l = 0; l < 20; l++ )
										{
											enemy[5 + ( l * 20 )] = 0;
											enemy[6 + ( l * 20 )] = 60;		/* 消去 */
											enemy[16 + ( l * 20 )] = 60;
										}
									}
									
									enemy[6 + ( i * 20 )] = 60;		/* 消去 */
									enemy[16 + ( i * 20 )] = 60;

								}
								else 
								{
									/* 倒してない */
									/* 無敵時間設定 */
									enemy[6 + ( i * 20 )] = 30;		/* のけぞり時間 */
	//								enemy[7 + ( i * 20 )] = 6;		/* 位置補正 */
									enemy[16 + ( i * 20 )] = 60;	/* 無敵時間 */
									if ( enemy[3 + ( i * 20 )] >= 4 )	/* ボスのときはのけぞりがない */
									{
										enemy[6 + ( i * 20 )] = 0;		/* のけぞり時間 */
	//									enemy[7 + ( i * 20 )] = 0;		/* 位置補正 */
									}
								}
							}
						}
					}
				}
			}
#endif
			if ( enemy[7 + ( i * 20 )] > 0 )
			{
				enemy[7 + ( i * 20 )]--;					/* 移動補正 */
				if ( enemy[9 + ( i * 20 )] == 0 )
				{
					if ( ( ( map1_item_j( enemy[1 + ( i * 20 )] - 4 , enemy[2 + ( i * 20 )] + 2 ) >= 100 )
					    && ( map1_item_j( enemy[1 + ( i * 20 )] - 4 , enemy[2 + ( i * 20 )] + 2 ) <= 119 ) )
					  || ( ( map1_item_j( enemy[1 + ( i * 20 )] - 4 , enemy[2 + ( i * 20 )] + 2 ) >= 200 )
					    && ( map1_item_j( enemy[1 + ( i * 20 )] - 4 , enemy[2 + ( i * 20 )] + 2 ) <= 219 ) ) )
					{
					}
					else 
					{
						enemy[1 + ( i * 20 )] = enemy[1 + ( i * 20 )] - ( enemy[7 + ( i * 20 )] * enemy[7 + ( i * 20 )] );
					}
				}
				else 
				{
					if ( ( ( map1_item_j( enemy[1 + ( i * 20 )] + size + 4, enemy[2 + ( i * 20 )] + 2 ) >= 100 )
					    && ( map1_item_j( enemy[1 + ( i * 20 )] + size + 4, enemy[2 + ( i * 20 )] + 2 ) <= 119 ) )
					  || ( ( map1_item_j( enemy[1 + ( i * 20 )] + size + 4, enemy[2 + ( i * 20 )] + 2 ) >= 200 )
					    && ( map1_item_j( enemy[1 + ( i * 20 )] + size + 4, enemy[2 + ( i * 20 )] + 2 ) <= 219 ) ) )
					{
					}
					else 
					{
						enemy[1 + ( i * 20 )] = enemy[1 + ( i * 20 )] + ( enemy[7 + ( i * 20 )] * enemy[7 + ( i * 20 )] );
					}
				}
			}
			
			if ( enemy[6 + ( i * 20 )] > 0 )
			{
				enemy[6 + ( i * 20 )]--;					/* 消えるまでカウンタ */
				if ( enemy[6 + ( i * 20 )] == 0 )
				{
					if ( enemy[5 + ( i * 20 )] <= 0 )
					{
						if ( enemy[3 + ( i * 20 )] >= 4 )
						{
						}
						else 
						{
							if ( ( rand( ) % 2 ) == 0 )	/* ライフ出現 */
							{
								kane_set( enemy[1 + ( i * 20 )], enemy[2 + ( i * 20 )] );
							}
						}
						enemy[0 + ( i * 20 )] = 0;		/* 消去 */
					}
				}
				
			}
		}
	}

	if (isclaer == 0)	
	{
		sn = 2;
		demo[0] = 0;
	} 
}

void e_move( long i )
{
	
	if ( enemy[6 + ( i * 20 )] > 0 )
	{
		return;
	}
	if ( enemy[5 + ( i * 20 )] <= 0 )	/* 倒した */
	{
		return;
	}
	
	enemy[8 + ( i * 20 )] = 0;		/* アニメーションパターン */
	enemy[14+ ( i * 20 )] = 0;

}

/* ボスを倒した演出 */
void boss_dead( long x, long y )
{
/*	
	long i;
	long xx;
	long yy;
	for ( i = 0; i < 100; i++ ) 
	{
//		BltFromBackupPlane();		//	バックアッププレーンからのBlt
		if ( ( i % 10 ) == 0 )
		{
			xx = ( rand( ) % 96 );
			yy = ( rand( ) % 96 );
			se_wav( 24 + ( rand( ) % 3 ) );
			p_shot_f_set( x + xx, y + yy, 0 );
			p_shot_f2_set(  x + xx + 15, y + yy + 15, 0 );
		}
		p_shot_f_disp( );
		p_shot_f2_disp( );
		halt;
	}
*/
}

//-----------------------------------------------------------------------------------
/* 敵ショット生成 */
void e_shot_set( long type , long i , long x, long y )
{
	long ii;
	long j;
	long rg;
	long mx;
	long my;
	
	/* 発生位置X */
	/* 発生位置Y */
	for ( ii = 0; ii < 20; ii++ )
	{
		if ( mgc_e[ii * 10 + 0] == 0 )					/* 存在してない */
		{

			for ( j = 0 ; j <= 9 ;j++ )
			{
				mgc_e[ii * 10 + j] = 0;
			}
			if ( type == 1061 )							/* ヤドカリ */
			{
				mx = ( ( 32 / 2 ) - 5 ) + player[0] - x;
				my = ( ( 32 / 2 ) - 5 ) + player[1] - y;
				rg = funcTan2( mx, my );
				rg = ( 90 );
				mgc_e[ii * 10 + 1] = x << 16;		/* X */
				mgc_e[ii * 10 + 2] = y << 16;		/* Y */
				mgc_e[ii * 10 + 3] = 0;		/* アニメーションカウンタ */
				mgc_e[ii * 10 + 4] = funcSin( rg ) * 4;		/* 移動方向 */
				mgc_e[ii * 10 + 5] = funcCos( rg ) * 4;	/* 移動方向 */
				mgc_e[ii * 10 + 6] = 6;		/* ダメージ */
				mgc_e[ii * 10 + 7] = 2;		/* 属性 */
				mgc_e[ii * 10 + 9] = type;					/* タイプplayer[10] */
			}

			mgc_e[ii * 10 + 0] = 1;					/* 存在あり */
			break;
		}
	}

}
/* ショット移動 */
void e_shot_disp( )
{
	long i;
	long p_h;
	long size ;
	
	p_h = 22;	/* プレイヤーの当たり判定 */
	size = 0;
	
	/* 発生位置X */
	/* 発生位置Y */
	for ( i = 0; i < 20; i++ )
	{
		if ( mgc_e[i * 10 + 0] == 1 )					/* 存在している */
		{
			if ( mgc_e[i * 10 + 9] == 4010 ) 	/* １ボス */
			{
				size = 32;
				BltRect( 50, ( mgc_e[i * 10 + 1] >> 16 ) - d_x , ( 0 - ( mgc_e[i * 10 + 2] >> 16 ) ) + 480 - 32 - d_y, mgc_e[i * 10 + 3] * 32, 0 , 32, 32 );
				mgc_e[i * 10 + 1] = mgc_e[i * 10 + 1] + mgc_e[i * 10 + 4];		/* X */
				mgc_e[i * 10 + 2] = mgc_e[i * 10 + 2] + mgc_e[i * 10 + 5];		/* Y */
				mgc_e[i * 10 + 3]++;					/* アニメーションカウンタ */
				if ( mgc_e[i * 10 + 3] >= 4 )
				{
					mgc_e[i * 10 + 3] = 0;
				}
			}
			
			/* プレイヤーとの当たり判定 */
			if ( ( ( ( ( mgc_e[1 + i * 10] >> 16 ) ) < (player[0] + 22 ) )	/* 自機とのあたり判定 */
				&& ( (player[0] + 10 ) < ( ( mgc_e[1 + i * 10] >> 16 ) + size ) )
				&& ( ( ( mgc_e[2 + i * 10] >> 16 ) ) < (player[1] + p_h) )
				&& ( (player[1] + 5  ) < ( ( mgc_e[2 + i * 10] >> 16 ) + size ) ) ) 
				&& ( player[17] == 0 ) )
			{
				player[10] = 0;
				player[11] = 0;
				player[12] = 4;
				player[13] = 0;
				player[2] = 31;	/* ダメージ */
				player[17] = 120;	/* 無敵時間 */
				player[8] = player[8] - 1;
			}


			if ( ( ( mgc_e[i * 10 + 1] >> 16 ) >= 640 ) || ( ( mgc_e[i * 10 + 1] >> 16 ) <= 0 ) || ( ( mgc_e[i * 10 + 2] >> 16 ) <= -100 )|| ( ( mgc_e[i * 10 + 2] >> 16 ) >= 480 ))
			{
				mgc_e[i * 10 + 0] = 0;
			}
		}
	}
}

/* 敵用ジャンプ */
void enmjmp( long i )
{
	long y1;
	long px ;
	long px1 ;
	long px2 ;
	long py ;
	long py2 ;

	long size;
	long e10;
	/* 画面外では行わない */
/*
	if ( enemy[1 + ( i * 20 )] > 460 )
	{
		return;
	}
*/
	if ( enemy[3 + ( i * 20 )] == 1 )
	{
		size = 16;
	}
	else if ( enemy[3 + ( i * 20 )] == 2 )
	{
		size = 24;
	}
	else
	{
		size = 16;
	}

	if ( enemy[13 + ( i * 20 )] == 10 ) 
	{
		/* 今回の位置 */
		px = ( enemy[1 + ( i * 20 )] + 32 ) / 32;
		px1 = ( enemy[1 + ( i * 20 )] + size ) / 32;
		px2 = ( enemy[1 + ( i * 20 )] + size ) / 32;
		py = ( 16 - ( ( enemy[2 + ( i * 20 )] + 28 ) / 32 ) );
		/* 100 ～ 200は地面 */
		if ( ( !( ( map1[ px1 + ( ( py ) * 20 ) ] >= 100 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 119 ) ) )
		  && ( !( ( map1[ px2 + ( ( py ) * 20 ) ] >= 100 ) && ( map1[ px2 + ( ( py ) * 20 ) ] <= 119 ) ) ) 
		  && ( !( ( map1[ px1 + ( ( py ) * 20 ) ] >= 200 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 219 ) ) )
		  && ( !( ( map1[ px2 + ( ( py ) * 20 ) ] >= 200 ) && ( map1[ px2 + ( ( py ) * 20 ) ] <= 219 ) ) ) 
		  && ( !( ( map1[ px1 + ( ( py ) * 20 ) ] >= 600 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 619 ) ) )
		  && ( !( ( map1[ px2 + ( ( py ) * 20 ) ] >= 600 ) && ( map1[ px2 + ( ( py ) * 20 ) ] <= 619 ) ) ) )
		{
			enemy[13+ ( i * 20 )] = 0;
			enemy[12+ ( i * 20 )] = 0;
		}
		else 
		{
			return;
		}
	}

	enemy[12 + ( i * 20 )]++;
	if ( enemy[12+ ( i * 20 )] >= 10 )
	{
		enemy[12+ ( i * 20 )] = 0;
		
		enemy[13 + ( i * 20 )] = enemy[13 + ( i * 20 )] - 3;
//		if ( player[5] == 0 )
//		{
//			player[5] = -3;
//		}
		if ( ( enemy[13 + ( i * 20 )] <= 2 ) && ( enemy[13 + ( i * 20 )] > -2 ) )
		{
			enemy[13 + ( i * 20 )] = -3;
		}
		if ( enemy[13 + ( i * 20 )] < -8 )
		{
			enemy[13 + ( i * 20 )] = -8;
		}
		/* 地面判定 */
	}


	/* 画面端で落ちるのを防止 */
	e10 = enemy[13 + ( i * 20 )];
	if (  enemy[1+ ( i * 20 )] > ( 630 - size - size ) )
	{
		if ( enemy[13+ ( i * 20 )] < 0 )	/* 落下 */
		{
			e10 = 0;
		}
	}


	/* 今回の位置 */
	y1 = ( ( 0 - e10 ) * ( 0 - e10 ) * ( 0 - e10 ) );
//	player[1] = player[1] - ( y1 / 30 );


	enemy[2 + ( i * 20 )] = enemy[2 + ( i * 20 )] - ( y1 / 25 );
	px1 = ( enemy[1 + ( i * 20 )] + size ) / 32;
	px2 = ( enemy[1 + ( i * 20 )] + size ) / 32;
	py = ( 16 - ( ( enemy[2 + ( i * 20 )] + 28 ) / 32 ) );
	py2 = ( 16 - ( ( enemy[2 + ( i * 20 )] + 49 + 48 ) / 32 ) );
//	py = ( 16 - ( ( player[1] ) / 32 ) );

	if ( enemy[2 + ( i * 20 )] > 480 ) 
	{
		return;
	}

	if ( enemy[13 + ( i * 20 )] <= 0 )													/* 上り中は判定しない */
	{
		/* 地面判定 */
		if ( ( ( map1[ px1 + ( ( py ) * 20 ) ] >= 100 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 119 ) )
		  || ( ( map1[ px1 + ( ( py ) * 20 ) ] >= 200 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 219 ) ) 
		  || ( ( map1[ px1 + ( ( py ) * 20 ) ] >= 600 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 619 ) )
		  || ( ( map1[ px2 + ( ( py ) * 20 ) ] >= 600 ) && ( map1[ px2 + ( ( py ) * 20 ) ] <= 619 ) ) )
		{
			enemy[2 + ( i * 20 )] = ( ( 16 - py ) * 32 ) - 16; 
			enemy[13 + ( i * 20 )] = 10;
		}
	}
	else		/* 上昇中 */
	{
		if ( enemy[1+ ( i * 20 )] < ( 630 - size - size ) )
		{
			px = ( enemy[1 + ( i * 20 )] + size ) / 32;
			py = ( 16 - ( ( enemy[2 + ( i * 20 )] - ( y1 / 25 ) + 16 + 48  ) / 32 ) );

			/* 地面判定 */
			if ( ( ( map1[ px + ( ( py ) * 20 ) ] >= 100 ) && ( map1[ px + ( ( py ) * 20 ) ] <= 119 ) )
			  || ( ( map1[ px + ( ( py ) * 20 ) ] >= 200 ) && ( map1[ px + ( ( py ) * 20 ) ] <= 219 ) ) )
			{
				enemy[13+ ( i * 20 )] = -3;
				enemy[12+ ( i * 20 )] = 0;
			}
		}

	}
	if ( enemy[2 + ( i * 20 )] <= -14 )
	{
		enemy[0 + ( i * 20 )] = 0;
	}

}


/* 移動後の座標のマスを返す */
long map1_item_j( long x, long y )
{
	long px ;
	long py ;
	long i;
	long rc;
	
	px = ( x ) / 32;
	py = ( 15 - ( ( y + 16 ) / 32 ) );

	if ( py <= 0 )
	{
		py = 1;
	}
	
	rc = map1[ px + ( ( py ) * 20 ) ];
	
	/* アイテムとのあたり判定 */
	for ( i = 0; i < 50; i++ )
	{
		if ( item[( i * 10 ) + 0 ] == 1 )
		{
			/* プレイヤーとの当たり判定 */
			if ( ( ( (item[1 + ( i * 10 )]  ) < ( x ) )	/* 自機とのあたり判定 */
				&& ( ( x ) < (item[1 + ( i * 10 )] + 31 ) )
				&& ( (item[2 + ( i * 10 )] ) < ( y ) )
				&& ( ( y ) < ( item[2 + ( i * 10 )] + 32 ) ) ) )
			{
				if ( ( item[( i * 10 ) + 3 ] >= 500 ) && ( item[( i * 10 ) + 3 ] <= 620 ) ) /* 移動不可アイテム */
				{
					rc = 101;
					break;
				}
			}
		}

	}


	return( rc );
	
	
	
	
}


void kane_set( long x, long y )
{
	long i;
	
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
			kane[6 + ( i * 10 )] = 6;
			kane[7 + ( i * 10 )] = 0;
			kane[8 + ( i * 10 )] = 0;
			kane[9 + ( i * 10 )] = 0;
			break;
		}
	}
}

void kane_disp(  )
{
	long i;
	
	for ( i = 0; i < 20; i++ )
	{
		if ( kane[0 + ( i * 10 )] == 1 )
		{
			kane[8 + ( i * 10 )]++;
			if ( kane[8 + ( i * 10 )] >= 10 )
			{
				kane[8 + ( i * 10 )] = 0;
				kane[7 + ( i * 10 )]++;
				if ( kane[7 + ( i * 10 )] >= 4 )
				{
					kane[7 + ( i * 10 )] = 0;
				}
			} 
			if ( ( ( kane[9 + ( i * 10 )] % 2 ) == 0 ) || ( kane[9 + ( i * 10 )] < 240 ) )
			{
				BltRect( 12 , kane[1 + ( i * 10 )] - d_x, ( 0 - kane[2 + ( i * 10 )] ) + 480 - 32 - d_y, 32 * 4 , 0, 32, 32 );
			}
			
			kane[9 + ( i * 10 )]++;
			if ( kane[9 + ( i * 10 )] >= 300 )
			{
				kane[0 + ( i * 10 )] = 0;
			} 
			kane[5 + ( i * 10 )]++;
			if ( kane[5 + ( i * 10 )] >= 10 )
			{
				kane[5 + ( i * 10 )] = 0;
			} 

			k_jmp( i );
			/* とった */
			if ( ( ( (kane[1 + ( i * 10 )]  ) < (player[0] + 10 ) )	/* 自機とのあたり判定 */
				&& ( (player[0] + 22 ) < (kane[1 + ( i * 10 )] + 32 ) )
				&& ( (kane[2 + ( i * 10 )] ) < (player[1] + 32 ) )
				&& ( (player[1] + 5 ) < ( kane[2 + ( i * 10 )] + 32 ) ) ) )
			{
				player[8]++;
				if ( player[8] > save_data[6] )
				{
					player[8] = save_data[6];
				}
				kane[0 + ( i * 10 )] = 0;
			}
		}
	}
}
/* ジャンプＹ位置計算 */
void k_jmp( long i )
{
	long y1;
	long px1 ;
	long px2 ;
	long py ;
	long py2 ;
	long py3 ;

	if ( kane[6 + ( i * 10 )] == 10 ) 
	{
		return;
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
	
	/* 画面端で落ちるのを防止 */
	if ( ( kane[1 + ( i * 10 )] < 0 ) || ( kane[1 + ( i * 10 )] > 610 ) )
	{
		kane[6 + ( i * 10 )] = 0;
	}
	/* 今回の位置 */
	y1 = ( ( 0 - kane[6 + ( i * 10 )] ) * ( 0 - kane[6 + ( i * 10 )] ) * ( 0 - kane[6 + ( i * 10 )] ) );

	px1 = ( kane[1 + ( i * 10 )] + 16 ) / 32;
	px2 = ( kane[1 + ( i * 10 )] + 16 ) / 32;
	py = ( 16 - ( ( kane[2 + ( i * 10 )] + 28 ) / 32 ) );
	py2 = ( 16 - ( ( kane[2 + ( i * 10 )] + 49 + 48 ) / 32 ) );
	kane[2 + ( i * 10 )] = kane[2 + ( i * 10 )] - ( y1 / 25 );
	py3 = ( 16 - ( ( kane[2 + ( i * 10 )] + 32 + 32 + 16 ) / 32 ) );

	if ( kane[2 + ( i * 10 )] > 480 ) 
	{
		return;
	}

	if ( kane[6 + ( i * 10 )] <= 0 ) 													/* 上り中は判定しない */
	{
		/* 地面判定 */
		if ( ( ( map1[ px1 + ( ( py ) * 20 ) ] >= 100 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 299 ) )
		  || ( ( map1[ px2 + ( ( py ) * 20 ) ] >= 100 ) && ( map1[ px2 + ( ( py ) * 20 ) ] <= 299 ) ) 
		  || ( ( map1[ px1 + ( ( py ) * 20 ) ] >= 600 ) && ( map1[ px1 + ( ( py ) * 20 ) ] <= 619 ) )
		  || ( ( map1[ px2 + ( ( py ) * 20 ) ] >= 600 ) && ( map1[ px2 + ( ( py ) * 20 ) ] <= 619 ) ) )
		{
			kane[2 + ( i * 10 )] = ( ( 16 - py ) * 32 ) - 16; 
			kane[6 + ( i * 10 )] = 10;
		}
	}
	else		/* 上昇中 */
	{
		/* 地面判定 */
		if ( ( ( map1[ px1 + ( ( py3 ) * 20 ) ] >= 100 ) && ( map1[ px1 + ( ( py3 ) * 20 ) ] <= 119 ) )
		  || ( ( map1[ px1 + ( ( py3 ) * 20 ) ] >= 200 ) && ( map1[ px1 + ( ( py3 ) * 20 ) ] <= 219 ) )
		  || ( ( map1[ px2 + ( ( py3 ) * 20 ) ] >= 100 ) && ( map1[ px2 + ( ( py3 ) * 20 ) ] <= 119 ) )
		  || ( ( map1[ px2 + ( ( py3 ) * 20 ) ] >= 200 ) && ( map1[ px2 + ( ( py3 ) * 20 ) ] <= 219 ) ) )
		{
			kane[6 + ( i * 10 )] = -1;
			kane[5 + ( i * 10 )] = 0;
		}
	}
	if ( kane[2 + ( i * 10 )] <= -14 )
	{
		kane[0 + ( i * 10 )] = 0;
	}

}

void p_jump_f_set( long x, long y )
{
	long i;
	
	for ( i = 0; i < 10; i++ )
	{
		if ( pjumpf[( i * 10 ) + 0 ] == 0 )
		{
			pjumpf[( i * 10 ) + 0 ] = 1;
			pjumpf[( i * 10 ) + 1 ] = x;
			pjumpf[( i * 10 ) + 2 ] = y;
			pjumpf[( i * 10 ) + 5 ] = 0;
			pjumpf[( i * 10 ) + 6 ] = 0;
			break;
		}
	}
}

void p_jump_f_disp( )
{
	long i;
	
	for ( i = 0; i < 10; i++ )
	{
		if ( pjumpf[( i * 10 ) + 0 ] == 1 )
		{
			BltRect( 5, pjumpf[( i * 10 ) + 1 ] - d_x, 480 - pjumpf[( i * 10 ) + 2 ] - 32 - d_y , pjumpf[( i * 10 ) + 6 ] * 32, 48, 32, 32 );
			
			pjumpf[( i * 10 ) + 5 ]++;
			if ( pjumpf[( i * 10 ) + 5 ] >= 5 )
			{
				pjumpf[( i * 10 ) + 5 ] = 0;
				pjumpf[( i * 10 ) + 6 ]++;
				if ( pjumpf[( i * 10 ) + 6 ] >= 4 )
				{
					pjumpf[( i * 10 ) + 6 ] = 0;
					pjumpf[( i * 10 ) + 0 ] = 0;
				}
			}
		}
	}
}

void screen_BackupPlane( )
{
}

/* メニューの表示 */
void window_disp( )
{
	BltRect( 61, 96, 64 + ( 32 * demo[0] ), 0, gameflag[122] * 32 , 32, 32 );
	Blt( 60, 0, 0 );
}

/* メニューでのキー操作 */
void window_keys( )
{
	if ( IsPushKey( gameflag[0] ) )
	{
		soundPlaySe( EN_SE_SELECT );
		demo[0]--;
		if ( demo[0] < 0 )
		{
			demo[0] = 2;
		}
	}
	else if ( IsPushKey( gameflag[1] ) )
	{
		soundPlaySe( EN_SE_SELECT );
		demo[0]++;
		if ( demo[0] > 2 )
		{
			demo[0] = 0;
		}
	}
	//	スペースキーが押される
	if ( IsPushOKKey( ) ) 
	{
		if ( demo[0] == 0 )
		{
			sn = 1;
		}
		if ( demo[0] == 1 )
		{
			gameflag[71] = 1;
			sn = 10;
		}
		if ( demo[0] == 2 )
		{
			g_scene = EN_SN_TITLE;	/* タイトルへ */
//			gameflag[40] = 1;
			sn = 10;
		}
	}
	//Bが押される
	if ( IsPushCancelKey( ) )
	{
		sn = 1;
	}

}



/* 絶対値を返す */
long labs( long para )
{
	long rc;
	
	rc = para;
	
	if ( para < 0 )
	{
		rc = para * -1;
	}
	return( rc );
}

/* 魔法生成 */
void p_mgc_set( long muki , long p3 )
{
	long i;
	long k; 
	long ii;
	long rg;
	long max;
	
	rg = 0;
	
	if ( muki == 0 )
	{
		rg = ( 0 );
	}
	else if ( muki == 1 )
	{
		rg = ( 45 );
	}
	else if ( muki == 2 )
	{
		rg = ( 90 );
	}
	else if ( muki == 3 )
	{
		rg = ( 135 );
	}
	else if ( muki == 4 )
	{
		rg = ( 180 );
	}

	if ( p3 == 1 )
	{
		rg = rg * -1;
	}
 
	/* 発生位置X */
	/* 発生位置Y */

	max = 1;
	for ( i = 0; i < max; i++ )
	{

		k = 5;

		for ( ii = 0; ii < 40; ii++ )
		{
			if ( pmshot[ii * 20 + 2] == 0 )
			{
				pmshot[ii * 20 + 0] = funcSin( rg ) * k;			/* 弾速x */
				pmshot[ii * 20 + 1] = funcCos( rg ) * k;			/* 弾速y */
				pmshot[ii * 20 + 2] = 1;						/* 存在 */
				pmshot[ii * 20 + 3] = 0;						/* 種類 */
				pmshot[ii * 20 + 4] = 0 ;						/*  */
				pmshot[ii * 20 + 5] = ( player[0] ) << 16 ;			/* 初期位置の設定 */
				pmshot[ii * 20 + 6] = ( player[1] ) << 16 ;
				pmshot[ii * 20 + 7] = muki;						/* 方向 */
				
				pmshot[ii * 20 + 8] = 0;						/*  */
				pmshot[ii * 20 + 9] = 0 ;						/*  */
				pmshot[ii * 20 + 10] = 0 ;						/*  */
				break;
			}
		}
	}


}

void p_mgc_disp( )
{
	long ii;
	long f;
	long b;
	
	for ( ii = 0; ii < 40; ii++ )
	{
		if ( pmshot[ii * 20 + 2] == 1 )
		{
			
			f = 1;
			b = 230;
			
			
			BltRect( 4, ( pmshot[ii * 20 + 5] >> 16 ) - d_x, 480 - ( pmshot[ii * 20 + 6] >> 16 ) - 32 - d_y, 0, 96, 32, 32 );
//			BlendExBltRectW( 4, ( pmshot[ii * 20 + 5] >> 16 ) - d_x, 480 - ( pmshot[ii * 20 + 6] >> 16 ) - 32 - d_y, 0, 96, 32, 32 , f, b );

			pmshot[ii * 20 + 5] = pmshot[ii * 20 + 0] + pmshot[ii * 20 + 5];		/* 弾速x */
			pmshot[ii * 20 + 6] = pmshot[ii * 20 + 1] + pmshot[ii * 20 + 6];		/* 弾速x */
			
			if ( ( ( pmshot[ii * 20 + 5] >> 16 ) > 640 ) || ( ( pmshot[ii * 20 + 5] >> 16 ) < -32 ) )
			{
				pmshot[ii * 20 + 2] = 0;				/* 存在無し */
			}
			if ( ( ( pmshot[ii * 20 + 6] >> 16 ) > 480 ) || ( ( pmshot[ii * 20 + 6] >> 16 ) < -32 ) )
			{
				pmshot[ii * 20 + 2] = 0;				/* 存在無し */
			}
			
			if ( ( ( map1_item_j( ( pmshot[ii * 20 + 5] >> 16 ) + 16 , ( pmshot[ii * 20 + 6] >> 16 ) + 16 ) >= 100 )
			    && ( map1_item_j( ( pmshot[ii * 20 + 5] >> 16 ) + 16 , ( pmshot[ii * 20 + 6] >> 16 ) + 16 ) <= 220 ) ) )
			{
				pmshot[ii * 20 + 2] = 0;				/* 存在無し */
				p_shot_f2_set( ( pmshot[5 + ii * 20] >> 16 ) + 15, ( pmshot[6 + ii * 20] >> 16 ) + 15, 0 );
				if ( pmshot[3 + ii * 20] == 2 )	/* スキルクラッシュショット */
				{
					p_shot_f_set( ( pmshot[5 + ii * 20] >> 16 ) + ( rand( ) % 32 ) - 16, ( pmshot[6 + ii * 20] >> 16 ) + ( rand( ) % 32 ) - 16, 0 );
					p_shot_f_set( ( pmshot[5 + ii * 20] >> 16 ) + ( rand( ) % 32 ) - 16, ( pmshot[6 + ii * 20] >> 16 ) + ( rand( ) % 32 ) - 16, 0 );
					p_shot_f_set( ( pmshot[5 + ii * 20] >> 16 ) + ( rand( ) % 32 ) - 16, ( pmshot[6 + ii * 20] >> 16 ) + ( rand( ) % 32 ) - 16, 0 );
				}
				else 
				{
					p_shot_f_set( ( pmshot[5 + ii * 20] >> 16 ) , ( pmshot[6 + ii * 20] >> 16 ), 0 );
				}
			}
			
			
		}
	}

}

void replay_input_save( void )
{
	long input_keys;
	input_keys = 0;
	
	/* キー処理は基本的にplayer[2]が何状態の時にキーを入れたらどうなるを記述していく */
	if ( IsPressKey( gameflag[2] ) )
	{
		input_keys = input_keys + 1;
	}
	else if ( IsPressKey( gameflag[3] ) )
	{
		input_keys = input_keys + 10;
	}
	
	if ( IsPressKey( gameflag[0] ) )
	{
		input_keys = input_keys + 100;
	}
	else if ( IsPressKey( gameflag[1] ) )
	{
		input_keys = input_keys + 1000;
	}
	
	//	スペースキーが押される
	if ( IsPressKey( gameflag[4] ) )
	{
		input_keys = input_keys + 10000;
	}
	
	//Bが押される
	if ( IsPressKey( gameflag[5] ) )
	{
		input_keys = input_keys + 100000;
	}
	
	//Bが押される
	if ( IsPushKey( gameflag[5] ) )
	{
		input_keys = input_keys + 1000000;
	}
	/* 12ボタン　か　Ｃが押された */
//	if ( ( ( IsPushJoyKey(6) != 0 ) || ( IsPushJoyKey(12) != 0 ) || IsPushKey( gameflag[6] ) ) )
//	{
//		input_keys = input_keys + 10000000;
//	}
	
	replay[replay_time] = input_keys;
}

void replay_input_load( void )
{
	long i;
	long input_keys;
	input_keys = 0;
	
	for ( i = 0; i < 10 ; i++ )
	{
		replay_load_key[i] = 0 ;	
	}
	
	input_keys = replay[replay_time];
	
//	if ( input_keys >= 10000000 )
//	{
//		input_keys = input_keys - 10000000;
//		replay_load_key[7] = 1 ;	
//	}
	if ( input_keys >= 1000000 )
	{
		input_keys = input_keys - 1000000;
		replay_load_key[6] = 1 ;	
	}
	if ( input_keys >= 100000 )
	{
		input_keys = input_keys - 100000;
		replay_load_key[5] = 1 ;	
	}
	if ( input_keys >= 10000 )
	{
		input_keys = input_keys - 10000;
		replay_load_key[4] = 1 ;	
	}
	if ( input_keys >= 1000 )
	{
		input_keys = input_keys - 1000;
		replay_load_key[3] = 1 ;	
	}
	if ( input_keys >= 100 )
	{
		input_keys = input_keys - 100;
		replay_load_key[2] = 1 ;	
	}
	if ( input_keys >= 10 )
	{
		input_keys = input_keys - 10;
		replay_load_key[1] = 1 ;	
	}
	if ( input_keys >= 1 )
	{
		input_keys = input_keys - 1;
		replay_load_key[0] = 1 ;	
	}
}

void replay_file_save_w( void )
{

	long d_num;
	d_num = 1;
	sprintf(string,"replay/%d/replay_data_w_%d.dat", (int)stage, (int)save_data[3] );
	SaveFile( string, &replay[0], sizeof( replay ) );

}
void replay_file_save( long type )
{

	long d_num;
	d_num = replay_file_name;
	if ( type == 0 )
	{
		sprintf(string,"replay/%d/replay_data_%d.dat", (int)stage, (int)save_data[3] );
	}
	else
	{
		sprintf(string,"replay/%d/replay_data_j%d.dat", (int)stage, (int)save_data[3] );
	}
	SaveFile( string, &replay[0], sizeof( replay ) );
}

long replay_file_load( void )
{
	long rc ;
	long d_num;
	
	rc = 1;
	d_num = 1;
	
	if ( gameflag[125] == 0 )
	{
		sprintf( string, "replay/%d/replay_data_%d.dat", (int)stage, (int)save_data[3] );
	}
	else 
	{
		sprintf( string, "replay/%d/replay_data_j%d.dat", (int)stage, (int)save_data[3] );
	}
	if ( LoadFile( string, &replay[0], sizeof( replay ) ) )
	{
		rc = 0;
	}

	return( rc );
}


long GetKyori( long i1, long i2 )
{
	long rc = 0;
	
	rc = ( long )sqrt( ( i1 * i1 ) + ( i2 * i2 ) );
	
	return ( rc );
}

void debugdisp( void )
{
#if DEBUG_DISP
	BltNumericImage( player[21], 8, 10, 10, 18, 0, 0, 10, 8 );	
	BltNumericImage( replay[replay_time], 8, 10, 20, 18, 0, 0, 10, 8 );	
	BltNumericImage( player[31], 8, 10, 30, 18, 0, 0, 10, 8 );	
	BltNumericImage( funcSin( player[20] ), 8, 10, 40, 18, 0, 0, 10, 8 );	
	BltNumericImage( funcCos( player[20] ), 8, 10, 50, 18, 0, 0, 10, 8 );	
	BltNumericImage( funcTan2( player[30],player[31] ), 8, 10, 60, 18, 0, 0, 10, 8 );	
	BltNumericImage( ( MOTatan(player[30] * 256,player[31] * 256) ), 8, 10, 70, 18, 0, 0, 10, 8 );	
#endif
}







