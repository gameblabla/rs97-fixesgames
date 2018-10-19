/*
Spartak Chess based on stockfish engine.
Copyright (C) 2010 djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __GAME_H__
#define __GAME_H__

#include <string>

#pragma once

namespace xUi
{
class eDialog;
class eBoard;
class eDesktop;
}
//namespace xUi

class eGame
{
public:
	eGame();
	~eGame();

	void New(bool side = false);
	bool Move(const char* move = NULL);
	enum eApplyCellResult { AC_ERROR, AC_RESET, AC_MOVE_BEGIN, AC_MOVE_END, AC_SELECT_PIECE };
	eApplyCellResult ApplyCell(const char* cell = NULL);
	bool SelectPiece(char piece);
	bool Command(char cmd);
	bool Update();

	xUi::eDesktop& Desktop() const { return *desktop; }

protected:
	void Init();
	void Done();
	void UpdateBoardPosition();
	bool Finished() const;
	void UpdateMove();
	void OpenMenu();
	void UpdateMenu();
	void ProcessDialogs();
	void CloseDialog(xUi::eDialog** d);
	bool MoveSide() const { return move_count % 2 != 0; }
	void Load();
	void Store();

protected:
	xUi::eDesktop*	desktop;
	xUi::eBoard*	board;
	xUi::eDialog*	piece_selector;
	xUi::eDialog*	game_status;
	xUi::eDialog*	move_status;
	xUi::eDialog*	splash;
	xUi::eDialog*	menu;

	char move[6];
	char move_ai[6];
	int move_count;
	std::string moves;

	enum eGameState { GS_NONE, GS_CHECK, GS_MATE, GS_STALEMATE, GS_DRAW };
	eGameState game_state;

	enum eState { S_NONE, S_SPLASH0, S_SPLASH, S_INIT, S_GAME, S_QUIT };
	eState state;
	int timer;

	enum eMoveState { MS_PLAYER, MS_AI_THINK, MS_AI_MOVE0, MS_AI_MOVE };
	eMoveState move_state;
	bool move_state_changed;
	char stored_cursor[3];
	void MoveState(eMoveState _ms);

	enum eDifficulty { D_EASY, D_NORMAL, D_HARD };
	eDifficulty difficulty;
	static const char* difficulty_names[];
};

#endif//__GAME_H__
