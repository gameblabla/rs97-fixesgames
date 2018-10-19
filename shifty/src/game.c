#include "game.h"
#include "backend/video.h"
#include "ai.h"
#include "block.h"
#include "font.h"
#include "network.h"
#include "piece.h"
#include "player.h"
#include "states.h"
#include "tileset.h"

Game game;
Image *backgroundGfx;

static void gameRestart()
{
	boardInit(&player1.board, BOARD_OFFSET_X_P1, BOARD_OFFSET_Y, &player1.stats);
	memset(&player1.stats, 0, sizeof(Stats));
	player1.speed = PLAYER_SPEED;
	player1.board.canMove = 1;

	player1.local = 1;
	player1.ai.mode = AI_ANALYZE;
	player1.ai.target.x = 2;

	pieceNew(&player1.pieceNext);
	pieceNew(&player1.piece);
	player1.piece.drawShadow = 1;
	pieceCenter(&player1.piece);

	if (game.mode != GAME_MODE_SOLO)
	{
		boardInit(&player2.board, BOARD_OFFSET_X_P2, BOARD_OFFSET_Y, &player2.stats);
		memset(&player2.stats, 0, sizeof(Stats));
		player2.speed = PLAYER_SPEED;
		player2.board.canMove = 1;

		player2.local = game.mode == GAME_MODE_NETWORKED ? 0 : 1;
		player2.ai.mode = AI_ANALYZE;
		player2.ai.target.x = 2;

		pieceNew(&player2.pieceNext);
		pieceNew(&player2.piece);
		player2.piece.drawShadow = 1;
		pieceCenter(&player2.piece);
	}
}

void gameLoad()
{
	game.state = GAME_STATE_PLAYING;

	if (!(backgroundGfx = loadImage(game.mode == GAME_MODE_SOLO ? "data/bg01p1.bmp" : "data/bg01p2.bmp")))
		goto error;

	if (!blocksTileset && !(blocksTileset = tilesetLoad("data/blocks.bmp", BLOCK_SIZE, BLOCK_SIZE, 5, 50)))
		goto error;

	defaultKeymap(player1.keyMap, 1);
	
	if (game.mode == GAME_MODE_LOCAL)
		defaultKeymap(player2.keyMap, 2);

	gameRestart();
	return;

	error:
		gameUnload();
		fprintf(stderr, "Missing game assets.\n");
		exit(1);
}

void gameUnload()
{
	if (game.mode == GAME_MODE_NETWORKED)
		networkCloseConnection();

	if (blocksTileset)
	{
		tilesetUnload(blocksTileset);
		blocksTileset = NULL;
	}
	unloadImage(backgroundGfx);
}

void gameLogic()
{
	if (*player1.keyMap[KEY_BACK])
	{
		*player1.keyMap[KEY_BACK] = 0;

		stateNew = STATE_TITLE;
	}

	if (*player1.keyMap[KEY_OK])
	{
		*player1.keyMap[KEY_OK] = 0;

		switch (game.state)
		{
			case GAME_STATE_OVER:
				gameRestart();
			/* Fall through. */
			case GAME_STATE_PAUSED:
				game.state = GAME_STATE_PLAYING;
			break;
			case GAME_STATE_PLAYING:
				game.state = GAME_STATE_PAUSED;
			break;
		}
	}

	if (game.state == GAME_STATE_PLAYING)
	{
		++game.ticks;

		if (game.mode == GAME_MODE_NETWORKED)
			networkReceivePacket();

		if (player1.board.over)
		{
			if (game.mode == GAME_MODE_SOLO || (player2.board.over || player2.board.canMove))
			{
				game.state = GAME_STATE_OVER;
				return;
			}
		}
		else if (game.mode != GAME_MODE_SOLO && player2.board.over)
		{
			if (player1.board.over || player1.board.canMove)
			{
				game.state = GAME_STATE_OVER;
				return;
			}
		}
		
		playerLogic(&player1);

		if (game.mode != GAME_MODE_SOLO)
			playerLogic(&player2);
	}
}

void gameDrawing()
{
	char messageTxt[50];

	clearScreen();

	drawImage(backgroundGfx, NULL, 0, 0);

	if (game.state != GAME_STATE_PAUSED)
	{
		boardDraw(&player1.board);
		pieceDraw(&player1.piece, player1.board.x, player1.board.y);
		pieceDraw(&player1.pieceNext, PIECE_NEXT_X_P1, PIECE_NEXT_Y_P1);

		if (game.mode != GAME_MODE_SOLO)
		{
			boardDraw(&player2.board);
			pieceDraw(&player2.piece, player2.board.x, player2.board.y);
			pieceDraw(&player2.pieceNext, PIECE_NEXT_X_P2, PIECE_NEXT_Y_P2);
		}
	}

	/* Text. */
	dTextCenteredAtOffset(fontDefault, SHADOW_OUTLINE, "Shifty Pills", 16, game.mode == GAME_MODE_SOLO ? 215 : SCREEN_W/2);

	if (game.mode == GAME_MODE_SOLO)
	{
		char statsTxt[150];
		char rulesTxt[100];
		char shiftTxt[10];

		snprintf(shiftTxt, 10, "%*d/%d", 2, player1.board.shift.count - player1.board.shift.ticks, player1.board.shift.count);

		snprintf(statsTxt, 150, "Score:  %-*d\nPiece  #%d\nPills:  %d\nClears: %d\nShift in: %s", 7, player1.stats.score, player1.stats.pieces, player1.stats.blocks, player1.stats.linesHorizontal + player1.stats.linesVertical, game.modifiers.shiftEvent ? shiftTxt : "n/a");
		dTextCenteredAtOffset(fontDefault, SHADOW_DROP, statsTxt, 85, 215);
		
		dText(fontDefault, SHADOW_OUTLINE, "Rules:", 118, 166);

		snprintf(rulesTxt, 100, "Line up %d or more pills of\na matching color in order\nto clear them.", LINE_LEN);
		dText(fontDefault, SHADOW_DROP, rulesTxt, 118, 182);
	}
	else
	{
		char statsP1Txt[150];
		char statsP2Txt[150];
		int textY = 90;

		snprintf(statsP1Txt, 150, "Piece  #%-*d\nClears: %d\nShift: %*d/%d", 4, player1.stats.pieces, player1.stats.linesHorizontal + player1.stats.linesVertical, 2, player1.board.shift.count - player1.board.shift.ticks, player1.board.shift.count);
		snprintf(statsP2Txt, 150, "Piece  #%-*d\nClears: %d\nShift: %*d/%d", 4, player2.stats.pieces, player2.stats.linesHorizontal + player2.stats.linesVertical, 2, player2.board.shift.count - player2.board.shift.ticks, player2.board.shift.count);

		dTextCentered(fontDefault, SHADOW_OUTLINE, "Player 1", textY);
		dTextCentered(fontDefault, SHADOW_DROP, statsP1Txt, textY + fontDefault->h + fontDefault->leading);
		dTextCentered(fontDefault, SHADOW_OUTLINE, "Player 2", textY + (fontDefault->h + fontDefault->leading) * 5);
		dTextCentered(fontDefault, SHADOW_DROP, statsP2Txt, textY + (fontDefault->h + fontDefault->leading) * 6);
	}

	if (game.state == GAME_STATE_OVER)
	{
		int x;
		int y;

		snprintf(messageTxt, 50, "GAME  OVER");
		x = BOARD_OFFSET_X_P1 + (BOARD_W * BLOCK_SIZE)/2 - strlen(messageTxt) * (fontDefault->w + fontDefault->tracking)/2;
		y = BOARD_OFFSET_Y + (BOARD_H * BLOCK_SIZE)/2 - (fontDefault->h + fontDefault->leading)/2;
		dText(fontDefault, SHADOW_OUTLINE, messageTxt, x, y);
	}
	else if (game.state == GAME_STATE_PAUSED)
	{
		int x;
		int y;

		snprintf(messageTxt, 50, "PAUSED");
		x = BOARD_OFFSET_X_P1 + (BOARD_W * BLOCK_SIZE)/2 - strlen(messageTxt) * (fontDefault->w + fontDefault->tracking)/2;
		y = BOARD_OFFSET_Y + (BOARD_H * BLOCK_SIZE)/2 - (fontDefault->h + fontDefault->leading)/2;
		dText(fontDefault, SHADOW_OUTLINE, messageTxt, x, y);
	}

	if (showFps)
	{
		snprintf(messageTxt, 50, "%d", fps);
		dText(fontDefault, SHADOW_NONE, messageTxt, SCREEN_W - 21, 1);
	}

	flipScreen();
}
