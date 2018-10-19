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

class ePicture : public eDialog
{
public:
	ePicture(const char* name, const ePoint2& size);
	virtual void Paint(eBufferRGBA& buf);

protected:
	eBufferRGBA buffer;
};

ePicture::ePicture(const char* name, const ePoint2& size) : buffer(size)
{
	ReadPNG(name, buffer.Data(), size.x, size.y, 4);
}

void ePicture::Paint(eBufferRGBA& buf)
{
	valid = true;
	for(int j = 0; j < buffer.Size().y; ++j)
		for(int i = 0; i < buffer.Size().x; ++i)
			buf[bound.beg + ePoint2(i, j)] = buffer[ePoint2(i, j)];
}

eDialog* Picture(const char* name, const ePoint2& size)
{
	eDialog* p = new ePicture(name, size);
	p->Create();
	return p;
}

}
//namespace xUi
