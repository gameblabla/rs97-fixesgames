/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2010 Marco Costalba, Joona Kiiski, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#if !defined(UCI_H_INCLUDED)
#define UCI_H_INCLUDED

#include <string>
#include <stdint.h>

////
//// Prototypes
////

std::string ToString(int v);
std::string ToString(float v);
int FromString(const std::string& v);

struct UCI_Command
{
	UCI_Command() { Clear(); }
	const std::string& Execute(const std::string& cmd);
	bool operator>>(std::string& v);
	bool operator>>(int& v);
	UCI_Command& operator<<(const std::string& v);
	UCI_Command& operator<<(uint64_t v);
	UCI_Command& operator<<(int64_t v);
	UCI_Command& operator<<(int v);
	UCI_Command& operator<<(char v);
	UCI_Command& operator<<(void (*func)(UCI_Command&)) { func(*this); return *this; }

	bool good() const { return cmd_pos != std::string::npos; }

	void Clear();

	std::string::size_type cmd_pos;
	std::string cmd;
	std::string res;
};
inline void endl(UCI_Command& uci) { uci << "\n"; }

extern void uci_main_loop();

#endif // !defined(UCI_H_INCLUDED)
