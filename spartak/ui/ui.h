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

#ifndef __UI_H__
#define __UI_H__

#include "../std_types.h"

#pragma once

struct eRGBA
{
	eRGBA(dword _rgba = 0) : rgba(_rgba) {}
	eRGBA(byte _r, byte _g, byte _b, byte _a = 255) : r(_r), g(_g), b(_b), a(_a) {}
	union
	{
		struct { byte r,g,b,a; };
		dword rgba;
	};
	operator dword() const { return rgba; }
};
inline eRGBA operator+(const eRGBA& v1, const eRGBA& v2)
{
	byte r = int(v1.r)*(255 - v2.a)/255 + int(v2.r)*v2.a/255;
	byte g = int(v1.g)*(255 - v2.a)/255 + int(v2.g)*v2.a/255;
	byte b = int(v1.b)*(255 - v2.a)/255 + int(v2.b)*v2.a/255;
	return eRGBA(r, g, b, v1.a);
}

struct ePoint2
{
	ePoint2(int _x = 0, int _y = 0) : x(_x), y(_y) {}
	int x, y;
};
inline ePoint2 operator+(const ePoint2& v1, const ePoint2& v2)
{
	return ePoint2(v1.x + v2.x, v1.y + v2.y);
}

struct eRect
{
	eRect() {}
	eRect(const ePoint2& b, const ePoint2& e) : beg(b), end(e) {}
	ePoint2 beg;
	ePoint2 end;
};

enum eZero { ZERO = 0 };

template<class T> class eBuffer
{
public:
	eBuffer(const ePoint2& _size) : size(_size) { data = new T[size.x*size.y]; Clear(); }
	~eBuffer() { delete[] data; }
	const ePoint2& Size() const { return size; }
	T* Data() const { return data; }
	T& operator[](const ePoint2& pos) const { return at(pos); }
	T& at(const ePoint2& pos) const { return data[pos.x + pos.y*size.x]; }
	void Clear(const eRect& bound, const T& v = ZERO)
	{
		for(int j = bound.beg.y; j < bound.end.y; ++j)
			for(int i = bound.beg.x; i < bound.end.x; ++i)
				at(ePoint2(i, j)) = v;
	}
	void Clear(const T& v = ZERO)
	{
		eRect b;
		b.end = size;
		Clear(b, v);
	}
protected:
	ePoint2 size;
	T* data;
};

namespace xUi
{

typedef eBuffer<eRGBA> eBufferRGBA;

class eFont
{
public:
	eFont();
	enum { CHAR_W = 8, CHAR_H = 16, FONT_W = CHAR_W*26, FONT_H = CHAR_H*3 };
	void DrawChar(eBufferRGBA& buf, const ePoint2& pos, char ch, const eRGBA& color = eRGBA(255, 255, 255, 255));
	void DrawText(eBufferRGBA& buf, const ePoint2& pos, const char* text, const eRGBA& color = eRGBA(255, 255, 255, 255));
protected:
	eBuffer<byte> buffer;
};

bool ReadPNG(const char* name, void* buffer, dword w, dword h, byte channels);

}
//namespace xUi

int Clock();

#endif//__UI_H__
