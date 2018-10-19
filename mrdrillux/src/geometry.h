#ifndef geometry_h_included
#define geometry_h_included

#if TILE_SIZE == 24
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 8
#elif TILE_SIZE == 48
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FONT_SIZE 16
#else
#error Unsupported tile size!
#endif

#if SCREEN_WIDTH == 320
#define DEPTH_X 250
#define DEPTH_Y 50
#define SCORE_X 240
#define SCORE_Y 110
#define AIR_BAR_X 243
#define AIR_BAR_Y 160
#define AIR_NUM_X (AIR_BAR_X + 7)
#define AIR_NUM_Y (AIR_BAR_Y + 4)
#define GAME_NOTICE_Y 70
#define MAIN_MENU_OPTIONS_X 90
#define MAIN_MENU_OPTIONS_Y 140
#define MAIN_MENU_OPTIONS_DY 18
#define HIGHSCORE_MARK_X 156
#define HIGHSCORE_MARK_Y 152
#define HIGHSCORE_TEXT_X 35
#define HIGHSCORE_LABEL_Y 75
#define HIGHSCORE_SCORE_Y (HIGHSCORE_LABEL_Y + 12)
#define SCOREBOARD_X 65
#define SCOREBOARD_LABEL_Y 60
#define SCOREBOARD_SCORE_Y 75
#define SCOREBOARD_SCORE_DY 12
#elif SCREEN_WIDTH == 640
#define DEPTH_X 500
#define DEPTH_Y 100
#define SCORE_X 480
#define SCORE_Y 220
#define AIR_BAR_X 486
#define AIR_BAR_Y 320
#define AIR_NUM_X (AIR_BAR_X + 14)
#define AIR_NUM_Y (AIR_BAR_Y + 8)
#define GAME_NOTICE_Y 140
#define MAIN_MENU_OPTIONS_X 180
#define MAIN_MENU_OPTIONS_Y 300
#define MAIN_MENU_OPTIONS_DY 30
#define HIGHSCORE_MARK_X (308 + 5)
#define HIGHSCORE_MARK_Y (300 + 5)
#define HIGHSCORE_TEXT_X 70
#define HIGHSCORE_LABEL_Y 150
#define HIGHSCORE_SCORE_Y (HIGHSCORE_LABEL_Y + 25)
#define SCOREBOARD_X 130
#define SCOREBOARD_LABEL_Y 120
#define SCOREBOARD_SCORE_Y 150
#define SCOREBOARD_SCORE_DY 25
#else
#error Unsupported screen width!
#endif

#endif
