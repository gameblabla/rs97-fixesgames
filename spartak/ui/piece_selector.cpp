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

namespace xUi
{

class ePieceSelector : public eDialog
{
public:
	ePieceSelector(bool _side, eBoard* _board) : side(_side), board(_board), selected(1) {}
	virtual void Paint(eBufferRGBA& buf);
	virtual bool Command(char cmd);
	bool side;
	eBoard* board;
	char selected;
};
void ePieceSelector::Paint(eBufferRGBA& buf)
{
	valid = true;
	for(int j = 0; j < buf.Size().y; ++j)
		for(int i = 0; i < buf.Size().x; ++i)
			buf[ePoint2(i, j)] = buf[ePoint2(i, j)] + eRGBA(0, 0, 0, 0xc0);
	ePoint2 pos(68, 100);
	font->DrawText(buf, pos, "Select piece:");
	pos.y += 16;
	for(char p = 1; p < 5; ++p)
	{
		eBoard::eCellView view = board->CELL_NORMAL;
		if(selected == p)
			view = board->Flash() ? board->CELL_LIGHT : board->CELL_DARK;
		board->DrawCell(buf, pos, p % 2, view, p + side*6);
		pos.x += board->PIECE_SIZE;
	}
}
bool ePieceSelector::Command(char cmd)
{
	switch(cmd)
	{
	case 'l':
		if(selected > 1)
		{
			--selected;
			Invalidate();
		}
		break;
	case 'r':
		if(selected < 4)
		{
			++selected;
			Invalidate();
		}
		break;
	case 'a':
		switch(selected)
		{
		case 1:	Text("q");	break;
		case 2:	Text("r");	break;
		case 3:	Text("n");	break;
		case 4:	Text("b");	break;
		}
		break;
	case 'b':
		Text("-");
		break;
	}
	return true;
}

eDialog* PieceSelector(bool side, eBoard* board)
{
	eDialog* ps = new ePieceSelector(side, board);
	ps->Create();
	return ps;
}

}
//namespace xUi
