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


////
//// Includes
////

#include <assert.h>
//#include <iostream>
//#include <sstream>
#include <string>
#include <stdio.h>

#include "book.h"
#include "evaluate.h"
#include "misc.h"
#include "move.h"
#include "movegen.h"
#include "position.h"
#include "san.h"
#include "search.h"
#include "uci.h"
#include "ucioption.h"

using namespace std;

////
//// Local definitions:
////

#ifdef _MSC_VER
#include <stdint.h>
#define snprintf sprintf_s
#define PRId64 "lld"
#define PRIu64 "llu"
#else
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif//_MSC_VER

std::string ToString(int v)
{
  char buf[128];
  snprintf(buf, 128, "%d", v);
  return buf;
}
std::string ToString(float v)
{
  char buf[128];
  snprintf(buf, 128, "%g", v);
  return buf;
}
int FromString(const std::string& v)
{
  return atoi(v.c_str());
}


namespace {

  // UCIInputParser is a class for parsing UCI input. The class
  // is actually a string stream built on a given input string.

//  typedef istringstream UCIInputParser;

  // The root position. This is set up when the user (or in practice, the GUI)
  // sends the "position" UCI command. The root position is sent to the think()
  // function when the program receives the "go" command.
  Position RootPosition(0);

  // Local functions
  bool handle_command(UCI_Command& uci);
  void set_option(UCI_Command& uci);
  void set_position(UCI_Command& uci);
  bool go(UCI_Command& uci);
  void perft(UCI_Command& uci);
}


////
//// Functions
////

/// uci_main_loop() is the only global function in this file. It is
/// called immediately after the program has finished initializing.
/// The program remains in this loop until it receives the "quit" UCI
/// command. It waits for a command from the user, and passes this
/// command to handle_command and also intercepts EOF from stdin,
/// by translating EOF to the "quit" command. This ensures that Stockfish
/// exits gracefully if the GUI dies unexpectedly.

void uci_main_loop() {

  RootPosition.from_fen(StartPosition);
  UCI_Command uci;

  do {
      // Wait for a command from stdin
//      if (!getline(cin, command))
//          command = "quit";

  } while (handle_command(uci));
}

bool UCI_Command::operator>>(std::string& v)
{
	if(!good())
		return false;
	string::size_type p = cmd.find(' ', cmd_pos);
	if(p != string::npos)
	{
		v = cmd.substr(cmd_pos, p - cmd_pos);
		cmd_pos = p + 1;
	}
	else
	{
		v = cmd.substr(cmd_pos);
		cmd_pos = string::npos;
	}
	return true;
}
bool UCI_Command::operator>>(int& v)
{
	string val;
	if(!operator>>(val))
		return false;
	v = FromString(val);
	return true;
}
UCI_Command& UCI_Command::operator<<(const std::string& _v)
{
	res += _v;
	return *this;
}
UCI_Command& UCI_Command::operator<<(int64_t v)
{
	char buf[128];
	snprintf(buf, 128, "%"PRId64, v);
	res += buf;
	return *this;
}
UCI_Command& UCI_Command::operator<<(uint64_t v)
{
	char buf[128];
	snprintf(buf, 128, "%"PRIu64, v);
	res += buf;
	return *this;
}
UCI_Command& UCI_Command::operator<<(int v)
{
	char buf[128];
	snprintf(buf, 128, "%d", v);
	res += buf;
	return *this;
}
UCI_Command& UCI_Command::operator<<(char _v)
{
	res += _v;
	return *this;
}

void UCI_Command::Clear()
{
	cmd_pos = 0;
	cmd.clear();
	res.clear();
}
const string& UCI_Command::Execute(const string& _cmd)
{
	Clear();
	cmd = _cmd;
	handle_command(*this);
	return res;
}

////
//// Local functions
////

namespace {

  // handle_command() takes a text string as input, uses a
  // UCIInputParser object to parse this text string as a UCI command,
  // and calls the appropriate functions. In addition to the UCI
  // commands, the function also supports a few debug commands.

  bool handle_command(UCI_Command& uci) {

    string token;

    if (!(uci >> token)) // operator>>() skips any whitespace
        return true;

    if (token == "quit")
        return false;

    if (token == "go")
        return go(uci);

    if (token == "uci")
    {
        uci << "id name " << engine_name()
             << "\nid author Tord Romstad, Marco Costalba, Joona Kiiski\n";
        print_uci_options(uci);
        uci << "uciok" << endl;
    }
    else if (token == "ucinewgame")
    {
        push_button(uci, "New Game");
        Position::init_piece_square_tables();
        RootPosition.from_fen(StartPosition);
    }
    else if (token == "isready")
    {
        uci << "readyok" << endl;
    }
    else if (token == "position")
        set_position(uci);
    else if (token == "setoption")
        set_option(uci);

    // The remaining commands are for debugging purposes only.
    // Perhaps they should be removed later in order to reduce the
    // size of the program binary.
    else if (token == "d")
        RootPosition.print(uci);
    else if (token == "flip")
    {
        Position p(RootPosition, RootPosition.thread());
        RootPosition.flipped_copy(p);
    }
    else if (token == "eval")
    {
        EvalInfo ei;
        uci << "Incremental mg: " << mg_value(RootPosition.value())
             << "\nIncremental eg: " << eg_value(RootPosition.value())
             << "\nFull eval: " << evaluate(RootPosition, ei) << endl;
    }
    else if (token == "key")
    {
        uci << "key: " << RootPosition.get_key()
             << "\nmaterial key: " << RootPosition.get_material_key()
             << "\npawn key: " << RootPosition.get_pawn_key() << endl;
    }
    else if (token == "perft")
    {
        perft(uci);
    }
    else
    {
        uci << "Unknown command: " << token << endl;
    }

    return true;
  }


  // set_position() is called when Stockfish receives the "position" UCI
  // command. The input parameter is a UCIInputParser. It is assumed
  // that this parser has consumed the first token of the UCI command
  // ("position"), and is ready to read the second token ("startpos"
  // or "fen", if the input is well-formed).

  void set_position(UCI_Command& uci) {

    string token;

    if (!(uci >> token)) // operator>>() skips any whitespace
        return;

    if (token == "startpos")
        RootPosition.from_fen(StartPosition);
    else if (token == "fen")
    {
        string fen;
        while (uci >> token && token != "moves")
        {
            fen += token;
            fen += ' ';
        }
        RootPosition.from_fen(fen);
    }

    if(uci.good())
    {
        if (token != "moves")
          uci >> token;

        if (token == "moves")
        {
            Move move;
            StateInfo st;
            while(uci >> token)
            {
                move = move_from_string(RootPosition, token);
                if(move_is_legal(RootPosition, move))
                {
					RootPosition.do_move(move, st);
					if (RootPosition.rule_50_counter() == 0)
						RootPosition.reset_game_ply();
                }
                else
                {
                	uci << token << ": illegal move" << endl;
                }
            }
			if(RootPosition.is_mate())
				uci << "mate" << endl;
			else if(RootPosition.is_check())
				uci << "check" << endl;
			else if(RootPosition.is_draw())
				uci << "draw" << endl;

			// Our StateInfo st is about going out of scope so copy
            // its content inside RootPosition before they disappear.
            RootPosition.detach();
        }
    }
  }


  // set_option() is called when Stockfish receives the "setoption" UCI
  // command. The input parameter is a UCIInputParser. It is assumed
  // that this parser has consumed the first token of the UCI command
  // ("setoption"), and is ready to read the second token ("name", if
  // the input is well-formed).

  void set_option(UCI_Command& uci) {

    string token, name, value;

    if (!(uci >> token)) // operator>>() skips any whitespace
        return;

    if (token == "name" && uci >> name)
    {
        while (uci >> token && token != "value")
            name += (" " + token);

        if (token == "value" && uci >> value)
        {
            while (uci >> token)
                value += (" " + token);

            set_option_value(uci, name, value);
        } else
            push_button(uci, name);
    }
  }


  // go() is called when Stockfish receives the "go" UCI command. The
  // input parameter is a UCIInputParser. It is assumed that this
  // parser has consumed the first token of the UCI command ("go"),
  // and is ready to read the second token. The function sets the
  // thinking time and other parameters from the input string, and
  // calls think() (defined in search.cpp) with the appropriate
  // parameters. Returns false if a quit command is received while
  // thinking, returns true otherwise.
  struct eGo
  {
  	eGo()
  	{
  		time[0] = time[1] = 0;
  		inc[0] = inc[1] = 0;
  		movesToGo = 0;
  		depth = 0;
  		nodes = 0;
  		moveTime = 0;
  		infinite = ponder = false;
  		searchMoves[0] = MOVE_NONE;
  	}
	int time[2];
	int inc[2];
	int movesToGo, depth, nodes, moveTime;
	bool infinite, ponder;
	Move searchMoves[500];
  };
  bool go(UCI_Command& uci) {

    string token;

    eHeapAlloc<eGo> go;
    while (uci >> token)
    {
        if (token == "infinite")
        	go->infinite = true;
        else if (token == "ponder")
        	go->ponder = true;
        else if (token == "wtime")
            uci >> go->time[0];
        else if (token == "btime")
            uci >> go->time[1];
        else if (token == "winc")
            uci >> go->inc[0];
        else if (token == "binc")
            uci >> go->inc[1];
        else if (token == "movestogo")
            uci >> go->movesToGo;
        else if (token == "depth")
            uci >> go->depth;
        else if (token == "nodes")
            uci >> go->nodes;
        else if (token == "movetime")
            uci >> go->moveTime;
        else if (token == "searchmoves")
        {
            int numOfMoves = 0;
            while (uci >> token)
            	go->searchMoves[numOfMoves++] = move_from_string(RootPosition, token);

            go->searchMoves[numOfMoves] = MOVE_NONE;
        }
    }

    assert(RootPosition.is_ok());

    return think(uci, RootPosition, go->infinite, go->ponder, RootPosition.side_to_move(),
                 go->time, go->inc, go->movesToGo, go->depth, go->nodes, go->moveTime, go->searchMoves);
  }

  void perft(UCI_Command& uci) {

    string token;
    int depth, tm, n;
    Position pos(RootPosition, RootPosition.thread());

    if (!(uci >> depth))
        return;

    tm = get_system_time();

    n = perft(pos, depth * OnePly);

    tm = get_system_time() - tm;
    uci << "\nNodes " << n
        << "\nTime (ms) " << tm
        << "\nNodes/second " << (int)(n/(tm/1000.0)) << endl;
  }
}
