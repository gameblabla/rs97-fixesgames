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

#include "dialog.h"

namespace xUi
{

class eMenu : public eDialog
{
public:
	eMenu() : selected(0) {}
	virtual void Create();
	virtual void Paint(eBufferRGBA& buf);
	virtual bool Command(char cmd);
	void UpdateItem(const std::string& text, const std::string& id);
protected:
	void AddItem(const std::string& text, const std::string& id);
	struct eItem
	{
		std::string text;
		std::string id;
	};
	typedef std::vector<eItem> eItems;
	eItems items;
	int	selected;
};
void eMenu::Create()
{
	AddItem("New game (white)", "nw");
	AddItem("New game (black)", "nb");
	AddItem("", "d");
//	AddItem("Book", "b");
	AddItem("Quit", "q");
	bound.beg = ePoint2(16 + 26/2, 16 + 26*2 + 2);
	bound.end = bound.beg + ePoint2(7*26, 20 + font->CHAR_H*items.size());
}
void eMenu::AddItem(const std::string& text, const std::string& id)
{
	eItem i;
	i.text = text;
	i.id = id;
	items.push_back(i);
}
void eMenu::UpdateItem(const std::string& text, const std::string& id)
{
	for(eItems::iterator i = items.begin(); i != items.end(); ++i)
	{
		if(i->id == id)
		{
			i->text = text;
			Invalidate();
		}
	}
}
void eMenu::Paint(eBufferRGBA& buf)
{
	valid = true;
	ePoint2 p = bound.beg + ePoint2(10, 10);
	for(int j = bound.beg.y; j < bound.end.y; ++j)
		for(int i = bound.beg.x; i < bound.end.x; ++i)
			buf[ePoint2(i, j)] = buf[ePoint2(i, j)] + eRGBA(0, 0, 0, 0xc0);
	for(eItems::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		int idx = i - items.begin();
		bool sel = selected == idx;
		font->DrawText(buf, p, i->text.c_str(), sel ? eRGBA(255, 255, 255) : eRGBA(160, 160, 160));
		p.y += font->CHAR_H;
	}
}
bool eMenu::Command(char cmd)
{
	switch(cmd)
	{
	case 'u':
		if(--selected < 0)
			selected = (int)items.size() - 1;
		Invalidate();
		break;
	case 'd':
		if(++selected >= (int)items.size())
			selected = 0;
		Invalidate();
		break;
	case 'a':
		Text(items[selected].id);
		break;
	case 'b':
	case 'f':
		Text("-");
		break;
	}
	return true;
}

eDialog* Menu()
{
	eDialog* m = new eMenu;
	m->Create();
	return m;
}
void MenuUpdateItem(eDialog* m, const std::string& text, const std::string& id)
{
	((eMenu*)m)->UpdateItem(text, id);
}

}
//namespace xUi
