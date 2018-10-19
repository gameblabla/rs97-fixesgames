#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>
#include <unistd.h>

#include "define.h"
#include "extern.h"
#include "function.h"
#include "util_snd.h"

#include "scene.h"

#ifdef GP2X
#include <unistd.h>
#endif

#ifdef PSPUMODE
#include <pspkernel.h>
#endif 

#ifdef SCALING
#include "get_resolution.h"
#endif

SDL_Surface *ScreenSurface;
void main_init( void );
void main_init_config( void );
void ExitProgram(void);

///----------------------------------------------------------------------------
/// PSP Related Stuff
///----------------------------------------------------------------------------
#ifdef PSPUMODE
PSP_MODULE_INFO("Gannatsu Portable", 0, 1, 1);
PSP_HEAP_SIZE_KB(21*1024);
int iDoRun;
#endif

int main(int argc, char *argv[])
{
	SDL_Rect rect;
	Uint32 flags;
	SDL_Surface* tmp;
	char AppPath[1024];
	char AppPathw[1024];

#ifdef PSPUMODE
	// PSP things
	//psp_setup_callbacks();
	scePowerSetClockFrequency(333, 333, 166);
#endif 

	/* 初期化 */
	memset( &AppPath[0], '\0', sizeof( AppPath ) );
	memset( &AppPathw[0], '\0', sizeof( AppPath ) );
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return 1;

#ifdef GP2X
#else
	#ifdef MacOS
		if ( argc > 0 )
		{
			strcpy( &AppPath[0], argv[0] );
			char_work = strrchr( &AppPath[0], '/' );
			strlen = char_work - &AppPath[0];
			strncpy( &AppPathw[0], &AppPath[0], strlen );
			chdir( &AppPathw[0] );
		}
	#else
	#endif
#endif	
	
#ifdef GP2X
	flags = SDL_HWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWPALETTE;
#elif defined(PSPUMODE)
	flags = SDL_HWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWPALETTE;
#elif defined(GCW)
	//flags = SDL_HWSURFACE | SDL_TRIPLEBUF;
	flags = SDL_HWSURFACE/* | SDL_TRIPLEBUF*/;
#elif defined(DREAMCAST)
	flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
#else
	flags = SDL_SWSURFACE;
#endif

	SDL_ShowCursor(SDL_DISABLE);
	
#ifdef SCALING
	Get_Resolution();
	printf("screen_scale.w_display %d \n", screen_scale.w_display);
	printf("screen_scale.w_scale %d \n", screen_scale.w_scale);
	printf("screen_scale.w_scale_size %d \n", screen_scale.w_scale_size);
	real_screen = SDL_SetVideoMode(screen_scale.w_display, screen_scale.h_display, DEPTH, SDL_SWSURFACE | SDL_NOFRAME);
	g_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, DISPLY_WIDTH, DISPLY_HEIGHT, DEPTH, 0,0,0,0);
#else
	printf("%d %d %d\n", DISPLY_WIDTH, DISPLY_HEIGHT, DEPTH);
	//g_screen = SDL_SetVideoMode(DISPLY_WIDTH, DISPLY_HEIGHT, DEPTH, flags);
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
	g_screen = SDL_CreateRGBSurface(flags, DISPLY_WIDTH, DISPLY_HEIGHT, DEPTH, 0, 0, 0, 0);
#endif

	rect.x = (DISPLY_WIDTH-320)/2;
	rect.y = (DISPLY_HEIGHT-240)/2;
	rect.w = DISPLY_WIDTH;
	rect.h = DISPLY_HEIGHT;
	
	tmp = SDL_LoadBMP("image/color.bmp");	
	SetGscreenPalette( tmp );
	SDL_FreeSurface(tmp);
	
	if (!g_screen)
	{
		fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
		return 1;
	}
	
	SDL_WM_SetCaption("gnp", "");

	FunctionInit( );
	main_init( );

	scenemanager( );
	
	ExitProgram( );
	return 0;
}


void main_init( void )
{
	g_scene = EN_SN_LOGO;

	main_init_config( );	

#ifndef NOSOUND
	soundLoadBuffer(EN_BGM_GAME01, (Uint8 *)"sound/bgm/01.ogg", -1);
	soundLoadBuffer(EN_BGM_GAME02, (Uint8 *)"sound/bgm/02.ogg", -1);
	soundLoadBuffer(EN_BGM_GAME03, (Uint8 *)"sound/bgm/03.ogg", -1);
	soundLoadBuffer(EN_BGM_GAME04, (Uint8 *)"sound/bgm/04.ogg", -1);
	soundLoadBuffer(EN_BGM_GAME05, (Uint8 *)"sound/bgm/05.ogg", -1);
	soundLoadBuffer(EN_BGM_GAME06, (Uint8 *)"sound/bgm/06.ogg", -1);
	soundLoadBuffer(EN_BGM_GAME07, (Uint8 *)"sound/bgm/07.ogg", -1);
	
	soundLoadBufferSE(EN_SE_ATK1   , (Uint8 *)"sound/se/atk1.wav" );
	soundLoadBufferSE(EN_SE_DAMEGE , (Uint8 *)"sound/se/damage.wav" );
	soundLoadBufferSE(EN_SE_L1     , (Uint8 *)"sound/se/l1.wav" );
	soundLoadBufferSE(EN_SE_LANDING, (Uint8 *)"sound/se/landing.wav" );
	soundLoadBufferSE(EN_SE_MSG    , (Uint8 *)"sound/se/msg.wav" );
	soundLoadBufferSE(EN_SE_SELECT , (Uint8 *)"sound/se/select.wav" );
	soundLoadBufferSE(EN_SE_SW     , (Uint8 *)"sound/se/sw.wav" );
	soundLoadBufferSE(EN_SE_JUMP   , (Uint8 *)"sound/se/jump.wav" );
	soundLoadBufferSE(EN_SE_PAWA   , (Uint8 *)"sound/se/puwa.wav" );
#endif

	Set_Volume( gameflag[60] );

}

void ExitProgram(void)
{
	char path_config[512];
	char path_folder[512];

#ifdef MINGW
	sprintf(path_config, "save/config");
	sprintf(path_folder, "save");
#elif defined(_TINSPIRE)
	sprintf(path_config, "./save/config.tns");
	sprintf(path_folder, "./save");
#else		
	sprintf(path_config, "%s/.ganbare/config", getenv("HOME"));
	sprintf(path_folder, "%s/.ganbare", getenv("HOME"));
#endif
	
	soundStopBgm(0);
	soundStopSeAll();
	soundRelease();

	printf("SaveGameFlag(path_config); (ExitProgram)\n");
	SaveGameFlag(path_config);
	
	closePAD();
	SDL_Quit();

#ifdef GP2X
	chdir("/usr/gp2x");
	execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif
#ifdef PSPUMODE
	sceDisplayWaitVblankStart();
	sceKernelExitGame(); 
#endif
}


void main_init_config( void )
{
	int i;
	char path_config[512];
	char path_folder[512];
		
#ifdef MINGW
	sprintf(path_config, "save/config");
	sprintf(path_folder, "save");
#elif defined(_TINSPIRE)
	sprintf(path_config, "./save/config.tns");
	sprintf(path_folder, "./save");
#elif defined(DREAMCAST)
	sprintf(path_config, "/cd/.ganbare/config");
	sprintf(path_folder, "/cd/.ganbare");
#else		
	sprintf(path_config, "%s/.ganbare/config", getenv("HOME"));
	sprintf(path_folder, "%s/.ganbare", getenv("HOME"));
#endif

#ifdef MINGW
	mkdir(path_folder);
#elif defined(_TINSPIRE)
	mkdir(path_folder, 0755);
#else	
	mkdir(path_folder, 0755);
#endif
	
	i = 0;
	
	if ( ! ( LoadGameFlag(path_config) == 0 ) )
	{
		ResetGameFlag();
#ifdef GP2X
		gameflag[0]=GP2X_BUTTON_UP;		//上	Up
		gameflag[1]=GP2X_BUTTON_DOWN;	//下	Down
		gameflag[2]=GP2X_BUTTON_LEFT;	//左	Left
		gameflag[3]=GP2X_BUTTON_RIGHT;	//右	Right
		gameflag[4]=GP2X_BUTTON_X;	//Ａ	Z
		gameflag[5]=GP2X_BUTTON_B;	//Ｂ	X
		gameflag[6]=GP2X_BUTTON_Y;	//Ｃ	C
		gameflag[7]=GP2X_BUTTON_A;	//Ｄ	S
		gameflag[8]=GP2X_BUTTON_L;	//Ｄ	S
		gameflag[9]=GP2X_BUTTON_R;	//Ｄ	S
		gameflag[10]=GP2X_BUTTON_START;	//Ｄ	S
		gameflag[11]=GP2X_BUTTON_SELECT;	//Ｄ	S
#else 
		gameflag[0]=GP2X_BUTTON_UP;		//上	Up
		gameflag[1]=GP2X_BUTTON_DOWN;	//下	Down
		gameflag[2]=GP2X_BUTTON_LEFT;	//左	Left
		gameflag[3]=GP2X_BUTTON_RIGHT;	//右	Right
		gameflag[4]=GP2X_BUTTON_A;	//Ａ	Z
		gameflag[5]=GP2X_BUTTON_X;	//Ｂ	X
		gameflag[6]=GP2X_BUTTON_Y;	//Ｃ	C
		gameflag[7]=GP2X_BUTTON_B;	//Ｄ	S
		gameflag[8]=GP2X_BUTTON_L;	//Ｄ	S
		gameflag[9]=GP2X_BUTTON_R;	//Ｄ	S
		gameflag[10]=GP2X_BUTTON_START;	//Ｄ	S
		gameflag[11]=GP2X_BUTTON_SELECT;	//Ｄ	S
#endif
		
		gameflag[60] = 128;	//音量

		gameflag[70] = 1;	//タイトル画面から来た
		gameflag[71] = 1;	//リトライを選択した


		gameflag[100] = 0;	//クリアフラグ
		
		gameflag[107] = 0;	//play time s
		gameflag[108] = 0;	//play time s
		gameflag[109] = 0;	//play time m
		gameflag[110] = 0;	//play time h
		gameflag[111] = 59;	//CLEAR time s
		gameflag[112] = 59;	//CLEAR time m
		gameflag[113] = 99;	//CLEAR time h
		
		
		gameflag[120] = 1;	//ステージセレクト
		gameflag[121] = 1;	//到達ステージ
		gameflag[122] = 0;	//キャラセレクト
		gameflag[123] = -1;	//リプレイ選択時のステージ
		gameflag[124] = 0;	//リプレイＡＬＬ、ＯＮＥ
		gameflag[125] = 0;	//リプレイjamp
		gameflag[126] = 0;	//裏面
		gameflag[127] = 0;	//トータルアタック
		
		gameflag[132] = 0;	//リプレイ
		gameflag[133] = 0;	//アンカータイプ

		gameflag[135] = 0;	//トータルアタック 時間					[130]>[135]
		gameflag[136] = 0;	//トータルアタック ミス				[131]>[136]
		gameflag[137] = 1595959;	//トータルアタック クリア時間	[132]>[137]
		gameflag[138] = 999;	//トータルアタック クリアミス	[133]>[138]
		
		for ( i = 0; i <= 99; i++ )
		{
			gameflag[200 + i] = 1105959;
		}
		for ( i = 0; i <= 99; i++ )
		{
			gameflag[300 + i] = 100;
		}
	}
	gameflag[121] = 50;	//到達ステージ
	gameflag[100] = 1;	//到達ステージ
}




