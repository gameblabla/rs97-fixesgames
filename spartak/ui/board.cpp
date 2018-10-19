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

#include "board.h"
#include <string.h>
#include <assert.h>

namespace xUi
{

eBoard::eBoard()
	: chess_pieces(ePoint2(PIECES_W, PIECES_H))
	, chess_board(ePoint2(BOARD_W, BOARD_H))
	, position(ePoint2(BOARD_DIM, BOARD_DIM))
	, flash(false), flip(false)
{
	ResetTimer();
	Position("");
	Cursor("");
	Selected("");
	ReadPNG("res/chess_pieces.png",	chess_pieces.Data(), PIECES_W, PIECES_H, 4);
	ReadPNG("res/chess_board.png",	chess_board.Data(), BOARD_W, BOARD_H, 4);
}
void eBoard::Position(const char* fen)
{
	Invalidate();
	static const std::string pieces_chars = "KQRNBPkqrnbp";
	const char* piece = fen;
	int skip = 0;
	for(int j = 0; j < BOARD_DIM; ++j)
	{
		for(int i = 0; i < BOARD_DIM; ++i)
		{
			signed char p = -1;
			if(skip)
				--skip;
			if(!skip && *piece)
			{
				if(*piece >= '1' && *piece <= '8')
					skip = *piece - '0';
				else
				{
					std::string::size_type i = pieces_chars.find(*piece);
					if(i != std::string::npos)
						p = i;
				}
				++piece;
				if(*piece == '/')
					++piece;
			}
			position[ePoint2(i, j)] = p;
		}
	}
}
void eBoard::Cursor(const char* pos)
{
	Invalidate();
	if(*pos)
	{
		assert(strlen(pos) == 2);
	}
	strcpy(cursor, pos);
	flash = true;
}
void eBoard::Selected(const char* pos)
{
	Invalidate();
	if(*pos)
	{
		assert(strlen(pos) == 2);
	}
	strcpy(selected, pos);
	flash = true;
}
signed char eBoard::Piece(const char* pos) const
{
	if(!pos)
		pos = selected;
	if(!*pos)
		return -1;
	int i = pos[0] - 'a';
	int j = '8' - pos[1];
	return position[ePoint2(i, j)];
}
void eBoard::DrawCell(eBufferRGBA& buf, const ePoint2& pos, bool black, eCellView view, signed char piece)
{
	ePoint2 cell_pos(0, 0);
	if(black)
		cell_pos.y = PIECE_SIZE;
	switch(view)
	{
	case CELL_NORMAL: 	break;
	case CELL_LIGHT:	cell_pos.x = PIECE_SIZE; break;
	case CELL_DARK:		cell_pos.x = 2*PIECE_SIZE; break;
	}
	if(piece >= 0)
	{
		ePoint2 piece_pos(0, 0);
		if(piece >= 6)
		{
			piece -= 6;
			piece_pos.y = PIECE_SIZE;
		}
		piece_pos.x = piece*PIECE_SIZE;
		for(int x = 0; x < PIECE_SIZE; ++x)
		{
			for(int y = 0; y < PIECE_SIZE; ++y)
			{
				eRGBA board_col = chess_board[cell_pos + ePoint2(x, y)];
				eRGBA piece_col = chess_pieces[piece_pos + ePoint2(x, y)];
				buf[pos + ePoint2(x, y)] = board_col + piece_col;
			}
		}
	}
	else
	{
		for(int x = 0; x < PIECE_SIZE; ++x)
		{
			for(int y = 0; y < PIECE_SIZE; ++y)
			{
				buf[pos + ePoint2(x, y)] = chess_board[cell_pos + ePoint2(x, y)];
			}
		}
	}
}
bool eBoard::Update()
{
	if(cursor[0] && PassedTimeMs() > 200)
	{
		flash = !flash;
		Invalidate();
	}
	return eDialog::Update();
}
void eBoard::Paint(eBufferRGBA& buf)
{
	valid = true;
	for(int i = 0; i < BOARD_DIM; ++i)
	{
		char col = flip ? 'h' - i : 'a' + i;
		font->DrawChar(buf, ePoint2(16 + 9 + i*PIECE_SIZE, 0), col);
		font->DrawChar(buf, ePoint2(16 + 9 + i*PIECE_SIZE, PIECE_SIZE*BOARD_DIM + eFont::CHAR_H), col);

		char row = flip ? '1' + i : '1' + BOARD_DIM - i - 1;
		font->DrawChar(buf, ePoint2(4, 16 + 5 + i*PIECE_SIZE), row);
		font->DrawChar(buf, ePoint2(16 + 2 + PIECE_SIZE*BOARD_DIM, 16 + 5 + i*PIECE_SIZE), row);
	}

	for(int j = 0; j < BOARD_DIM; ++j)
	{
		for(int i = 0; i < BOARD_DIM; ++i)
		{
			ePoint2 cell = flip ? ePoint2(BOARD_DIM - i - 1, BOARD_DIM - j - 1) : ePoint2(i, j);
			char cell_str[3];
			cell_str[0] = 'a' + cell.x;
			cell_str[1] = '8' - cell.y;
			cell_str[2] = '\0';
			signed char p = position[cell];
			bool black = (i + j) % 2 != 0;
			eCellView view = CELL_NORMAL;
			if(!strcmp(cell_str, selected))
				view = CELL_LIGHT;
			if(!strcmp(cell_str, cursor))
				view = flash ? CELL_LIGHT : CELL_DARK;
			DrawCell(buf, ePoint2(16 + i*PIECE_SIZE, 16 + j*PIECE_SIZE), black, view, p);
		}
	}
}
bool eBoard::Command(char cmd)
{
	switch(cmd)
	{
	case 'l':
	case 'r':
	case 'u':
	case 'd':
		{
			char pos[3];
			strcpy(pos, Cursor());
			if(!pos[0])
				return true;
			if(flip)
			{
				switch(cmd)
				{
				case 'l':	cmd = 'r';	break;
				case 'r':	cmd = 'l';	break;
				case 'u':	cmd = 'd';	break;
				case 'd':	cmd = 'u';	break;
				}
			}
			switch(cmd)
			{
			case 'l':
				if(pos[0] != 'a')
					--pos[0];
				else
					pos[0] = 'h';
				break;
			case 'r':
				if(pos[0] != 'h')
					++pos[0];
				else
					pos[0] = 'a';
				break;
			case 'u':
				if(pos[1] != '8')
					++pos[1];
				else
					pos[1] = '1';
				break;
			case 'd':
				if(pos[1] != '1')
					--pos[1];
				else
					pos[1] = '8';
				break;
			}
			Cursor(pos);
		}
		return true;
	}
	return false;
}

}
//namespace xUi
