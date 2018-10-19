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

#ifndef __BOARD_H__
#define __BOARD_H__

#include "dialog.h"

#pragma once

namespace xUi
{

class eBoard : public eDialog
{
public:
	eBoard();
	void	Position(const char* fen);
	void	Cursor(const char* pos);
	const char*	Cursor() { return cursor; }
	void	Selected(const char* pos);
	const char*	Selected() { return selected; }
	signed char Piece(const char* pos = NULL) const;
	enum
	{
		PIECE_SIZE = 26, PIECES_W = PIECE_SIZE*6, PIECES_H = PIECE_SIZE*2,
		BOARD_W = PIECE_SIZE*3, BOARD_H = PIECE_SIZE*2,
		BOARD_DIM = 8,
	};
	virtual void Paint(eBufferRGBA& buf);
	virtual bool Command(char cmd);
	virtual bool Update();

	enum eCellView { CELL_NORMAL, CELL_LIGHT, CELL_DARK };
	void DrawCell(eBufferRGBA& buf, const ePoint2& pos, bool black, eCellView view, signed char piece);
	bool Flash() const { return flash; }

	void Flip(bool _flip) { flip = _flip; Invalidate(); }
	bool Flip() const { return flip; }

	eBufferRGBA chess_pieces;
	eBufferRGBA chess_board;

protected:
	int		PassedTimeMs() const { return Clock() - timer; }
	void	ResetTimer() { timer = Clock(); }
	void 	Invalidate() { ResetTimer(); eDialog::Invalidate(); }
protected:
	eBuffer<signed char> position;
	char	cursor[3];
	char	selected[3];
	bool	flash;
	bool	flip;

	int		timer;
};

}
//namespace xUi

#endif//__BOARD_H__
