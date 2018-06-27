

// Enums.
enum
{
	MENU_Null = 0,
	MENU_Main,
	MENU_Game,
	MENU_HallOfFame,
	MENU_Quit,

};


// Prototypes.
void Fade(s32 nFadeVal);

void MenuInit(void);
void MenuMain_Init(void);
u32 MenuMain_Main(void);
void MenuHighScores_Init(void);
u32 MenuHighScores_Main(void);
void MenuGetName_Init(void);
u32 MenuGetName_Main(void);

s32 Scr_CheckHighSc(u32 nScorePrm);
void Scr_Load(void);
void Scr_Save(void);



