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

#ifndef __DIALOG_H__
#define __DIALOG_H__

#include "ui.h"
#include <string>
#include <vector>

#pragma once

namespace xUi
{

class eDialog;
typedef std::vector<eDialog*> eDialogs;

class eDialog
{
public:
	eDialog() : valid(false) {}
	virtual ~eDialog() {}
	virtual void Create() {}
	virtual void Destroy();
	virtual void Paint(eBufferRGBA& buf);
	virtual bool Command(char cmd) { return false; }
	virtual bool Valid() const;
	virtual bool Update();

	void Insert(eDialog* child) { childs.push_back(child); Invalidate(); }
	void Remove(eDialog* child);

	void Invalidate() { valid = false; }

	const eRect& Bound() const { return bound; }
	void Bound(const eRect& b) { bound = b; Invalidate(); }

	const std::string& Text() const { return text; }
	void Text(const std::string& t) { if(text != t) { text = t; Invalidate(); } }

protected:
	eRect bound;
	std::string text;
	eDialogs childs;
	bool valid;

	static eFont* font;
};

class eDesktop : public eDialog
{
public:
	eDesktop();
	virtual ~eDesktop();
	virtual bool Update();
	void	Clear() { buffer.Clear(); }
	eRGBA*	Buffer() const { return buffer.Data(); }
	eDialog* Focus() const { return focus; }
	void	Focus(eDialog* f) { focus = f; }
	bool	Command(char cmd) { if(!focus) return false; return focus->Command(cmd); }

protected:
	eDialog* focus;
	eBufferRGBA	buffer;
};

eDialog* Picture(const char* name, const ePoint2& size);

}
//namespace xUi

#endif//__DIALOG_H__
