// Apricots screen drawing and update
// Author: M.D.Snellgrove
// Date: 26/3/2002
// History:

// Changes by M Harman for Windows version, June 2003:
//   Changes for graphics and font related stuff.

// Changes by M Snellgrove 8/7/2003
//    Player 2 playfield fixed

// Changes by M Snellgrove 15/7/2003
//    SDLfont class used for fonts

#include "apricots.h"
#include "menu.h"

// Draw the blitted images on the screen
extern SDL_Surface *ScreenSurface;
void drawblits(gamedata &g, int rx, int ry){

  // Draw radars
  g.radar.reset();
  while (g.radar.next()){
    g.images[g.radar().image].blit(g.virtualscreen,g.radar().x-rx,g.radar().y-ry);
  }

  // Draw guns
  g.gun.reset();
  while (g.gun.next()){
    g.images[162+g.gun().d].blit(g.virtualscreen,g.gun().x-rx,g.gun().y-ry);
    g.images[171+g.gun().ammo].blit(g.virtualscreen,g.gun().x-rx,g.gun().y-ry);
  }

  // Draw planes
  g.p.reset();
  while (g.p.next()){
    if ((g.p().state < 3) && (!g.p().hide)){
      g.images[g.p().image+g.p().d].blit(g.virtualscreen,
                                         (int)g.p().x-rx,(int)g.p().y-ry);
      if (g.p().boost){
        g.images[145+g.p().d].blit(g.virtualscreen,(int)g.p().x+g.xboost[g.p().d]-rx,
                                   (int)g.p().y+g.yboost[g.p().d]-ry);
      }
    }
  }

  // Draw drak mothership & guns
  if ((g.drak) && (g.drakms.exist == 1)){
    g.images[318].blit(g.virtualscreen,(int)g.drakms.x-rx,(int)g.drakms.y-ry);
    g.drakgun.reset();
    while (g.drakgun.next()){
      g.images[g.drakgun().image[g.drakgun().d]].blit(g.virtualscreen,
                                                   (int)g.drakms.x+g.drakgun().x-rx,
                                                   (int)g.drakms.y+g.drakgun().y-ry);
    }
  }

  // Draw falling things
  g.fall.reset();
  while (g.fall.next()){
    g.images[g.fall().image].blit(g.virtualscreen,
                                  (int)g.fall().x-rx,(int)g.fall().y-ry);
  }

  // Draw shots
  g.shot.reset();
  while (g.shot.next()){
    g.images[114].blit(g.virtualscreen,(int)g.shot().x-rx,(int)g.shot().y-ry);
  }

  // Draw laser
  g.laser.reset();
  while (g.laser.next()){
    g.images[g.laser().image].blit(g.virtualscreen,g.laser().x-rx,g.laser().y-ry);
  }

  // Draw flames
  g.flame.reset();
  while (g.flame.next()){
    g.images[94+int(g.flame().time/5*GAME_SPEED)%3]
             .blit(g.virtualscreen,g.flame().x-rx,g.flame().y-ry);
  }

  // Draw smoke
  g.smoke.reset();
  while (g.smoke.next()){
    g.images[97+int(g.smoke().time/7*GAME_SPEED)]
             .blit(g.virtualscreen,g.smoke().x-rx,(int)g.smoke().y-ry);
  }

  // Draw explosions
  g.explosion.reset();
  while (g.explosion.next()){
    switch(g.explosion().type){
      case 0:
        g.images[101+int(g.explosion().time/2*GAME_SPEED)]
             .blit(g.virtualscreen,g.explosion().x-rx,g.explosion().y-ry);
        break;
      case 1:
        g.images[247+int(g.explosion().time/2*GAME_SPEED)]
             .blit(g.virtualscreen,g.explosion().x-rx,g.explosion().y-ry);
        break;
      case 2:
        g.images[231+int(g.explosion().time*GAME_SPEED)]
             .blit(g.virtualscreen,g.explosion().x-rx,g.explosion().y-ry);
        break;
      case 3:
        g.images[204+int(g.explosion().time/2*GAME_SPEED)]
             .blit(g.virtualscreen,g.explosion().x-rx,g.explosion().y-ry);
        break;
      case 4:
        g.images[214+int(g.explosion().time*GAME_SPEED)]
             .blit(g.virtualscreen,g.explosion().x-rx,g.explosion().y-ry);
        break;
    }
  }

}

// Display the scores

void display_score(SDL_Surface *virtualscreen, plane &p, int x, int y, int mission,
                   int targetscore, SDLfont &whitefont, SDLfont &greenfont){

  if ((mission == 0) || (mission == 1)){
    char scoretitlestring[] = "Score:";
    whitefont.write(virtualscreen, x, y, scoretitlestring);
    char scorestring[] = " xxxx";
    int score = p.score;

    if (score < 0){
      scorestring[0] = '-';
      score = -score;
    }
    int thousands = score / 1000;
    int hundreds = (score % 1000) / 100;
    int tens = (score % 100) / 10;
    int units = score % 10;
    scorestring[1] = '0' + thousands;
    scorestring[2] = '0' + hundreds;
    scorestring[3] = '0' + tens;
    scorestring[4] = '0' + units;
    if (p.score >= targetscore){
      greenfont.write(virtualscreen, x+56, y, scorestring);
    }else{
      whitefont.write(virtualscreen, x+56, y, scorestring);
    }
  }else{
    char targettitlestring[] = "Targets:";
    whitefont.write(virtualscreen, x, y, targettitlestring);
    char targetstring[] = "xx";
    int tens = p.targetscore / 10;
    int units = p.targetscore % 10;
    targetstring[0] = '0' + tens;
    targetstring[1] = '0' + units;
    if (p.targetscore == 0){
      greenfont.write(virtualscreen, x+72, y, targetstring);
    }else{
      whitefont.write(virtualscreen, x+72, y, targetstring);
    }
  }  

}

// Display the players' info

void display_playinfo(SDL_Surface *virtualscreen, int player, plane &p, int y,
                      shape images[319], int mission, int targetscore,
                      SDLfont &whitefont, SDLfont &greenfont, gamedata &g){

  // Display player
  SDL_Rect rect;

  if(g.players == 1){
	  rect.x = 0;
	  rect.y = y;
	  rect.w = 16;
	  rect.h = 16;
	  SDL_FillRect(virtualscreen,&rect,16);
	  images[p.image+13].blit(virtualscreen,0,y);
  }
  else{
/*
	  rect.x = 0;
	  rect.y = y - 5;
	  rect.w = 16;
	  rect.h = 16;
	  SDL_FillRect(virtualscreen,&rect,16);
	  images[p.image+13].blit(virtualscreen,0,y - 5);
*/
  }
  char playerstring[] = "Player x";
  playerstring[7] = '0' + player;
  whitefont.write(virtualscreen, 24, y, playerstring);
  // Display score
  display_score(virtualscreen, p, 112, y, mission, targetscore, whitefont, greenfont);
  if(g.players == 1){
	  // Display shots
	  images[114].blit(virtualscreen,246 - 24,y+6);
	  char shotstring[] = "xx";
	  shotstring[0] = '0' + (p.ammo / 10);
	  shotstring[1] = '0' + (p.ammo % 10);
	  whitefont.write(virtualscreen, 256 - 24, y + 4, shotstring);
	  // Display bombs
	  images[119].blit(virtualscreen,285 - 24,y+5);
	  char bombstring[] = "xx";
	  bombstring[0] = '0' + (p.bombs / 10);
	  bombstring[1] = '0' + (p.bombs % 10);
	  whitefont.write(virtualscreen, 296 - 24, y + 4, bombstring);
  }
  else{
	  // Display shots
	  images[114].blit(virtualscreen,246 - 24,y + 2);
	  char shotstring[] = "xx";
	  shotstring[0] = '0' + (p.ammo / 10);
	  shotstring[1] = '0' + (p.ammo % 10);
	  whitefont.write(virtualscreen, 256 - 24, y, shotstring);
	  // Display bombs
	  images[119].blit(virtualscreen,285 - 24,y + 1);
	  char bombstring[] = "xx";
	  bombstring[0] = '0' + (p.bombs / 10);
	  bombstring[1] = '0' + (p.bombs % 10);
	  whitefont.write(virtualscreen, 296 - 24, y, bombstring);
  }

}

// Display the whole screen

void drawall(gamedata &g){

  int ypos;
  int ypos2;

  // Set screenheight
  int screenheight = 0;
  if (g.players == 1){
    screenheight = limit(GAME_HEIGHT,GAME_HEIGHT,240); //464);
  }else{
    screenheight = limit(GAME_HEIGHT,GAME_HEIGHT,120);
  }

  if(!g.scoreBarPos) // top
  {
	ypos = 0;
	ypos2 = 8;
  }
  else // bottom
  {
	ypos = screenheight - 16;
	ypos2 = screenheight - 8;
  }

  // Player 1
//  int x1 = limit(int(g.player1->x)-308, 0, GAME_WIDTH-320);
  int x1 = limit(int(g.player1->x)-160, 0, GAME_WIDTH-320);
  int y1 = limit(int(g.player1->y)-54, 0, GAME_HEIGHT-screenheight);
  SDL_Rect srcrect;
    srcrect.x = x1;
    srcrect.y = y1;
    srcrect.w = 320;
    srcrect.h = screenheight;
  SDL_BlitSurface(g.gamescreen, &srcrect, g.virtualscreen, NULL);
  SDL_Rect cliprect;
    cliprect.x = 0;
    cliprect.y = 0;
    cliprect.w = 320;
    cliprect.h = screenheight;
  SDL_SetClipRect(g.virtualscreen, &cliprect);
  drawblits(g,x1,y1);
  SDL_SetClipRect(g.virtualscreen, NULL);

  if (g.players == 1){
  if(GameState == STATE_GAME)
//	  display_playinfo(g.virtualscreen, 1, *g.player1, screenheight - 16, g.images, g.mission,
//                   g.targetscore, g.whitefont, g.greenfont, g);
	  display_playinfo(g.virtualscreen, 1, *g.player1, ypos, g.images, g.mission,
                   g.targetscore, g.whitefont, g.greenfont, g);
  }else{
  if(GameState == STATE_GAME)
	  display_playinfo(g.virtualscreen, 1, *g.player1, screenheight - 8, g.images, g.mission,
                   g.targetscore, g.whitefont, g.greenfont, g);
  }

/*
  display_playinfo(g.virtualscreen, 1, *g.player1, screenheight, g.images, g.mission,
                   g.targetscore, g.whitefont, g.greenfont, g);
*/

  // Player 2
  if (g.players == 2){
    int x2 = limit(int(g.player2->x)-160, 0, GAME_WIDTH-320);
    int y2 = limit(int(g.player2->y)-54, 0, GAME_HEIGHT-screenheight);//-208); //224);
    SDL_Rect srcrect;
      srcrect.x = x2;
      srcrect.y = y2;
      srcrect.w = 320;
      srcrect.h = screenheight;
    SDL_Rect desrect;
      desrect.x = 0;
      desrect.y = 120;
      desrect.w = 320;
      desrect.h = screenheight;
    SDL_BlitSurface(g.gamescreen, &srcrect, g.virtualscreen, &desrect);
    SDL_Rect cliprect;
      cliprect.x = 0;
      cliprect.y = 120;
      cliprect.w = 320;
      cliprect.h = screenheight;
    SDL_SetClipRect(g.virtualscreen, &cliprect);
    drawblits(g,x2,y2-120);
    SDL_SetClipRect(g.virtualscreen, NULL);

    if(GameState == STATE_GAME)
  	  display_playinfo(g.virtualscreen, 2, *g.player2, 240 - 8, g.images, g.mission,
                     g.targetscore, g.whitefont, g.greenfont, g);
/*
    display_playinfo(g.virtualscreen, 2, *g.player2, 464, g.images, g.mission,
                     g.targetscore, g.whitefont, g.greenfont, g);
*/
  }

  if(GameState == STATE_INTRO || GameState == STATE_MENU)
  {
	if(CurrentMenu == MenuMainIntro)
		menuDraw(CurrentMenu, 140, 100, g);
	else if(CurrentMenu == MenuMain)
		menuDraw(CurrentMenu, 120, 100, g);
	else if(CurrentMenu == MenuOptions)
		menuDraw(CurrentMenu, 110, 100,g);
	else if(CurrentMenu == MenuGameOptions)
		menuDraw(CurrentMenu, 20, 80, g);
	else
		menuDraw(CurrentMenu, 140, 100, g);
  }
  else if(GameState == STATE_MENU_PLANE_SELECT)
  {
	menuPlanesDraw(20, 10, g);
  }
  else if(GameState == STATE_MENU_PLAYER_CONTROL_SELECT)
  {
	menuSelectPlayerDraw(g);
  }

  // Best AI Score
  if (g.planes > g.players){
    plane bestp;
    bool found = false;
    g.p.reset();
    while (g.p.next()){
      if ((g.p().control == 0) && (g.p().side > 0)){
        if (!found){
          bestp = g.p();
          found = true;
        }else{
          if ((g.mission == 0) || (g.mission == 1)){
            if (g.p().score > bestp.score){
              bestp = g.p();
            }
          }else{
            if (g.p().targetscore < bestp.targetscore){
              bestp = g.p();
            }          
          }
        }
      }
    }
    // Display AI
    if(GameState == STATE_GAME){
	    SDL_Rect rect;
	    char compstring[] = "Plane x";
	    compstring[6] = '0' + bestp.side;

	    if (g.players == 1){
	        rect.x = 304;
	        rect.y = ypos;
	        rect.w = 16;
	        rect.h = 16;
	        SDL_FillRect(g.virtualscreen,&rect,16);
	        g.images[bestp.image+13].blit(g.virtualscreen,304,ypos);
		g.whitefont.write(g.virtualscreen, 24, ypos2, compstring);
	    }
	    else{
		// TODO: display the best player score
	    }
	    // Display score
	    if (g.players == 1){
	    display_score(g.virtualscreen, bestp, 112, ypos2, g.mission, g.targetscore,
		          g.whitefont, g.greenfont);
	    }else{
	    display_score(g.virtualscreen, bestp, 112, screenheight-8, g.mission, g.targetscore,
		          g.whitefont, g.greenfont);
	    }
    }

  }

  // Update screen display
  SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 320;
    rect.h = 240;
  SDL_BlitSurface(g.virtualscreen, &rect, g.physicalscreen, NULL);

  //SDL_Flip(g.physicalscreen);
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
		SDL_Surface *p = SDL_ConvertSurface(g.physicalscreen, ScreenSurface->format, 0);
		SDL_SoftStretch(p, NULL, ScreenSurface, NULL);
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
		SDL_Flip(ScreenSurface);
		SDL_FreeSurface(p);
  //SDL_UpdateRect(g.physicalscreen, 0, 0, 0, 0);

  g.sound.update();

}

