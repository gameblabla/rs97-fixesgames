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

#ifndef __IO_H__
#define __IO_H__

#pragma once

namespace xIo
{
void ResourcePath(const char* _path);
const char* Resource(const char* _r);
}
//namespace xIo

namespace xLog
{
bool Open();
void Close();
void LOG(const char* str);
}
//namespace xLog

using xLog::LOG;

#endif//__IO_H__
