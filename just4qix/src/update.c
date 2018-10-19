/* game world
the game world knows nothing about rendering or bitmaps
*/
#include <SDL/SDL.h>
#include "update.h"
#include "misc.h"
#include "render.h"
#include "particle.h"

#define Maxhenchmen 8

/* typdefs */



Henchman henchmen[Maxhenchmen];
int ActiveHench =0;
int Lvlhenchmen =0;
/* globals */
extern bool Running;
extern int GameState;

float menu_ofx=0;
float menu_ofy=0;

float playerx;
float playery;
float playerdx=0;
float playerdy=0;

float bossx;
float bossy;
float bossdx=0;
float bossdy=0;

float backy;

extern bool CLEFT;
extern bool CRIGHT;
extern bool CDOWN;
extern bool CUP;
extern bool CFIRE;
extern bool CFILL;
extern bool CQUIT;

extern SDL_Surface* Surf_Reveal;
extern SDL_Surface* Surf_Background;
extern SDL_Surface* Surf_Boss;
extern SDL_Surface* Surf_Temp;

extern int playerAnimSpeed;

Uint16 colour_hide;
Uint16 colour_reveal;
Uint16 colour_track;

int percent;
int dispPercent;

int no_menu_items = 3;
int menu_index =0;
int menu_shake=0;
int numPixelFill=0;
int lives;
int level;
char lvlName[30];
int dispLvlNameUntil;
int henchChance;
int invinsibleUntil=0;

Uint16 *pixels;

#define stackSize 200000
int stack[stackSize];
int stackPointer;

bool menu_on = true;



void UpdateStart(){
	int sx,sy,sw,sh;
	
	sw = rand() % ((screen_width-1)/4);
	sh = rand() % ((screen_height-1)/4);
	sx = rand() % ((screen_width-1) - sw);
	sy = rand() % ((screen_height -1)- sh);
	//clear overlay surface
	SDL_FillRect(Surf_Reveal,NULL,colour_hide);
	
	SDL_Rect drect;
	drect.x = sx;
	drect.y = sy;
	drect.w = sw;
	drect.h = sh;
	SDL_FillRect(Surf_Reveal,&drect,colour_track);
	drect.x = sx+1;
	drect.y = sy+1;
	drect.w = sw-2;
	drect.h = sh-2;
	SDL_FillRect(Surf_Reveal,&drect,colour_reveal);
	numPixelFill = (sw-2)*(sh-2);
	playerx=sx;
	playery=sy;
}




void UpdateMenu(){
	
	
	
	
	
		
	static keydelay = 0;
	if (CQUIT) {
		GameState=GAME_EXIT; 
		return;
	}
	if (CFIRE && menu_index == 0) { 
		menu_on = false ; 
		playBlip();
		GameState=GAME_START; 
		return;
	}
	if (CFIRE && menu_index == 2) { 
		playBlip();
		GameState=GAME_EXIT; 
		return;
	}
	if (SDL_GetTicks() > keydelay) {
		if (CDOWN) { menu_index +=1; playBlip();}
		else if (CUP) { menu_index -=1; playBlip();}
		menu_index = (menu_index + 3) % 3;
		keydelay = SDL_GetTicks()+100;
	}
	
	menu_shake = (rand() % 6)-3 ;
}

void put_pixel16( int x, int y, Uint16 pixel ) {
	
	//printf("%d %d \n",x,y);
	//Uint16 *pixels = (Uint16 *)Surf_Reveal->pixels; 
	pixels[ ( y * Surf_Reveal->w ) + x ] = pixel;
} 

Uint16 get_pixel16( int x, int y) {
	//Uint16 *pixels = (Uint16 *)Surf_Reveal->pixels; 
	return pixels[ ( y * Surf_Reveal->w ) + x ];
} 

void IncPercent(int count)
{
	numPixelFill += count;
	float calc;
	int totPixel = 320*240;
	calc = (numPixelFill/ (float)totPixel) * 100; //todo: needs fixing
	percent = (int)calc;
}

/* this routine fills when the player makes a connection with a line
x and y are the current position of the player
dx and dy where the modifications made to get to the current position and therefore indicate the direction
we calculate the two possible fills(reveals) and then unfill the largest
*/
void Fill(int x, int y, int dx, int dy) {
	
	int x1 = (x-dx) - dy;
	int y1 = (y-dy) - dx;
	int x2 = (x-dx) + dy;
	int y2 = (y-dy) + dx;
	
	
	Uint32 count1=0;
	Uint32 count2=0;
	//calculate the two fill points to compare
	
	if (get_pixel16(x1,y1) == colour_hide) { 
		count1 = floodFillScanlineStack( x1, y1, colour_reveal, colour_hide);
	}
	if (get_pixel16(x2,y2) == colour_hide) { 
		count2 = floodFillScanlineStack( x2, y2, colour_reveal, colour_hide);
	}
	//debug:printf("x:%d y:%d x1:%d y1:%d x2:%d y2:%d count1:%d count2:%d \n",x,y,x1,y1,x2,y2,count1, count2);
	//now reverse out the largest flood fill
	if (count1<count2) {
		IncPercent(count1);
		
		if (count2>0) { count2 = floodFillScanlineStack( x2, y2, colour_hide, colour_reveal); 	}
	} else {
		IncPercent(count2);
		
		if (count1>0) { count1 = floodFillScanlineStack( x1, y1, colour_hide, colour_reveal); }
	}
}

//stack routines required for flood fill
bool push(int x, int y) 
{ 
	 int h = 240;
    if(stackPointer < stackSize - 1) 
    { 
        stackPointer++; 
        stack[stackPointer] = h * x + y; 
        return 1; 
    }     
    else 
    { 
        return 0; 
    }    
}     
//stack routine required for flood fill
bool pop (int *x , int *y) 
{ 
    int h = 240;	
    if(stackPointer > 0) 
    { 
        int p = stack[stackPointer]; 
        *x = p / h; 
        *y = p % h; 
        stackPointer--; 
        return 1; 
    }     
    else 
    { 
        return 0; 
    }    
}  

//stack routine required for flood fill
void emptyStack() 
{ 
    int x, y; 
    while(pop(&x, &y)); 
}





//stack routine required for flood fill
void emptyStackDestroy() 
{ 
    int x, y, i; 
    while(pop(&x, &y)) {
	   
	if (ActiveHench <0) { continue; }
	for (i=0;i < ActiveHench; i++)
	{
		if (henchmen[i].x == x && henchmen[i].y==y) {
			deleteHenchman(i);
			playPop();
			CreateParticleExplosion(x, y, 200, 200, 200, 2, 100);
			i--; //because we deleted a henchman we need to back up
		}
	}
    } 
}



//The scanline floodfill algorithm using our own stack routines, faster
Uint32 floodFillScanlineStack(int x, int y, Uint16 newColor, Uint16 oldColor)
{
    Uint32 fillcount=0;
    int w=320;
    int h=240;	
    if(oldColor == newColor) return;
    emptyStack();
    pixels = (Uint16 *)Surf_Reveal->pixels; 
    int y1; 
    bool spanLeft, spanRight;
    
    if(!push(x, y)) return;
    
    while(pop(&x, &y))
    {    
        y1 = y;
        while(y1 >= 0 && get_pixel16(x,y1) == oldColor) y1--;
        ++y1;
        spanLeft = spanRight = 0;
        while(y1 < h && get_pixel16(x,y1) == oldColor )
        {
            put_pixel16(x,y1, newColor);
	     ++fillcount;		
            if(!spanLeft && x > 0 && get_pixel16(x-1,y1) == oldColor) 
            {
                if(!push(x - 1, y1)) return;
                spanLeft = 1;
            }
            else if(spanLeft && x > 0 && get_pixel16(x-1,y1) != oldColor)
            {
                spanLeft = 0;
            }
            if(!spanRight && x < w - 1 && get_pixel16(x+1,y1) == oldColor) 
            {
                if(!push(x + 1, y1)) return;
                spanRight = 1;
            }
            else if(spanRight && x < w - 1 && get_pixel16(x+1,y1) != oldColor)
            {
                spanRight = 0;
            } 
            ++y1;
        }
    }
    return fillcount;
}

bool connected(int x, int y) {
	if (get_pixel16(x,y) == colour_track) { return true;}
	return false;
}

bool colourCheck(int x,int y,Uint16 colour)
{
	if ( y<screen_height && y>=0 && x<screen_width && x>=0 ) {
		return (get_pixel16(x , y) == colour);
	}
	return false;
}


void moveplayer()
{
	if (get_pixel16(playerx + playerdx, playery) ==  colour_track) { // on the line
				playerx += playerdx;
			}
			if (get_pixel16(playerx, playery + playerdy) ==  colour_track) { // on the line
				playery += playerdy;
			}
}

void updatePlayer(){
	
	if (CLEFT && (int)playerx >0 ) { playerdx=-1;}
	else if (CRIGHT && (int)playerx <screen_width-1) { playerdx=1;}
	else { playerdx=0;}
	if (CUP && (int)playery >0) { playerdy=-1;}
	else if (CDOWN && (int)playery <screen_height-1) { playerdy=1;}
	else { playerdy=0;}
	
	if (!CFIRE) {
		playerAnimSpeed=50;
		int x,y;
		while (pop(&x,&y)){ //backup player
			put_pixel16( playerx, playery, colour_hide);
			playerx = x;
			playery = y;
			return;
			//printf("pop %d %d\n",(int)playerx,(int)playery);
		}
		moveplayer();
			
	} else {	//fire pressed
		playerAnimSpeed=15;
		//The following check for player pressing fire and pressing a direction still on the track
		if (colourCheck(playerx+playerdx,playery+playerdy,colour_track) && stackPointer < 2) { //track movement
			moveplayer();
			return;
		}
		//heading off track now
		if (playerdx !=0 ) { //player movement
			push(playerx,playery); //record for backup later
			playerx += playerdx;
			if (connected(playerx,playery) && stackPointer>2 ) {
				IncPercent(stackPointer); 
				emptyStackDestroy(); //clear backup
				CLEFT = CRIGHT = CUP = CDOWN = CFIRE = false; //stop
				playFill();
				Fill(playerx, playery, playerdx,0); //todo: needs fixing
				
			} 
			put_pixel16( playerx, playery, colour_track); //floats to ints
			
			return;  //this return blocks diagonals from being possible because I am too weak and feeble to cope with it
		}
		
		if (playerdy !=0) { //player movement
			push(playerx,playery); //record for backup later
			playery += playerdy;
			if (connected(playerx,playery) && stackPointer>2) {
				IncPercent(stackPointer);
				emptyStackDestroy(); //clear backup
				CLEFT = CRIGHT = CUP = CDOWN = CFIRE = false; //stop
				playFill();
				Fill(playerx, playery, 0, playerdy); //todo: needs fixing
			}
			put_pixel16( playerx, playery, colour_track); //floats to ints
			
		}
		
		
	}
}

void updateLevelOver(){
	static timer=0;
	static float backdy=0.5;
	if ((backy + backdy) < 0 |  (backy + screen_height+ backdy) >  Surf_Background->h) { backdy = -backdy;}
	backy += backdy;
	if (CQUIT) {
		StopMusic();
		PlayIntroMusic();
		GameState = GAME_MENU;  //go back to menu
		CQUIT = false;
		return;
	}
	//percent code
	if (SDL_GetTicks() > timer ) {
		if (dispPercent < percent) {
			++dispPercent;
			if (dispPercent> 89) { 
				++lives; 
				CreateParticleExplosion( rand() % screen_width, rand() % screen_height, rand() % 200 + 50, rand() % 200 + 50, rand() % 200 + 50, 1, 500);	
			}
			playPlayerDeath();
		}
		//printf("percent %d %d\n",dispPercent,percent);
		
		timer = SDL_GetTicks()+1000;
	}
	UpdateParticles();
	
}



void findDirection(Henchman* henchy){
	
   
		
	int possDirectx[5];  //keep going plus 4 directions
	int possDirecty[5];  //keep going plus 4 directions
	int numDirect=-1;
	
	if  (henchy->dy ==0) { //left right
		if ( colourCheck(henchy->x , henchy->y + 1,colour_track)) { ++numDirect;  possDirectx[numDirect] = 0; possDirecty[numDirect] =1; }
		if ( colourCheck(henchy->x , henchy->y - 1,colour_track)) { ++numDirect;  possDirectx[numDirect] =0; possDirecty[numDirect] = -1; }	
	}
	if ( henchy->dx ==0) {
		if ( colourCheck(henchy->x + 1, henchy->y,  colour_track)) { ++numDirect;  possDirectx[numDirect] = 1; possDirecty[numDirect] = 0; }
		if ( colourCheck(henchy->x - 1, henchy->y,  colour_track)) { ++numDirect;  possDirectx[numDirect] = -1; possDirecty[numDirect] = 0; }	
	}
	if ( colourCheck(henchy->x + henchy->dx, henchy->y + henchy->dy, colour_track )) { ++numDirect;  possDirectx[numDirect] = henchy->dx; possDirecty[numDirect] = henchy->dy; }  //keep going
	if (numDirect > -1)	{	
		//follow player if possible
		if(stackPointer > 1) 
		{ 
			int h = 240;
			int p = stack[2]; 
			int x = p / h; 
			int y = p % h; 
			int i;
			for (i=0; i<=numDirect; i++)
			{
				if ( henchy->x + possDirectx[i] == x && henchy->y + possDirecty[i] == y) {
					henchy->dx =  possDirectx[i];
					henchy->dy = 	 possDirecty[i];	
					return;
				}
			}
		}
		
		int choice = rand() % (numDirect + 1);
		//printf("%d",choice);	
		henchy->dx =  possDirectx[choice];
		henchy->dy = 	 possDirecty[choice];	
	} else { //crap back it out quick
		henchy->dx =  -henchy->dx;
		henchy->dy = 	-henchy->dy;	
	}
}

void addHenchman(int x, int y){
	if (ActiveHench < Lvlhenchmen ) {
		Henchman henchy;
		henchy.x = x;
		henchy.y = y;
		henchy.dx=0;
		henchy.dy=0;
		findDirection(&henchy);
		henchmen[ActiveHench] = henchy;
		++ActiveHench;
		//printf("active hench %d\n",ActiveHench);
		CreateParticleExplosion(x, y, 200, 50, 50, 1, 100);
		playWhirly();
	}
}

static void deleteHenchman(int index)
{
    /* Replace the particle with the one at the end
       of the list, and shorten the list. */
    ActiveHench--;
    henchmen[index] = henchmen[ActiveHench];
    //printf("deleted hench\n");
}




void updateHenchmen()
{
	if (ActiveHench <=0) { return; }
	int i,j;
	for (j=0; j<2;j++){ //move twice as fast
		for (i=0;i < ActiveHench; i++)
		{
			if (henchmen[i].x == playerx && henchmen[i].y==playery && invinsibleUntil==0) { //colision with player
				if (lives > 0) { lives--; 
					invinsibleUntil = SDL_GetTicks()+3000;
					playPlayerDeath();
				}
				deleteHenchman(i);
				playPop();
				CreateParticleExplosion(playerx, playery, 200, 200, 200, 2, 100);
				i--;  // backup we just deleted one
			} else { //normal move
			
				findDirection(&henchmen[i]);
				henchmen[i].x = henchmen[i].x + henchmen[i].dx;
				henchmen[i].y = henchmen[i].y + henchmen[i].dy;
			}
		}
	}
}


void updateBoss() {
	
	if ( (get_pixel16(bossx,bossy) == colour_track) && ( rand() % 100 < henchChance  ) ){
		addHenchman(bossx,bossy);
		
	}
	
	//check for boss hitting screen limits
	if (bossx + bossdx > screen_width | bossx + bossdx < 0 ) { bossdx = -bossdx;}
	if (bossy + bossdy > screen_height |bossy + bossdy < 0 ) { bossdy = -bossdy;}
	
	bossx += bossdx;
	bossy += bossdy;
}

void update( ) { //main update loop
	
	static quartersecTimer =0;
	if (CQUIT) { GameState = GAME_MENU; CQUIT=false; StopMusic(); PlayIntroMusic();return;}	
	updatePlayer();
	updateBoss();
	updateHenchmen();
	UpdateParticles();
	if (lives == 0)
	{
		GameState = GAME_END;
		return;
	}
	if (dispPercent > 80 ) { 
			
		GameState = LEVEL_END; 
		return;
	}
	int timenow = SDL_GetTicks();
	if (timenow > quartersecTimer) {
		if (dispPercent < percent) { ++dispPercent;}
		quartersecTimer = SDL_GetTicks() + 250;
	}
	
	if (invinsibleUntil<timenow) { invinsibleUntil =0; }
}

void Init_Game(){
	lives=3;
	level=0;
	//define colours used throughout the game
	colour_hide = SDL_MapRGB(Surf_Reveal->format, 0, 0, 0);
	colour_reveal = SDL_MapRGB(Surf_Reveal->format, 255, 255, 255);
	colour_track = SDL_MapRGB(Surf_Reveal->format, 255, 0, 0);
	
	//create start position and initial fill
	pixels = (Uint16 *)Surf_Reveal->pixels;
	
}


void Next_Level(){
	level ++;
	if (level>10) { level=1; } //wrap levels for now
	//load background for level
	char filename[30];
	sprintf(filename,"gfx/b/background%d.bmp",level);
	Surf_Temp= Load(filename);
	Surf_Background = SDL_DisplayFormat(Surf_Temp);
	SDL_FreeSurface(Surf_Temp);
	
	//reset variables for the level
	ActiveHench =0;
	numPixelFill=0;
	percent=0;
	dispPercent=0;
	emptyStack() ;	
	invinsibleUntil = SDL_GetTicks()+5000;
	//initial boss position
	bossx=0;bossy=0;
	bossdx = (rand() % 20) / 10 + 0.5;
	bossdy = (rand() % 20) / 10 + 0.5;
	backy=0;//background scroll reset
	switch (level) {
	case 1:
		sprintf(lvlName,"%s","WHO AM I?");	
		Lvlhenchmen=1;
		henchChance=25;
		break;
	case 2:
		sprintf(lvlName,"%s","ONE WITHOUT SECOND");	
		Lvlhenchmen=2;
		henchChance=50;
		break;
	case 3:
		sprintf(lvlName,"%s","IMAGINED SEPERATION");	
		Lvlhenchmen=3;
		henchChance=25;
		break;
	case 4:
		sprintf(lvlName,"%s","UNBORN");	
		Lvlhenchmen=8;
		henchChance=10;
		break;
	case 5:
		sprintf(lvlName,"%s","NAMELESS");	
		Lvlhenchmen=5;
		henchChance=50;
		break;
	case 6:
		sprintf(lvlName,"%s","LOVE");	
		Lvlhenchmen=4;
		henchChance=25;
		break;
	case 7:
		sprintf(lvlName,"%s","NOBODY AND NO BODY");	
		Lvlhenchmen=5;
		henchChance=25;
		break;
	case 8:
		sprintf(lvlName,"%s","WHAT YOU ARE, I AM");	
		Lvlhenchmen=6;
		henchChance=25;
		break;
	case 9:
		sprintf(lvlName,"%s","KNOW WHAT YOU'RE NOT");	
		Lvlhenchmen=7;
		henchChance=50;
		break;
	case 10:
		sprintf(lvlName,"%s","WALK THE PATH");	
		Lvlhenchmen=8;
		henchChance=50;
		break;
	
	default:
		sprintf(lvlName,"%s","DEFAULT");	
		Lvlhenchmen=1;
		break;
	}
	
	
}
