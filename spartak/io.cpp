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

#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

namespace xIo
{

static string resource_path;
void ResourcePath(const char* _path)
{
	string path = _path;
	string::size_type p = path.rfind('/');
	if(p == string::npos)
		p = path.rfind('\\');
	if(p != string::npos)
		resource_path = path.substr(0, p + 1);
}
static string resource;
const char* Resource(const char* _r)
{
	resource = resource_path + _r;
	return resource.c_str();
}

}
//namespace xIo

namespace xLog
{

static FILE* log_file = NULL;

void Close()
{
	if(log_file)
	{
		fclose(log_file);
		log_file = NULL;
	}
}

bool Open()
{
	FILE* f = fopen(xIo::Resource("chess.log"), "w");
	if(!f)
		return false;
	log_file = f;
	atexit(Close);
	return true;
}

void LOG(const string& str)
{
	LOG(str.c_str());
}

void LOG(const char* str)
{
	if(xLog::log_file)
	{
		fputs(str, log_file);
		fflush(log_file);
	}
}

}
//namespace xLog
