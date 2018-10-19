//
//   tiles.c
//
//   Copyright 2007, 2008 Lancer-X/ASCEAI
//
//   This file is part of Meritous.
//
//   Meritous is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Meritous is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Meritous.  If not, see <http://www.gnu.org/licenses/>.
//

// tile data

struct TileInfo {
	int Is_Solid;
	int Is_Special;
	int Is_Passage;
	int Is_Wall;
	int Type;
};

struct TileInfo TileData[57] = {
	{1, 0, 0, 1, 0}, //  0
	{1, 0, 0, 1, 0}, //  1
	{1, 0, 0, 1, 0}, //  2
	{1, 0, 0, 1, 0}, //  3
	{1, 0, 0, 1, 0}, //  4
	{1, 0, 0, 1, 0}, //  5
	{1, 0, 0, 1, 0}, //  6
	{1, 0, 0, 1, 0}, //  7
	{1, 0, 0, 1, 0}, //  8
	{1, 0, 0, 1, 0}, //  9
	{1, 0, 0, 1, 0}, // 10
	{1, 0, 0, 1, 0}, // 11
	{0, 0, 0, 0, 1}, // 12
	{1, 0, 1, 0, 2}, // 13
	{1, 0, 1, 0, 2}, // 14
	{1, 0, 1, 0, 2}, // 15
	{1, 0, 1, 0, 2}, // 16
	{1, 0, 0, 0, 3}, // 17
	{0, 0, 0, 0, 1}, // 18
	{0, 0, 0, 0, 1}, // 19
	{0, 0, 0, 0, 1}, // 20
	{1, 0, 0, 1, 4}, // 21
	{1, 0, 0, 1, 4}, // 22
	{1, 0, 0, 1, 4}, // 23
	{1, 0, 0, 1, 4}, // 24
	{0, 1, 0, 0, 5}, // 25
	{0, 1, 0, 0, 6}, // 26
	{0, 0, 0, 0, 7}, // 27
	{0, 1, 0, 0, 8}, // 28
	{0, 1, 0, 0, 8}, // 29
	{0, 1, 0, 0, 8}, // 30
	{0, 1, 0, 0, 8}, // 31
	{0, 1, 0, 0, 8}, // 32
	{0, 0, 0, 0, 1}, // 33
	{1, 0, 0, 0, 0}, // 34
	{1, 0, 0, 0, 0}, // 35
	{1, 0, 0, 0, 0}, // 36
	{1, 0, 0, 0, 0}, // 37
	{1, 0, 0, 1, 4}, // 38
	{1, 0, 0, 1, 4}, // 39
	{1, 0, 0, 1, 4}, // 40
	{1, 0, 0, 1, 4}, // 41
	{0, 0, 0, 0, 8}, // 42
	{0, 0, 0, 0, 1}, // 43
	{0, 0, 0, 0, 1}, // 44
	{1, 0, 0, 1, 0}, // 45
	{1, 0, 0, 1, 0}, // 46
	{1, 0, 0, 1, 0}, // 47
	{1, 0, 0, 1, 0}, // 48
	{1, 0, 0, 1, 0}, // 49
	{1, 0, 0, 1, 0}, // 50
	{1, 0, 0, 1, 0}, // 51
	{1, 0, 0, 1, 0}, // 52
	{0, 1, 0, 0, 8}, // 53
	{0, 0, 0, 0, 1}, // 54
	{0, 0, 0, 0, 1}, // 55
	{0, 0, 0, 0, 1}  // 56
};

unsigned char automap_cols[10] = {
	140, // 0: Room wall
	192, // 1: Floor
	100, // 2: Passage
	255, // 3: Void
	120, // 4: Locked passage
	230, // 5: Checkpoint
	240, // 6: Closed chest
	200, // 7: Open chest
	230, // 8: Magic tile
	0  // 9: --

};
