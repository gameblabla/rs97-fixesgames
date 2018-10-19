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

#include "ui.h"
#include "../stockfish/misc.h"
#include <png.h>
#include <string>
#include "../io.h"

namespace xUi
{

bool ReadPNG(const char* name, void* buffer, dword w, dword h, byte channels)
{
	FILE* f = fopen(xIo::Resource(name), "rb");
	if(!f)
		return false;
	png_struct* png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
		return false;
	png_info* info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return false;
	}
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		return false;
	}
	png_byte header[8];
	if(fread(header, 1, 8, f) != 8)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(f);
		return false;
	}
	if(png_sig_cmp(header, 0, 8))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(f);
		return false;
	}
	png_init_io(png_ptr, f);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);
	if(png_get_image_width(png_ptr, info_ptr) != w || png_get_image_height(png_ptr, info_ptr) != h ||
	  png_get_bit_depth(png_ptr, info_ptr) != 8 || png_get_channels(png_ptr, info_ptr) != channels ||
		png_get_rowbytes(png_ptr, info_ptr) != png_get_channels(png_ptr, info_ptr)*png_get_image_width(png_ptr, info_ptr))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(f);
		return false;
	}
	png_byte** row_pointers = new png_byte*[h];
	for(dword y = 0; y < h; ++y)
	{
		row_pointers[y] = (byte*)buffer + y*png_get_rowbytes(png_ptr, info_ptr);
	}
	png_read_image(png_ptr, row_pointers);
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	delete[] row_pointers;
	fclose(f);
	return true;
}

eFont::eFont() : buffer(ePoint2(FONT_W, FONT_H))
{
	ReadPNG("res/font_monospace_14.png", buffer.Data(), FONT_W, FONT_H, 1);
}
void eFont::DrawChar(eBufferRGBA& buf, const ePoint2& pos, char ch, const eRGBA& color)
{
	ePoint2 symbol_pos(0, 0);
	if(ch >= 'A' && ch <= 'Z')
	{
		symbol_pos.x = (ch - 'A')*CHAR_W;
	}
	else if(ch >= 'a' && ch <= 'z')
	{
		symbol_pos.x = (ch - 'a')*CHAR_W;
		symbol_pos.y = CHAR_H;
	}
	else
	{
		static const std::string symbols = "0123456789 !?:;#$%&()+-*/=";
		std::string::size_type p = symbols.find(ch);
		if(p == std::string::npos)
			p = 10;// space
		symbol_pos.x = p*CHAR_W;
		symbol_pos.y = CHAR_H*2;
	}
	for(int x = 0; x < CHAR_W; ++x)
	{
		for(int y = 0; y < CHAR_H; ++y)
		{
			eRGBA c0(color);
			c0.a = buffer[symbol_pos + ePoint2(x, y)];
			eRGBA c = buf[pos + ePoint2(x, y)];
			c = c + c0;
			buf[pos + ePoint2(x, y)] = c;
		}
	}
}
void eFont::DrawText(eBufferRGBA& buf, const ePoint2& _pos, const char* text, const eRGBA& color)
{
	ePoint2 pos(_pos);
	while(*text)
	{
		if(*text == '\n')
		{
			pos.x = _pos.x;
			pos.y += CHAR_H;
		}
		else
		{
			DrawChar(buf, pos, *text, color);
			pos.x += CHAR_W;
		}
		++text;
	}
}

}
//namespace xUi

int Clock() { return get_system_time(); }
