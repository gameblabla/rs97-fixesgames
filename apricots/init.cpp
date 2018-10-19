// Apricots init routine
// Author: M.D.Snellgrove
// Date: 26/3/2002
// History:

// Changes by M Harman for Windows version, June 2003:
//   Changes for graphics and font related stuff.
//   Need to open "apricots.shapes" file in ios::binary mode.

// Changes by M Snellgrove 8/7/2003
//   Palette bluescale redefined (in SDL this appears to be 5 bit, not 6 bit)

// Changes by M Snellgrove 13/7/2003
//   Check for existence of apricots.shapes file

// Changes by Judebert 1/8/2003
//   Some configuration now read from apricots.cfg

// Changes by M Snellgrove 8/8/2003
//   Cursor hidden
//   All configuration now read from apricots.cfg, with error checking

#include "apricots.h"
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

// Display setup (double buffered with two playfields)
SDL_Surface *ScreenSurface;
void setup_display(gamedata &g){
  //g.physicalscreen = SDL_SetVideoMode(320, 240, 8, SDL_HWSURFACE|SDL_DOUBLEBUF);
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_SWSURFACE);
  g.physicalscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 8, 0,0,0,0);
/*
  g.physicalscreen = SDL_SetVideoMode(640, 480, 8, 
                              SDL_HWSURFACE|SDL_HWPALETTE|SDL_HWACCEL);
*/
  if (g.physicalscreen == NULL){
    fprintf(stderr, "Couldn't set 640x480x8 physical video mode: %s\n",SDL_GetError());
    exit(1);
  }

  //g.virtualscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 8,
  g.virtualscreen = SDL_CreateRGBSurface(SDL_HWSURFACE, 320, 240, 8,
                                         0, 0, 0, 0);
  if (g.virtualscreen == NULL){
    fprintf(stderr, "Couldn't set 640x480x8 virtual video mode: %s\n",SDL_GetError());
    exit(1);
  }

  //g.gamescreen = SDL_CreateRGBSurface(SDL_SWSURFACE, GAME_WIDTH, GAME_HEIGHT, 8,
  g.gamescreen = SDL_CreateRGBSurface(SDL_HWSURFACE, GAME_WIDTH, GAME_HEIGHT, 8,
                                      0, 0, 0, 0);
  if (g.gamescreen == NULL){
    fprintf(stderr, "Couldn't set game video mode: %s\n",SDL_GetError());
    exit(1);
  }
}

// Load font and set font colours

void load_font(SDL_Surface *screen, SDLfont &whitefont, SDLfont &greenfont){

  char filename[255];
  strcpy(filename,AP_PATH);
  strcat(filename,"alt-8x8.psf");
  whitefont.loadpsf(filename, 8, 8);
  whitefont.colour(screen, 1, 0);
  greenfont = whitefont;
  greenfont.colour(screen, 13, 0);
  
}

// Load shapes and set palette

void load_shapes(gamedata &g,shape images[]){

  char filename[255];
  strcpy(filename,AP_PATH);
  strcat(filename,"apricots.shapes");
  ifstream fin(filename, ios::binary);
  if(fin.fail()){
    fprintf(stderr, "Could not open file: %s\n", filename);
    exit(-1);
  }  

  {
    char dummy[14];
    fin.read(dummy, 14);
    int dummyb;
    fin >> dummyb;
    int red, green, blue;
    for( int c1=0;c1<256;c1++) {
      SDL_Color col = { 0, 0, 0, 0 };
      SDL_SetColors(g.physicalscreen, &col, c1, 1);
    }
    for (int c2=0;c2<16;c2++){
      fin >> red >> green >> blue;
      SDL_Color col = { red * 4, green * 4, blue * 4, 0 };
      SDL_SetColors(g.physicalscreen, &col, c2, 1);
    }
    for (int c3=0;c3<25;c3++){
      int green = ((2*c3) % 64) * 4;
      int blue = ((15+2*c3) % 64) * 4;
      SDL_Color col = { 0, green, blue, 0 };
      SDL_SetColors(g.physicalscreen, &col, c3+16, 1);
    }
    for (int c4=0;c4<256;c4++){
      Uint8 rgb[3];
      SDL_GetRGB(c4, g.physicalscreen->format, &rgb[0], &rgb[1], &rgb[2]);
      SDL_Color col = { rgb[0], rgb[1], rgb[2], 0 };
      SDL_SetColors(g.virtualscreen, &col, c4, 1);
      SDL_SetColors(g.gamescreen, &col, c4, 1);
    }
    fin.read(dummy, 1);
  }
  for (int i=1;i<=34;i++)
    images[i].read(fin);
  for (int j=69;j<=318;j++)
    images[j].read(fin);

  for (int c=0;c<256;c++){
    Uint8 rgb[3];
    SDL_GetRGB(c, g.physicalscreen->format, &rgb[0], &rgb[1], &rgb[2]);
    SDL_Color col = { rgb[0], rgb[1], rgb[2], 0 };
    for (int i=0;i<=318;i++){
      if (images[i].getSurface() != NULL)
        SDL_SetColors(images[i].getSurface(), &col, c, 1);
    }
  }

  fin.close();

}

// Sound initialization

void init_sound(sampleio &sound){

  char filenames[14][255];
  for (int i=0;i<14;i++){
    strcpy(filenames[i],AP_PATH);
  } 
  strcat(filenames[0],"engine.wav");
  strcat(filenames[1],"jet.wav");
  strcat(filenames[2],"explode.wav");
  strcat(filenames[3],"groundhit.wav");
  strcat(filenames[4],"fuelexplode.wav");
  strcat(filenames[5],"shot.wav");
  strcat(filenames[6],"gunshot.wav");
  strcat(filenames[7],"bomb.wav");
  strcat(filenames[8],"splash.wav");
  strcat(filenames[9],"laser.wav");
  strcat(filenames[10],"stall.wav");
  strcat(filenames[11],"gunshot2.wav");
  strcat(filenames[12],"afterburner.wav");
  strcat(filenames[13],"finish.wav");

  sound.init(14, filenames, 2, 6);

}

// Initialize the game constants

void init_gameconstants(gamedata &g){

// Initialize sin/cos arrays and afterburner co-ordinates
  for (int i=1;i<=16;i++){
    g.xmove[i] = -sin((i-1)*PI/8.0);
    g.ymove[i] = -cos((i-1)*PI/8.0);
    g.xboost[i] = 6 - int(9 * g.xmove[i]);
    g.yboost[i] = 6 - int(10 * g.ymove[i]);
  }
// Corrections
  g.ymove[5] = 0.0;
  g.ymove[13] = 0.0;
  g.xmove[1] = 0.0;
  g.xmove[9] = 0.0;
  g.xboost[1] = 5;
  g.xboost[5] = 17;
  g.yboost[5] = 7;
  g.xboost[13] = -5;
  g.yboost[13] = 7;
// Initialize flight data
  g.accel[1] = -0.18;
  g.accel[2] = -0.12;
  g.accel[3] = -0.04;
  g.accel[4] = -0.005;
  g.accel[5] = 0.0;
  g.accel[6] = 0.02;
  g.accel[7] = 0.05;
  g.accel[8] = 0.15;
  g.accel[9] = 0.2;
  g.accel[10] = 0.15;
  g.accel[11] = 0.05;
  g.accel[12] = 0.02;
  g.accel[13] = 0.0;
  g.accel[14] = -0.005;
  g.accel[15] = -0.04;
  g.accel[16] = -0.12;
// Initialize the bomb images
  g.bombimage[1] = 115;
  g.bombimage[2] = 122;
  g.bombimage[3] = 122;
  g.bombimage[4] = 122;
  g.bombimage[5] = 121;
  g.bombimage[6] = 120;
  g.bombimage[7] = 120;
  g.bombimage[8] = 120;
  g.bombimage[9] = 119;
  g.bombimage[10] = 118;
  g.bombimage[11] = 118;
  g.bombimage[12] = 118;
  g.bombimage[13] = 117;
  g.bombimage[14] = 116;
  g.bombimage[15] = 116;
  g.bombimage[16] = 116;

}

char *getHomeDir(char *home)
{
	if(home != NULL)
	{
		free(home);
	}

	home = (char *)malloc(strlen(getenv("HOME")) + strlen("/.apricots/") + 1);
	strcpy(home, getenv("HOME"));
	strcat(home, "/.apricots/");
	mkdir(home, 0755); // create $HOME/.apricots if it doesn't exist
	printf("home: %s\n", home);

	return home;
}

//--JAM: Gets a line from the config string with format:
//  NAME:VALUE\n
// and returns it as a string.  If there is no line for the
// name, returns the default.

string getConfig(string config, string name, string defval)
{
  // Pull out just the name line
  unsigned int ndx = config.find(name);
  
  if (ndx == string::npos)
  {
    return defval;
  }
  ndx = config.find(":", ndx);

  // Advance past spaces
  while (config.at(++ndx) == ' '){};
  if (ndx == string::npos)
  {
    return defval;
  }

  return config.substr(ndx, config.find("\n", ndx) - ndx);
}

// Similar to above, but this returns integers, and includes error checking.

int getConfig(string config, string name, int defval, int min, int max)
{
  // Pull out just the name line
  unsigned int ndx = config.find(name);
  
  if (ndx == string::npos)
  {
    return defval;
  }
  ndx = config.find(":", ndx);

  // Advance past spaces
  while (config.at(++ndx) == ' '){};
  if (ndx == string::npos)
  {
    return defval;
  }

  int value = strtol(config.substr(ndx, config.find("\n", ndx) - ndx).c_str(),0,10);

  // Bounds checking
  if ((value < min) || (value > max)){
    cerr << "Entry " << name.c_str() << " out of bounds in apricots.cfg" << endl;
    cerr << name.c_str() << " must take values between " << min << " and " << max << endl;
    exit(EXIT_FAILURE);
  }

  return value;

}

int saveConfig(const char *filePath, const char *fileName, gamedata &g)
{
	FILE *ofp;
	char *newFileName = NULL;

	newFileName = (char *)malloc(strlen(filePath) + strlen(fileName) + 1);
	if(newFileName == NULL)
	{
		fprintf(stderr, "Out of memory\n");
		return 1;
	}
	strcpy(newFileName, filePath);
	strcat(newFileName, fileName);

	ofp = fopen(newFileName, "w");
	if(ofp == NULL)
	{
		fprintf(stderr, "ERROR: (saveConfig) Can't open %s\n", newFileName);
		free(newFileName);
		return 1;
	}

	printf("Saving config\n");
	fprintf(ofp, "# Apricots configuration file");
	fprintf(ofp, "\n");
	fprintf(ofp, "# The total number of planes (humans and computer controlled)");
	fprintf(ofp, "# Can't be more than 6\n");
	fprintf(ofp, "NUM_PLANES: %d\n", g.planes);
	fprintf(ofp, "\n");
	fprintf(ofp, "# The total number of human players (1 or 2)\n");
	fprintf(ofp, "NUM_HUMANS: %d\n", g.players);
	fprintf(ofp, "\n");
	fprintf(ofp, "# The goal of the game\n");
	fprintf(ofp, "# Mission 0 : Reach the target score and get back to base\n");
	fprintf(ofp, "# Mission 1 : Same as above, but dying reduces score to 200\n");
	fprintf(ofp, "#             less than the target, so landing is necessary\n");
	fprintf(ofp, "# Mission 2 : Destroy all enemy airbases\n");
	fprintf(ofp, "MISSION: %d\n", g.mission);
	fprintf(ofp, "\n");
	fprintf(ofp, "# The score that must be reached to win in missions 0 and 1\n");
	fprintf(ofp, "TARGET_SCORE: %d\n", g.targetscore);
	fprintf(ofp, "\n");
	fprintf(ofp, "# Plane types (1=Spitfire, 2=Jet, 3=Stealth Bomber)\n");
	fprintf(ofp, "PLANE1: %d\n", g.planeinfo[1].planetype);
	fprintf(ofp, "PLANE2: %d\n", g.planeinfo[2].planetype);
	fprintf(ofp, "PLANE3: %d\n", g.planeinfo[3].planetype);
	fprintf(ofp, "PLANE4: %d\n", g.planeinfo[4].planetype);
	fprintf(ofp, "PLANE5: %d\n", g.planeinfo[5].planetype);
	fprintf(ofp, "PLANE6: %d\n", g.planeinfo[6].planetype);
	fprintf(ofp, "\n");
	fprintf(ofp, "# Base types (1=Standard, 2=Reversed, 3=Little, 4=Long\n");
	fprintf(ofp, "#             5=Original, 6=Shooty, 7=Twogun)\n");
	fprintf(ofp, "BASE1: %d\n", g.planeinfo[1].basetype);
	fprintf(ofp, "BASE2: %d\n", g.planeinfo[2].basetype);
	fprintf(ofp, "BASE3: %d\n", g.planeinfo[3].basetype);
	fprintf(ofp, "BASE4: %d\n", g.planeinfo[4].basetype);
	fprintf(ofp, "BASE5: %d\n", g.planeinfo[5].basetype);
	fprintf(ofp, "BASE6: %d\n", g.planeinfo[6].basetype);
	fprintf(ofp, "\n");
	fprintf(ofp, "# Controls (1=Player 1, 2=Player 2, 0=Computer AI)\n");
	fprintf(ofp, "CONTROL1: %d\n", g.planeinfo[1].control);
	fprintf(ofp, "CONTROL2: %d\n", g.planeinfo[2].control);
	fprintf(ofp, "CONTROL3: %d\n", g.planeinfo[3].control);
	fprintf(ofp, "CONTROL4: %d\n", g.planeinfo[4].control);
	fprintf(ofp, "CONTROL5: %d\n", g.planeinfo[5].control);
	fprintf(ofp, "CONTROL6: %d\n", g.planeinfo[6].control);
	fprintf(ofp, "\n");
	fprintf(ofp, "# Number of towerblocks\n");
	fprintf(ofp, "NUM_TOWERS: %d\n", g.towers);
	fprintf(ofp, "\n");
	fprintf(ofp, "# Number of neutral anti-aircraft guns\n");
	fprintf(ofp, "NUM_GUNS: %d\n", g.guns);
	fprintf(ofp, "\n");
	fprintf(ofp, "# Number of buildings\n");
	fprintf(ofp, "NUM_BUILDINGS: %d\n", g.buildings);
	fprintf(ofp, "\n");
	fprintf(ofp, "# Maximum number of trees\n");
	fprintf(ofp, "NUM_TREES: %d\n", g.trees);
	fprintf(ofp, "\n");
	fprintf(ofp, "# Whether or not the Drak show up.\n");
	fprintf(ofp, "# \"always\" will ensure the Drak appear\n");
	fprintf(ofp, "# \"sometimes\" means Drak appear 5%% of the time\n");
	fprintf(ofp, "# \"never\" (or anything else) means they never appear.\n");
	fprintf(ofp, "\n");
	if(g.drakoption == 2)
		fprintf(ofp, "DRAK: always\n");
	else if(g.drakoption == 1)
		fprintf(ofp, "DRAK: sometimes\n");
	else
		fprintf(ofp, "DRAK: never\n");
	fprintf(ofp, "\n");
	fprintf(ofp, "# Score Bar placement (0=Top, 1=Bottom)\n");
	fprintf(ofp, "SCOREBAR_POS: %d\n", g.scoreBarPos);
	

	fclose(ofp);
	free(newFileName);

	return 0;
}

// Initialize the game parameters
// Edit the values here to set game options

void init_gamedata(gamedata &g){

  //--JAM: Read from config file
  ap_home = getHomeDir(ap_home);
  string filename((string)ap_home);
  filename += "apricots.cfg";
  printf("Filename: %s\n", filename.c_str());
  ifstream config_stream(filename.c_str());
  string config;
  if (!config_stream.fail()){
    // Read config file line by line
    char line[256];
    while (!config_stream.eof()){
      config_stream.getline(line, 255);
      if (line[0] != '#'){
        config.append(line);
        config += "\n";
      }
    }
  }else{
  // Config file not found
    config = "\n";
  }
  config_stream.close();

  // Number of planes (1-6)
  g.planes = getConfig(config, "NUM_PLANES", 2, 1, 6);
  
  // Number of players (1 or 2)
  g.players = getConfig(config, "NUM_HUMANS", 1, 1, 2);
  // Error check
  if (g.players > g.planes){
    cerr << "Invalid configuration in apricots.cfg" << endl;
    cerr << "Number of human players cannot exceed number of planes" << endl;
    exit(EXIT_FAILURE);
  }

  // Mission
  // 0/1 means winner is first to reach targetscore and reach airbase
  // 1 makes score reduce to targetscore-200 upon dying (so must land)
  // 2 means winner is first to destroy all enemy airbases and reach airbase
  g.mission = getConfig(config, "MISSION", 0, 0, 2);

  // Targetscore for missions 0/1
  g.targetscore = getConfig(config, "TARGET_SCORE", 1400, 100, 5000);

  // Planetypes: 1=Spitfire, 2=Jet, 3=Stealth Bomber
  g.planeinfo[1].planetype = getConfig(config, "PLANE1", 1, 1, 3);
  g.planeinfo[2].planetype = getConfig(config, "PLANE2", 1, 1, 3);
  g.planeinfo[3].planetype = getConfig(config, "PLANE3", 1, 1, 3);
  g.planeinfo[4].planetype = getConfig(config, "PLANE4", 1, 1, 3);
  g.planeinfo[5].planetype = getConfig(config, "PLANE5", 1, 1, 3);
  g.planeinfo[6].planetype = getConfig(config, "PLANE6", 1, 1, 3);


  // Basetype: See create_airbases in setup.cpp
  g.planeinfo[1].basetype = getConfig(config, "BASE1", 1, 1, 7);
  g.planeinfo[2].basetype = getConfig(config, "BASE2", 1, 1, 7);
  g.planeinfo[3].basetype = getConfig(config, "BASE3", 1, 1, 7);
  g.planeinfo[4].basetype = getConfig(config, "BASE4", 1, 1, 7);
  g.planeinfo[5].basetype = getConfig(config, "BASE5", 1, 1, 7);
  g.planeinfo[6].basetype = getConfig(config, "BASE6", 1, 1, 7);


  // Control: 1=Player 1, 2=Player 2, 0=AI
  g.planeinfo[1].control = getConfig(config, "CONTROL1", 1, 0, 2);
  g.planeinfo[2].control = getConfig(config, "CONTROL2", 0, 0, 2);
  g.planeinfo[3].control = getConfig(config, "CONTROL3", 0, 0, 2);
  g.planeinfo[4].control = getConfig(config, "CONTROL4", 0, 0, 2);
  g.planeinfo[5].control = getConfig(config, "CONTROL5", 0, 0, 2);
  g.planeinfo[6].control = getConfig(config, "CONTROL6", 0, 0, 2);
  // Error check
  int count[3];
  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  for (int i=1; i<=g.planes; i++){
  count[g.planeinfo[i].control]++;
  }
  if (count[1] != 1){
    cerr << "Invalid configuration in apricots.cfg" << endl;
    cerr << "Invalid control selection for player 1" << endl;
    exit(EXIT_FAILURE);
  }
  if (count[2] != (g.players-1)){
    cerr << "Invalid configuration in apricots.cfg" << endl;
    cerr << "Invalid control selection for player 2" << endl;
    exit(EXIT_FAILURE);
  }
  for (int j=g.planes+1;j<=6;j++){
  if (g.planeinfo[j].control > 0){
      cerr << "Invalid configuration in apricots.cfg" << endl;
      cerr << "Human controls specified for non-playing plane" << endl;
      exit(EXIT_FAILURE);
    }
  }
  // Number of towerblocks
  g.towers = getConfig(config, "NUM_TOWERS", 5, 0, 30);

  // Number of neutral anti-aircraft guns
  g.guns = getConfig(config, "NUM_GUNS", 5, 0, 20);
  
  // Number of other buildings
  g.buildings = getConfig(config, "NUM_BUILDINGS", 20, 0 ,50);
  
  // Number of trees (max)
  g.trees = getConfig(config, "NUM_TREES", 50, 0, 100); 

  // Draks: 0=Never, 1=5% probability, 2=Always
  string drakval = getConfig(config, "DRAK", "sometimes");
  if (!drakval.compare("always"))
  {
    g.drakoption = 2;
  }
  else if (!drakval.compare("sometimes"))
  {
    g.drakoption = 1;
  }
  else
  {
    g.drakoption = 0;
  }

  g.playerJoy[0] = -1;
  g.playerJoy[1] = -1;
  g.playerJoyBut[0][0] = -1;
  g.playerJoyBut[0][1] = -1;
  g.playerJoyBut[0][2] = -1;
  g.playerJoyBut[1][0] = -1;
  g.playerJoyBut[1][1] = -1;
  g.playerJoyBut[1][2] = -1;

  // Score bar position
  g.scoreBarPos = getConfig(config, "SCOREBAR_POS", 0, 0, 1); 
}

// Main Initialization routine

void init_data(gamedata &g){

  init_gameconstants(g);

  init_gamedata(g);

  // save the config
  saveConfig(ap_home, "apricots.cfg", g);

  // Set Random seed
  srand(time(0));

  /* Initialize defaults, Video and Audio */
  if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK)==-1)){
    fprintf(stderr, "Could not initialize SDL: %s.\n", SDL_GetError());
    exit(-1);
  }

  SDL_JoystickEventState(SDL_ENABLE);

  setup_display(g);

  // Set Window title
  SDL_WM_SetCaption("Apricots", NULL);

  // Hide cursor
  SDL_ShowCursor(0);
  
  load_shapes(g, g.images);

  load_font(g.virtualscreen, g.whitefont, g.greenfont);
  
  init_sound(g.sound);

  loadPlaneInfo(g);

}
