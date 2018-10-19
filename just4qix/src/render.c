/*the rendering code
reads the game world and renders it to screen
*/

#include <SDL/SDL.h>
#include "misc.h"
#include "render.h"
#include "update.h"
#include "particle.h"

SDL_Surface* Surf_Display;
SDL_Surface* ScreenSurface;
SDL_Surface* Surf_Background;
int backHeight;
SDL_Surface* Surf_Reveal;

SDL_Surface* Surf_Figure;
SDL_Surface* Surf_Player;
SDL_Surface* Surf_Logo;
SDL_Surface* Surf_Temp;
SDL_Surface* Surf_Text;
SDL_Surface* Surf_Font;
SDL_Surface* Surf_Boss;
SDL_Surface* Surf_Henchman;
SDL_Surface* Surf_FarmGirl;

char fontMap[] = " !'ch%l`()u+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char message[] = "                                 WELCOME TO JUST4QIX.                 HELLO! TO EVERYONE IN THE DINGOO-DINGUX SCENE.                             A BIG WAVE TO EVERYONE ON THE A320 FORUMS!.                       LONG LIVE THE DINGOO!       HI TO MY FAMILY MEL, JAMES AND ISOBEL.      I HOPE YOU LIKE THE GAME..............";
int messagePos = 0;

extern float menu_ofx;
extern float menu_ofy;

extern float playerx;
extern float playery;
extern int menu_shake;
extern int menu_index;

extern float bossx;
extern float bossy;

extern bool menu_on;
extern int lives;
extern int level;
extern int dispPercent;
extern int percent;
extern float backy;
extern int dispLvlNameUntil;
extern char lvlName[];
extern Henchman henchmen[];  //not sure about this?
extern int ActiveHench;
int henchFrame=0;
int playerAnimSpeed=50;
extern int invinsibleUntil;
char percentStr[6];
char livesStr[10];
char levelStr[10];

SDL_Surface* Load(const char* File) {
    SDL_Surface* Surf_Temp = NULL;
    SDL_Surface* Surf_Return = NULL;

    if((Surf_Temp = SDL_LoadBMP(File)) == NULL) {
        return NULL;
    }

    Surf_Return = SDL_DisplayFormat(Surf_Temp);
    SDL_FreeSurface(Surf_Temp);

    return Surf_Return;
}

bool Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y) {
    if(Surf_Dest == NULL || Surf_Src == NULL) {
        return false;
    }

    SDL_Rect DestR;

    DestR.x = X;
    DestR.y = Y;

    SDL_BlitSurface(Surf_Src, NULL, Surf_Dest, &DestR);

    return true;
}

void DrawFont(SDL_Surface* Surf_Dest, int x, int y, char* str)
{
	int fontWidth=16;
	int fontHeight=16;
	int i;
	int j;
	int c;
	SDL_Rect srect;
	SDL_Rect drect;
	for (i=0; str[i]!='\0'; ++i)
	{
		c=-1;
		for (j=0; fontMap[j]!='\0'; ++j){
			if (fontMap[j]==str[i]) { c=j; }
		}
		if (c==-1) { return;} //character not found
		int fx = c * fontWidth;
		int fy = 0;
		
		srect.x = fx;  
		srect.y = fy;
		srect.w = fontWidth;
		srect.h = fontHeight;
		drect.x = x+(i*fontWidth);
		drect.y = y;
		
		if(SDL_BlitSurface(Surf_Font, &srect, Surf_Dest, &drect) < 0)
			fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
		
	}
}

void DrawFontCentred( SDL_Surface* Surf_Dest, char* str) {
	int l=strlen(str);
	int x=(screen_width/2) - (16*l)/2;
	int y=(screen_height/2) - (16/2);
	DrawFont(Surf_Dest, x, y, str);
}

void DrawFontCentredX( SDL_Surface* Surf_Dest, int y, char* str) {
	int l=strlen(str);
	int x=(screen_width/2) - (16*l)/2;
	DrawFont(Surf_Dest, x, y, str);
}


void drawMessage(){
	static int mx=0;
	static int chppos;
	int fontWidth=16;
	int fontHeight=16;
	int chpos = chppos;
	int x,j,c;
	SDL_Rect srect;
	SDL_Rect drect;
	for (x=-mx ; x< screen_width ;x+=fontWidth, chpos ++){
		if (message[chpos] =='\0') { chpos = 0;} //wrap message around
		//map message character to font position
		c=-1;
		for (j=0; fontMap[j]!='\0'; ++j){
			//printf("font %d mess %d\n",fontMap[j],message[chpos]);
			if (fontMap[j]==message[chpos]) { c=j; }
		}
		//printf("char %d chpos %d\n",c,chpos);
		if (c==-1) { continue;} //character not found
		
		int fx = c * fontWidth;
		int fy = 0;
		srect.x = fx;  
		srect.y = 0;
		srect.w = fontWidth;
		srect.h = fontHeight;
		drect.x = x;
		drect.y = 200;
		if(SDL_BlitSurface(Surf_Font, &srect, Surf_Display, &drect) < 0)
			fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
	}
	mx= mx + 1;
	if (mx >15) { mx = 0; 
		chppos++;
		if (message[chppos] =='\0') { chppos = 0;}
	}
}


bool DrawTiled(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int W, int H, int OX, int OY) { // X,Y,W,H are tiled area, OX, OY = offset of tile
    if(Surf_Dest == NULL || Surf_Src == NULL) {
        return false;
    }

    SDL_Rect DestR;
    SDL_Rect ClipR;
	int sw = Surf_Src->w;
	int sh = Surf_Src->h;
   
	int ix;
	int iy;
        int ox = sw - (OX % sw);
        int oy = sh - (OY % sh);
	ClipR.x = X;
	ClipR.y = Y;
	ClipR.w = W;
	ClipR.h = H;
        SDL_SetClipRect(Surf_Dest, &ClipR); //clip screen to tile area
	for (ix =X-ox; ix < X + W; ix += sw) {
	for (iy =Y-oy; iy < Y + H; iy += sh) {
		DestR.x = ix;
		DestR.y = iy;
		SDL_BlitSurface(Surf_Src, NULL, Surf_Dest, &DestR);
	}
	}	
	SDL_SetClipRect(Surf_Dest, NULL);  //reset clipping to normal
    return true;
}
  


void RenderMenu() {
	static int girlframe = 0;
	static int girlframedir = 1;
	static int timer=0;
	if (SDL_GetTicks() > timer) {
		girlframe += girlframedir;
		if (girlframe == 78) { girlframedir = -1;timer = SDL_GetTicks() +500;}
		else if (girlframe == 0) { girlframedir = 1;timer = SDL_GetTicks() +500;}
		else { timer = SDL_GetTicks() +50; }
	}
	
	//scroller
	SDL_FillRect(Surf_Display,NULL,SDL_MapRGB(Surf_Display->format,0, 0,0));
	//farmgirl
	
	SDL_Rect DestR;
	SDL_Rect SrcR;
	DestR.x = (screen_width - 264) / 2;
	DestR.y = (screen_height - 198) / 2;
	SrcR.x =(girlframe % 10) * 264;
	SrcR.y = (girlframe / 10) * 198;
	SrcR.w = 264;
	SrcR.h = 198;
	SDL_BlitSurface(Surf_FarmGirl, &SrcR, Surf_Display,&DestR);
	
	
	//********
	//DrawTiled(Surf_Display, Surf_MainScroll, 0, 80 ,320, 100,menu_ofx,menu_ofy);
	
	Draw(Surf_Display, Surf_Logo, 0, 60); 
	//figure
	//Draw(Surf_Display, Surf_Figure, 310-Surf_Figure->w, 0);
	
	//menu
	int i;
	for (i=0;i<3;++i){
		SDL_Rect DestR;
		SDL_Rect SrcR;
		if (menu_index == i) { DestR.x = 40 + menu_shake;}
		else { DestR.x = 40; }
		DestR.y = 100+i*20;
		SrcR.x =0;
		SrcR.y = 11*i;
		SrcR.w = 133;
		SrcR.h = 10;
		SDL_BlitSurface(Surf_Text, &SrcR, Surf_Display, &DestR);
	}
	drawMessage();
	//SDL_Flip(Surf_Display);
  SDL_SoftStretch(Surf_Display, NULL, ScreenSurface, NULL);
  SDL_Flip(ScreenSurface);
}

void RenderStart(){
	
	//background
	SDL_BlitSurface(Surf_Background, NULL, Surf_Display, NULL);	
	//overlay (hides background)
	SDL_BlitSurface(Surf_Reveal, NULL, Surf_Display, NULL);
	sprintf(levelStr,"LEVEL %d",level);
	DrawFontCentred( Surf_Display, levelStr);
	DrawFontCentredX(Surf_Display,screen_height/2 + 20,lvlName);
	
	//SDL_Flip(Surf_Display);
  SDL_SoftStretch(Surf_Display, NULL, ScreenSurface, NULL);
  SDL_Flip(ScreenSurface);
}

void RenderLevelOver(){
	
	SDL_Rect srect;
	srect.x=0;
	srect.y=backy;
	srect.w=320;
	srect.h=240;
	//background
	SDL_BlitSurface(Surf_Background, &srect, Surf_Display, NULL);	
	DrawFontCentred(Surf_Display,"WELL DONE!");
	 
	DrawParticles(Surf_Display, 0,0);
	sprintf(percentStr,"%d%%",dispPercent);
	DrawFontCentredX(Surf_Display,(screen_height / 2) + 20,percentStr);
	sprintf(livesStr,"LIVES %d",lives);
	DrawFontCentredX(Surf_Display,(screen_height / 2) + 40,livesStr);
	//SDL_Flip(Surf_Display);
  SDL_SoftStretch(Surf_Display, NULL, ScreenSurface, NULL);
  SDL_Flip(ScreenSurface);
}

void RenderGameOver(){
	
	DrawFontCentred(Surf_Display,"GAME OVER");
	//SDL_Flip(Surf_Display);
  SDL_SoftStretch(Surf_Display, NULL, ScreenSurface, NULL);
  SDL_Flip(ScreenSurface);
}





void renderHenchmen()
{
	static henchFrame=0;
	static timer = 0 ;
	if (SDL_GetTicks()>timer) { 
		henchFrame = (henchFrame +1) % 33;
		timer = SDL_GetTicks()+50;
	}
	if (ActiveHench <0) { return; }
	int i;
	for (i=0;i < ActiveHench; i++)
	{
		//Draw(Surf_Display, Surf_Henchman, henchmen[i].x - Surf_Henchman->w/2, henchmen[i].y- Surf_Henchman->h/2);
		int bw=30;
		int bh=60;
		SDL_Rect DestR;
		SDL_Rect SrcR;
		DestR.x =  henchmen[i].x-bw/2;
		DestR.y =  henchmen[i].y-bh/1.2;
		SrcR.y=bh * (henchFrame/7);
		SrcR.x= bw * (henchFrame % 7);
		SrcR.w=bw;
		SrcR.h=bh;
		SDL_BlitSurface( Surf_Henchman, &SrcR, Surf_Display, &DestR);
	}
}

void DrawBoss(int bossx, int bossy)
{
	static bossFrame=0;
	static timer = 0 ;
	if (SDL_GetTicks()>timer) { 
		bossFrame = (bossFrame +1) % 16;
		timer = SDL_GetTicks()+50;
	}
	int bw=128;
	int bh=126;
	SDL_Rect DestR;
	SDL_Rect SrcR;
	DestR.x = bossx-bw/2;
	DestR.y = bossy-bh/2;
	SrcR.y=bh * (bossFrame/4);
	SrcR.x= bw * (bossFrame % 4);
	
	SrcR.w=bw;
	SrcR.h=bh;
	SDL_BlitSurface(Surf_Boss, &SrcR, Surf_Display, &DestR);

  
}

void DrawPlayer(int x, int y)
{
	static fade=0;
	static playerFrame=0;
	static timer = 0 ;
	if (SDL_GetTicks()>timer) { 
		playerFrame = (playerFrame +1) % 16;
		timer = SDL_GetTicks()+playerAnimSpeed;
		fade = (fade + 10) % 255;
	}
	int bw=35;
	int bh=34;
	SDL_Rect DestR;
	SDL_Rect SrcR;
	DestR.x = x-bw/2;
	DestR.y = y-bh/2;
	SrcR.y=bh * (playerFrame/4);
	SrcR.x= bw * (playerFrame % 4);
	
	SrcR.w=bw;
	SrcR.h=bh;
	if (invinsibleUntil>0) {
		SDL_SetAlpha(Surf_Player,SDL_SRCALPHA, fade);
	} else { 
		SDL_SetAlpha(Surf_Player,SDL_SRCALPHA, 255);
	}
	SDL_BlitSurface(Surf_Player, &SrcR, Surf_Display, &DestR);

  
}


void Render(){
	static int shakex,shakey;
	//background
	SDL_BlitSurface(Surf_Background, NULL, Surf_Display, NULL);	
	//overlay (hides background)
	SDL_BlitSurface(Surf_Reveal, NULL, Surf_Display, NULL);	
	
	//henchmen
	renderHenchmen();
	//player
	//Draw(Surf_Display, Surf_Player, playerx - (Surf_Player->w)/2, playery - (Surf_Player->h)/2);
	DrawPlayer( playerx , playery );
	DrawParticles(Surf_Display, 0,0);
	DrawBoss(bossx, bossy);
	 
	//HUD
	if (dispPercent > 65) { 
		int shakeMax = (dispPercent-65)/2 +1;
		shakex = rand() % shakeMax - shakeMax/2; 
		shakey = rand() % shakeMax - shakeMax/2; } 
	else { shakex =0; shakey =0;}
	sprintf(percentStr,"%d%%",dispPercent);
	DrawFont(Surf_Display,240+shakex,20+shakey,percentStr);
	sprintf(livesStr,"LIVES %d",lives);
	DrawFont(Surf_Display,30,20,livesStr);
	
	//show it
	//SDL_Flip(Surf_Display);
  SDL_SoftStretch(Surf_Display, NULL, ScreenSurface, NULL);
  SDL_Flip(ScreenSurface);
}


bool InitVideo(){
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		return false;
	}
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
	if((Surf_Display = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0)) == NULL) {
		return false;
	}
	SDL_ShowCursor(0);
	return true;
}

void PleaseWait()
{
	DrawFontCentred(Surf_Display,"PLEASE WAIT...");
	//SDL_Flip(Surf_Display);
  SDL_SoftStretch(Surf_Display, NULL, ScreenSurface, NULL);
  SDL_Flip(ScreenSurface);
}


bool InitGfx(){
	
	
	if((Surf_Temp = Load("gfx/kromagrad_16x16.bmp")) == NULL) {
		return false;
	}
	Surf_Font = SDL_DisplayFormat(Surf_Temp);
	SDL_FreeSurface(Surf_Temp);
	SDL_SetColorKey( Surf_Font, SDL_SRCCOLORKEY, SDL_MapRGB(Surf_Font->format,255, 0,0) );	
	
	PleaseWait();
	
	if((Surf_Player= Load("gfx/atom.bmp")) == NULL) {
		return false;
	} 
	SDL_SetColorKey( Surf_Player, SDL_SRCCOLORKEY, SDL_MapRGB(Surf_Player->format,0, 0,0) );
	if((Surf_Logo= Load("gfx/logo.bmp")) == NULL) {
		return false;
	}
	SDL_SetColorKey( Surf_Logo, SDL_SRCCOLORKEY, SDL_MapRGB(Surf_Player->format,0, 0,0) );
	if((Surf_Text= Load("gfx/text.bmp")) == NULL) {
		return false;
	} 
	SDL_SetColorKey( Surf_Text, SDL_SRCCOLORKEY, SDL_MapRGB(Surf_Player->format,255, 255,255) );
	
	
	
	Surf_Reveal=SDL_CreateRGBSurface(SDL_SWSURFACE,320,240,16,0,0,0,0);
	SDL_SetColorKey( Surf_Reveal, SDL_SRCCOLORKEY, SDL_MapRGB(Surf_Reveal->format,255, 255,255) );
	
	if((Surf_Temp= Load("gfx/boss.bmp")) == NULL) {
		return false;
	} 
	Surf_Boss = SDL_DisplayFormat(Surf_Temp);
	SDL_FreeSurface(Surf_Temp);
	SDL_SetColorKey( Surf_Boss, SDL_SRCCOLORKEY, SDL_MapRGB(Surf_Boss->format,0, 0,0) );
	
	if((Surf_Temp= Load("gfx/fire.bmp")) == NULL) {
		return false;
	} sprintf(percentStr,"%d%%",dispPercent);
	
	Surf_Henchman = SDL_DisplayFormat(Surf_Temp);
	SDL_FreeSurface(Surf_Temp);
	SDL_SetColorKey( Surf_Henchman, SDL_SRCCOLORKEY, SDL_MapRGB(Surf_Henchman->format,0, 0,0) );
	
	if((Surf_Temp= Load("gfx/farmgirl.bmp")) == NULL) {
		return false;
	} 
	Surf_FarmGirl = SDL_DisplayFormat(Surf_Temp);
	SDL_FreeSurface(Surf_Temp);
	
	return true;
}




void CleanUpVideo(){
	SDL_FreeSurface(Surf_Display);
	
	
	SDL_FreeSurface(Surf_Logo);
	SDL_FreeSurface(Surf_Background);
	SDL_FreeSurface(Surf_Reveal);
	SDL_FreeSurface(Surf_Font);
	SDL_FreeSurface(Surf_Boss);
	SDL_FreeSurface(Surf_Henchman);
	SDL_FreeSurface(Surf_FarmGirl);
}
