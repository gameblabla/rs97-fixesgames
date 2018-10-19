#! /usr/bin/perl
#
# convert2chroma.pl
# Convert various other games into a format playable by Chroma
#
# The following inputs are recognised:
#
# BBC, BBC Designer     - disc image, memory snapshot
# C64                   - memory snapshot
# Spectrum              - tape image, memory snapshot
# CPC                   - memory snapshot
# Atari                 - memory snapshot
# Amiga                 - xor_lib directory
#
###############################################################################
#
# Copyright (C) 2010 Amf
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version. 
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
###############################################################################
#
# BBC levels
# ==========
# The BBC XOR Designer file format is as follows:
# 
#     in file                                                     in memory
#     0x000 - 0x1c1 Map data                                      0x900 - 0xbff
#     0x1c2 - 0x1c3 Address of top left map piece - 32            0x78 - 0x79
#     0x1c4 - 0x1c5 Address of top right map piece - 32           0x7a - 0x7b
#     0x1c6 - 0x1c7 Address of bottom left map piece - 32         0x7c - 0x7d
#     0x1c8 - 0x1c9 Address of bottom right map piece - 32        0x7e - 0x7f
#     0x1ca - 0x1cb Address of Player1 - 32                       0x80 - 0x81
#     0x1cc - 0x1cd Offset of Player1 from View1 (x,y)            0x82 - 0x83
#     0x1ce - 0x1cf View1 (x,y)                                   0x84 - 0x85
#     0x1d0 - 0x1d1 Address of View1                              0x86 - 0x87
#     0x1d2 - 0x1d3 Address of Player2 - 32                       0x88 - 0x89
#     0x1d4 - 0x1d5 Offset of Player2 from View2 (x,y)            0x8a - 0x8b
#     0x1d6 - 0x1d7 View2 (x, y)                                  0x8c - 0x8d
#     0x1d8 - 0x1d9 Address of View2                              0x8e - 0x8f
#     0x1da         Current player                 (must be 0x02) 0x90
#     0x1db         Unused - Other player alive   (ignored, 0x02) 0x91
#     0x1dc         Masks total                                   0x92
#     0x1dd         Masks collected                (must be 0x00) 0x93
#     0x1de         Unused - List length           (must be 0x00) 0x94
#     0x1df         Unused - Explosion number     (ignored, 0x00) 0x95
#     0x1e0         Unused - Switched             (ignored, 0x00) 0x96
#     0x1e1         Unused                                 (0x00) 0x97
#     0x1e2 - 0x1e3 Address of Teleport1                          0x98 - 0x99
#     0x1e4 - 0x1e5 Offset of Teleport1 from ViewTeleport1 (x,y)  0x9a - 0x9b
#     0x1e6 - 0x1e7 ViewTeleport1 (x,y)                           0x9c - 0x9d
#     0x1e8 - 0x1e9 Address of ViewTeleport1                      0x9e - 0x9f
#     0x1ea - 0x1eb Address of Teleport2                          0xa0 - 0xa1
#     0x1ec - 0x1ed Offset of Teleport2 from ViewTeleport2 (x,y)  0xa2 - 0xa3
#     0x1ee - 0x1ef ViewTeleport2 (x,y)                           0xa4 - 0xa5
#     0x1f0 - 0x1f1 Address of ViewTeleport2                      0xa6 - 0xa7
#     0x1f2 - 0x1ff Unused                                 (0x00)
#     0x200 - 0x211 Title
#     0x220 - 0x283 Completion message
#
# For BBC XOR, the level data is split into two chunks, 0x000 - 0x1ff and
# 0x200 - 0x211, located in different parts of the file. There is no
# completion message.
#
# In memory, the map data is stored in a 32 * 32 byte array, starting at 0x900.
# The address for the piece at (x, y) is therefore 0x900 + x + (32 * y). In the
# file, the map data defines only the inner grid from (1,1) to (30,30). It runs
# from left to right, top to bottom, and should be read as nibbles, with the
# least significant nibble coming first. The nibbles translate as follows:
# 
#     0x0  Space                          PIECE_SPACE
#     0x1  Wall                    %      PIECE_WALL
#     0x2  Magus                   1      PIECE_PLAYER_ONE 
#     0x3  Quaestor                2      PIECE_PLAYER_TWO
#     0x4  Map                     MmNn   PIECE_MAP_[TOP|BOTTOM]_[LEFT|RIGHT]
#     0x5  Dots                    -      PIECE_DOTS_X
#     0x6  Waves                   |      PIECE_DOTS_Y
#     0x7  Fish                    d      PIECE_ARROW_RED_DOWN
#     0x8  Chicken                 a      PIECE_ARROW_RED_LEFT
#     0x9  H Bomb                  D      PIECE_BOMB_RED_DOWN
#     0xa  V Bomb                  A      PIECE_BOMB_RED_LEFT
#     0xb  Mask                    *      PIECE_STAR
#     0xc  Door                    /      PIECE_DOOR
#     0xd  Fat Doll                o      PIECE_CIRCLE
#     0xe  Beam Me Up Scotty       T      PIECE_TELEPORT
#     0xf  Switch                  S      PIECE_SWITCH
#
# The map pieces are stored ambiguously in the map data. To resolve this, the
# address of each map piece is stored separately in 0x1c2 - 0x1c9. Note that
# these addresses, as well as those for the players, are offset by 32 bytes to
# allow for relative addressing - disassembling the code makes this clearer.
#
# Contrary to what the value of 0x1da might suggest, Quaestor moves first,
# thanks to a call to 0x17b7 when setting up the level - this is a hack.
# 
# The completion message is XORed with 0x7e to prevent casual reading.
# 
# There are fourteen common levels shared between BBC XOR and BBC XOR Designer:
# 
#         title                         differences
#      1: Dots And Waves                View2
#      2: Something Fishy
#      3: Chicken Supreme               View1 View2 
#      4: Explosive Mixture             View1
#      5: Henrys Anguish                View1 View2 
#      6: The Dolls House               View1 View2 
#      7: Dollys Revenge                View1 View2 
#      8: Enlightenment                 View2
#      9: The Challenge                 View1 View2 
#     10: Patience Pending              View1 View2 ViewTeleport1 ViewTeleport2
#     11: Razor Edge                    View2 ViewTeleport1 ViewTeleport2
#     12: The Happy Hour                View2 ViewTeleport1
#     13: Deja Vu                       View2
#     14: Penultimate                   View1 View2 ViewTeleport1
# 
# BBC XOR has a fifteenth level not present in XOR Designer:
# 
#     15: The Decoder
# 
# BBC XOR Designer has two hidden levels which aren't directly accessible:
# 
#     x1: Wheel Of Fortune
#     x2: The Hidden Cavern
# 
# BBC graphics
# ============
# The XOR Designer graphics format contains 16 sprites, following the order
# used for map data. Each sprite is stored in 72 bytes, being 12 x 24 pixels
# in standard BBC four colour mode. For XOR, the sprites for the walls are
# stored separately to the sprites for the pieces. There are also two extra
# wall sprites which we add to the end of the standard 16.
#
# Commodore 64 levels and graphics
# ================================
# The Commodore 64 XOR levels and graphics are stored in an identical format
# to those of the BBC version, and thus may be extracted in a similar way.
# Again, Quaestor move first. There are no differences between the levels and
# those of the BBC version.
#
# ZX Spectrum levels
# ==================
# The ZX Spectrum version splits the level data into two different chunks -
# the map data itself, and the location data, which takes the following format:
# 
#     0x00 - 0x01 Player1 (x, y)
#     0x02 - 0x03 Address of Player1
#     0x04 - 0x05 Offset of Player1 from View1 (x, y)
#     0x06 - 0x07 Address of View1
#     0x08 - 0x09 Player2 (x, y)
#     0x0a - 0x0b Address of Player2
#     0x0c - 0x0d Offset of Player2 from View1 (x, y)
#     0x0e - 0x0f Address of View2
#     0x10        Masks total (binary packed decimal)
#     0x11 - 0x02 Address of Teleport2
#     0x13 - 0x14 Teleport1 (x, y)
#     0x15 - 0x16 Address of Teleport1
#     0x17 - 0x18 Offset of Teleport1 from ViewTeleport1 (x, y)
#     0x19 - 0x1a Address of ViewTeleport1
#     0x1b - 0x1c Address of Teleport1
#     0x1d - 0x1e Teleport2 (x, y)
#     0x1f - 0x20 Address of Teleport2
#     0x21 - 0x22 Offset of Teleport2 from ViewTeleport2 (x, y)
#     0x23 - 0x24 Address of ViewTeleport2
#     0x25 - 0x26 Address of top left map piece
#     0x25 - 0x26 Address of top right map piece
#     0x25 - 0x26 Address of bottom left map piece
#     0x25 - 0x26 Address of bottom right map piece
#     0x2d - 0x2e Address of map data
#
# The map data is stored as per BBC XOR, and is unpacked to memory at 0xe500.
# The address for the piece at (x, y) is therefore 0xe500 + x + (32 * y).
# Unlike the BBC version, none of the addresses are offset in any way, and
# Magus is the first to move.
#
#         title                         differences from the BBC version
#      1: Dots And Waves                View1 View2
#      2: Something Fishy               1 (extra wall)
#      3: Chicken Supreme              
#      4: Explosive Mixture             3 (maps swapped), View1
#      5: Henrys Anguish                 
#      6: The Dolls House                
#      7: Dollys Revenge                 
#      8: Enlightenment                   
#      9: The Challenge                 View1 View2
#     10: Patience Pending              
#     11: Razor Edge                    2 (maps swapped)
#     12: The Happy Hour                
#     13: Deja Vu                       
#     14: Penultimate                   2 (players swapped), View1 View2
#     15: The Decoder
#
# Amstrad CPC levels
# ==================
# The Amstrad CPC XOR levels are stored in a similar format to those of the
# ZX Spectrum version, with the exception that the map data is unpacked to
# memory at 0x3c00, which affects the addresses stored in the location data.
# Again, Magus is the first to move.
#
#         title                         differences from the BBC version
#      1: Dots And Waves                View1 View2
#      2: Something Fishy               1 (extra wall)
#      3: Chicken Supreme
#      4: Explosive Mixture             3 (maps swapped), View1
#      5: Henrys Anguish
#      6: The Dolls House
#      7: Dollys Revenge
#      8: Enlightenment
#      9: The Challenge                 View1 View2
#     10: Patience Pending                
#     11: Razor Edge                    2 (maps swapped)
#     12: The Happy Hour                  
#     13: Deja Vu
#     14: Penultimate                   2 (players swapped), View1 View2
#     15: The Decoder                   10 (corruption of final line)
#
#         title                         differences from the Spectrum version
#     15: The Decoder                   10 (corruption of final line)
#
# Atari ST levels
# ===============
# The Atari ST level format is as follows:
#
#     0x000 - 0x3ff Map data
#     0x400 - 0x401 View1x
#     0x402 - 0x403 View1y
#     0x404 - 0x405 Offset from View1x to Player1x
#     0x406 - 0x407 Offset from View1y to Player1y
#     0x408 - 0x409 0x00 0x11
#     0x40a - 0x40b View2x
#     0x40c - 0x40d View2y
#     0x40e - 0x40f Offset from View2x to Player2x
#     0x410 - 0x411 Offset from View2y to Player2y
#     0x412 - 0x413 0x00 0x12
#     0x414 - 0x415 Masks total
#     0x416 - 0x417 ViewTeleport1x
#     0x418 - 0x419 ViewTeleport1y
#     0x41a - 0x41b 0x00 0x00
#     0x41c - 0x41d Address of Teleport1
#     0x41e - 0x41f Offset from ViewTeleport1x to Teleport1x
#     0x420 - 0x421 Offset from ViewTeleport1y to Teleport1y
#     0x422 - 0x423 ViewTeleport2x
#     0x424 - 0x425 ViewTeleport2y
#     0x426 - 0x427 0x00 0x00
#     0x428 - 0x429 Address of Teleport2
#     0x42a - 0x42b Offset from ViewTeleport2x to Teleport2x
#     0x42c - 0x42d Offset from ViewTeleport2y to Teleport2y
#     0x42e - 0x431 0x1b 0x59 0x21 0x23
#     0x432 - 0x445 Title
#
# The address for (x,y) is stored as x + (32 * y) - there is no offset.
# The map data defines the whole grid from (0, 0) to (31, 31). It runs from
# left to right, then from top to bottom. It should be read as bytes, which
# are translated as follows:
#
#     0x00        Space                     PIECE_SPACE
#     0x01        Wall               %      PIECE_WALL
#     0x02        Magus              1      PIECE_PLAYER_ONE
#     0x03        Quaestor           2      PIECE_PLAYER_TWO
#     0x04        Map top left       M      PIECE_MAP_TOP_LEFT
#     0x05        Dots               -      PIECE_DOTS_X
#     0x06        Waves              |      PIECE_DOTS_Y
#     0x07        Fish               d      PIECE_ARROW_RED_DOWN
#     0x08        Chicken            a      PIECE_ARROW_RED_LEFT
#     0x09        H Bomb             D      PIECE_BOMB_RED_DOWN
#     0x0a        V Bomb             A      PIECE_BOMB_RED_LEFT
#     0x0b        Mask               *      PIECE_STAR
#     0x0c        Door               /      PIECE_DOOR
#     0x0d        Fat Doll           o      PIECE_CIRCLE
#     0x0e        Map top right      m      PIECE_MAP_TOP_RIGHT
#     0x0f        Map bottom left    N      PIECE_MAP_BOTTOM_LEFT
#     0x10        Map bottom right   n      PIECE_MAP_BOTTOM_RIGHT
#     0x11        Beam Me Up Scotty  T      PIECE_TELEPORT
#     0x12        Switch             S      PIECE_SWITCH
#     0x13        Fish               d      PIECE_ARROW_RED_DOWN
#     0x14        Chicken            a      PIECE_ARROW_RED_LEFT
#     0x15        Chicken            a      PIECE_ARROW_RED_LEFT
#     0x16        Fish               d      PIECE_ARROW_RED_DOWN
#     0x17        Fish               d      PIECE_ARROW_RED_DOWN
#     0x18        Chicken            a      PIECE_ARROW_RED_LEFT
#     0x19        Chicken            a      PIECE_ARROW_RED_LEFT
#     0x1a        Fish               d      PIECE_ARROW_RED_DOWN
#     0x21 - 0x5c Wall               %      PIECE_WALL
#     0x61        End piece          ?      PIECE_UNKNOWN
#     0x62        Wall               %      PIECE_WALL
#
# Magus moves first.
#
#         title                         differences from the BBC version
#      1: Dots And Waves                1 View1 View2
#      2: Something Fishy               3 (players swapped)
#      3: Chicken Supreme
#      4: Explosive Mixture             
#      5: Henrys Anguish                2 (players swapped) View1 View2
#      6: The Dolls House
#      7: Dollys Revenge
#      8: Enlightenment
#      9: The Challenge                 1 View1 View2
#     10: Patience Pending              ViewTeleport1 ViewTeleport2
#     11: Razor Edge                    
#     12: The Happy Hour                View2 ViewTeleport1 ViewTeleport2
#     13: Deja Vu
#     14: Penultimate                   View1 View2 ViewTeleport1
#     15: The Decoder                   Ending ViewTeleport1 ViewTeleport2
#
# Amiga levels
# ============
# Amiga .maze files have fifteen levels, sequentially, each with the following
# format:
# 
#     0x000 - 0x01e Title
#     0x01f - 0x41e Map Data
#     0x420 - 0x423 View 1 x
#     0x424 - 0x427 View 1 y
#     0x428 - 0x42b Player 1 x
#     0x42c - 0x42f Player 1 y
#     0x430 - 0x433 View 2 x
#     0x434 - 0x437 View 2 y
#     0x438 - 0x43b Player 2 x
#     0x43c - 0x43f Player 2 y
#     0x440 - 0x443 Mask Total
#     0x444 - 0x447 ViewTeleport 1 x
#     0x448 - 0x44b ViewTeleport 1 y
#     0x44c - 0x44f Teleport 1 x
#     0x450 - 0x453 Teleport 1 y
#     0x454 - 0x457 ViewTeleport 1 x
#     0x458 - 0x45b ViewTeleport 1 y
#     0x45c - 0x45f Teleport 1 x
#     0x460 - 0x463 Teleport 1 y
# 
# The map data defines the whole grid from (0, 0) to (31, 31). It runs from
# left to right, then from top to bottom. It should be read as bytes; the
# lowest six bits of each byte determine the piece:
# 
#     0x00  Space                        PIECE_SPACE
#     0x01  Magus                 1      PIECE_PLAYER_ONE
#     0x02  Quaestor              2      PIECE_PLAYER_TWO 
#     0x03  Fish                  v      PIECE_ARROW_RED_DOWN
#     0x04  Chicken               <      PIECE_ARROW_RED_LEFT
#     0x05  Fat Doll              o      PIECE_CIRCLE
#     0x06  Mask                  *      PIECE_STAR
#     0x07  Switch                S      PIECE_SWITCH
#     0x08  Map TL                M      PIECE_MAP_TOP_LEFT
#     0x09  Map TR                m      PIECE_MAP_TOP_RIGHT
#     0x0a  Map BL                N      PIECE_MAP_BOTTOM_LEFT
#     0x0b  Map BR                n      PIECE_MAP_BOTTOM_RIGHT
#     0x0c  Beam Me Up Scotty     T      PIECE_TELEPORT
#     0x0d  Waves                 |      PIECE_EARTH_Y
#     0x0e  Dots                  -      PIECE_EARTH_X
#     0x0f  V Bomb                Z      PIECE_BOMB_RED_LEFT
#     0x10  H Bomb                X      PIECE_BOMB_RED_DOWN
#     0x11  Door                  /      PIECE_DOOR
#     0x20  Wall                  %      PIECE_WALL
#
# Quaestor is the first to move.
# 
# There are two sets of fifteen levels:
# 
#     XOR's Mazes:
# 
#         Amiga title        BBC equivalent     differences from BBC version
#      1: Olaf's Warm Up     Dots and Waves     2 View1 View2
#      2: Rocks & Zeppelins  Something Fishy    3 (players swapped)
#      3: Something Tricky   Chicken Supreme    
#      4: Explosive Mixture  Explosive Mixture  
#      5: Erik's Anguish     Henrys Anguish     3 (players swapped) View1 View2
#      6: The Ball's House   The Doll's House   
#      7: Rolling Revenge    Dollys Revenge     
#      8: Enlightenment      Enlighenment       
#      9: The Challenge      The Challenge      1 View1 View2
#     10: Patience Pending   Patience Pending   ViewTeleport1 ViewTeleport2
#     11: Razor Edge         Razor Edge         8 (last line corrupted)
#     12: The Happy Hour     The Happy Hour     View2 ViewTeleport1 ViewTeleport2
#     13: Deja Vu            Deja Vu            
#     14: Penultimate        Penultimate        View1 View ViewTeleport2
#     15: The Decoder        The Decoder        Ending ViewTeleport1 ViewTeleport2
#  
#
#     Procyon's Mazes:
# 
#         Amiga title        BBC equivalent     differences from BBC version
#      1: Taurus                  
#      2: Andromeda                       
#      3: Perseus                         
#      4: Cameloparous                    
#      5: Ursa Nagor                            insoluble!
#      6: Draco                   
#      7: Hercules                        
#      8: Delphinus                       
#      9: Aquila                  
#     10: Ophiuchus          Wheel of Fortune   2 View1 View2 ViewTeleport1 ViewTeleport2
#     11: Corvus                  
#     12: Canis Maior                     
#     13: Hydrus                  
#     14: Vela Carina                           insoluble!
#     15: Crux               The Hidden Cavern  View1 View2 ViewTeleport2
# 
# Amiga Graphics
# ==============
# There are fifteen sets of 20 background sprites in the "vimages*" files. The
# format begins with a 32 colour palette, with each entry taking two bytes, of
# the form 0x0R 0xGB. This is followed by a number of 960 byte blocks, with
# each block encoding one bit of colour data for ten sprites. Within each
# block, the first three bytes of a word encode 24 pixels for a sprite. The
# first two blocks encode bit 1 for sprites 0 - 9 and 10 - 19, the next two
# encode bit 2, and so on.
# 
# The "cimages" file contains one set of 60 character sprites. It takes a
# similar format to the "vimages*" files, but contains no palette information.
# 
# Enigma
# ======
# Enigma has five different wall pieces, but these are merely cosmetic. We
# merge them all into the single wall piece present in Chroma. Enigma also has
# "certain death" piece, which we also replace with a wall, on the basis that 
# in a successful game, the player will interact with this piece in exactly the
# same manner as a wall.
# 

@commands = (
        # Convert XOR levels and graphics
	["bbc", \&convert_bbc_all],
	["designer", \&convert_designer_all],
	["c64", \&convert_c64_all],
	["spectrum", \&convert_spectrum_all],
	["cpc", \&convert_cpc_all],
	["atari", \&convert_atari_all],
	["amiga", \&convert_amiga_all],

	# Convert XOR levels
	["bbc-levels", \&convert_bbc_levels],
	["c64-levels", \&convert_bbc_levels],
	["designer-levels", \&convert_designer_levels],
	["designer-level-file", \&convert_designer_level],
	["spectrum-levels", \&convert_spectrum_levels],
	["cpc-levels", \&convert_spectrum_levels],
	["atari-levels", \&convert_atari_levels],
	["amiga-levels", \&convert_amiga_levels],

	# Convert XOR graphics
	["bbc-graphics", \&convert_bbc_graphics],
	["c64-graphics", \&convert_c64_graphics],
	["designer-graphics", \&convert_bbc_graphics],
	["designer-graphics-file", \&convert_bbc_graphics_file],
	["spectrum-graphics", \&convert_spectrum_graphics],
	["cpc-graphics", \&convert_cpc_graphics],
	["atari-graphics", \&convert_atari_graphics],
	["amiga-graphics", \&convert_amiga_graphics],
	["amiga-graphics-file", \&convert_amiga_graphics_file],

        # Convert BBC XOR music
	["bbc-music", \&convert_bbc_music],
        # Patch BBC XOR disc image to show all levels
	["bbc-patch", \&convert_bbc_patch],
        # Patch Amiga disc image to avoid manual "copy protection" check
	["amiga-patch", \&convert_amiga_patch],

        # Convert Chroma XOR levels back into 8bit binary files
	["chroma2xor", \&convert_chroma2xor],

        # Convert third party XOR files
        ["zanten-file", \&convert_zanten_level], # http://www.rvvz.demon.nl/xor/
        ["ovine-file", \&convert_ovine_level],   # http://xor.ovine.net/

        # Convert Engima level(s)
	["enigma", \&convert_enigma],
	["enigma-file", \&convert_enigma_file],
	["chroma2enigma", \&convert_chroma2enigma]

	);

@titles = (
	"Dots and Waves",
        "Something Fishy",
        "Chicken Supreme",
        "Explosive Mixture",
        "Henrys Anguish",
        "The Dolls House",
        "Dollys Revenge",
        "Enlightenment",
        "The Challenge",
        "Patience Pending",
        "Razor Edge",
        "The Happy Hour",
        "Deja Vu",
        "Penultimate",
	"The Decoder"
	);

if($#ARGV >= 2)
{
    for($i = 0; $i <= $#commands; $i ++)
    {
	if($ARGV[0] eq "--".$commands[$i][0])
	{
	    $command = $commands[$i][1];
	    &$command($ARGV[1], $ARGV[2]);
	    exit(0);
	}
    }
}

print STDERR << "EOF";
Usage: convert2chroma.pl <mode> <input> <output>

    convert2chroma.pl --bbc <input> <output>		# BBC XOR
    convert2chroma.pl --designer <input> <output>	# BBC XOR Designer
    convert2chroma.pl --c64 <input> <output>		# Commodore 64 XOR
    convert2chroma.pl --spectrum <input> <output>	# Spectrum XOR
    convert2chroma.pl --cpc <input> <output>		# Amstrad CPC XOR
    convert2chroma.pl --amiga <input> <output>		# Amiga XOR
    convert2chroma.pl --atari <input> <output>		# Amiga ST XOR

    convert2chroma.pl --enigma <input> <output>		# Enigma

There are various other options available; see inside the script for details.

EOF

exit(0);

###############################################################################

sub read_file
{
    my $file = shift(@_);
    my $data;

    open(INPUT, $file) || die "Couldn't open file ($file): $!\n";
    binmode INPUT;
    undef $/;
    $data = <INPUT>;
    close INPUT;

    return $data;
}

sub locate_string
{
    my $data = shift(@_);
    my $string = shift(@_);
    my $name = shift(@_);
    my $index;

    $index = index($$data, $string);
    
    if($index != -1 && index($$data, $string, $index + 1) != -1)
    {
	print "Ambiguous match";
	return -2;
    }

    return $index;
}

sub byte
{
    my $data = shift(@_);
    my $offset = shift(@_);

    return ord(substr($$data, $offset, 1));
} 

sub short
{
    my $data = shift(@_);
    my $offset = shift(@_);

    return ord(substr($$data, $offset, 1)) + 256 * ord(substr($$data, $offset + 1, 1));
}

sub long
{
    my $data = shift(@_);
    my $offset = shift(@_);

    return ord(substr($$data, $offset + 3, 1)) + 256 * ord(substr($$data, $offset + 2, 1));
}

sub convert_address
{
    my $address = shift(@_);
    my $x = shift(@_);
    my $y = shift(@_);

    $$x = $address % 32;
    $$y = int($address / 32);
}

sub make_directory
{
    my $directory = shift(@_);

    if(! -e $directory)
    {   
        mkdir($directory, 0755) || die "Couldn't make directory ($directory): $!\n";
    }
}

###############################################################################

sub create_level
{
    my $level = shift(@_);
    my $file_output = shift(@_);

                   #  12M
    $translation = " %???-|daDA*/oTS";

    print "Creating level '$file_output': ";

    for($j = 0; $j < 32; $j ++)
    {
        for($i = 0; $i < 32; $i ++)
        {
            $map[$i][$j] = "%";
        }   
    }

    $title2number{"Andromeda"} = 2;
    $title2number{"Aquila"} = 9;
    $title2number{"Cameloparous"} = 4;
    $title2number{"Canis Maior"} = 12;
    $title2number{"Chicken Supreme"} = 3;
    $title2number{"Corvus"} = 11;
    $title2number{"Crux"} = 15;
    $title2number{"Deja Vu"} = 13;
    $title2number{"Delphinus"} = 8;
    $title2number{"Dollys Revenge"} = 7;
    $title2number{"Dots And Waves"} = 1;
    $title2number{"Dots and Waves"} = 1;
    $title2number{"Draco"} = 6;
    $title2number{"Enlightenment"} = 8;
    $title2number{"Erik's Anguish"} = 5;
    $title2number{"Explosive Mixture"} = 4;
    $title2number{"Henrys Anguish"} = 5;
    $title2number{"Hercules"} = 7;
    $title2number{"Hydrus"} = 13;
    $title2number{"Olaf's Warm Up"} = 1;
    $title2number{"Ophiuchus"} = 10;
    $title2number{"Patience Pending"} = 10;
    $title2number{"Penultimate"} = 14;
    $title2number{"Perseus"} = 3;
    $title2number{"Razor Edge"} = 11;
    $title2number{"Rocks & Zeppelins"} = 2;
    $title2number{"Rolling Revenge"} = 7;
    $title2number{"Something Fishy"} = 2;
    $title2number{"Something Tricky"} = 3;
    $title2number{"Taurus"} = 1;
    $title2number{"The Ball's House"} = 6;
    $title2number{"The Challenge"} = 9;
    $title2number{"The Decoder"} = 15;
    $title2number{"The Dolls House"} = 6;
    $title2number{"The Happy Hour"} = 12;
#   $title2number{"The Hidden Cavern"} = 15;
    $title2number{"Ursa Nagor"} = 5;
    $title2number{"Vela Carina"} = 14;
#   $title2number{"Wheel Of Fortune"} = 10;

    # Populate map from data
    $teleports = 0;

    # Compressed map - four bits per piece
    if($$level{"mode"} eq "8bit")
    {
        $offset = 0;
        for($j = 1; $j < 31; $j ++)
        {   
            for($i = 1; $i < 31; $i += 2)
            {   
                $c = byte(\$$level{"map"}, $offset);
                $offset ++;

                $c1 = $c % 16;
                $c2 = ($c - %c1) / 16;

                $map[$i][$j] = substr($translation, $c1, 1);
                $map[$i + 1][$j] = substr($translation, $c2, 1);

                if($map[$i][$j] eq "T" || $map[$i + 1][$j] eq "T")
                {
                    $teleports ++;
                }
            }
        }
    }

    # Uncompressed map - one byte per piece
    if($$level{"mode"} eq "amiga")
    {
	$offset = 0;
        for($j = 0; $j < 32; $j ++)
        {
            for($i = 0; $i < 32; $i ++)
            {   
                $c = byte(\$$level{"map"}, $offset);
                $c = $c & 0x3f; # ?
                $offset ++;
                $m = "?";

                $m = " " if($c == 0x00);
                $m = "1" if($c == 0x01);
                $m = "2" if($c == 0x02);
                $m = "d" if($c == 0x03);
                $m = "a" if($c == 0x04);
                $m = "o" if($c == 0x05);
                $m = "*" if($c == 0x06);
                $m = "S" if($c == 0x07);
                $m = "M" if($c == 0x08);
                $m = "m" if($c == 0x09);
                $m = "N" if($c == 0x0a);
                $m = "n" if($c == 0x0b);
                $m = "T" if($c == 0x0c);
                $m = "|" if($c == 0x0d);
                $m = "-" if($c == 0x0e);
                $m = "A" if($c == 0x0f);
                $m = "D" if($c == 0x10);
                $m = "/" if($c == 0x11);
                $m = "%" if($c == 0x20);

                if($m eq "T")
                {   
                    $teleports ++;
                }

                if($m eq "?")
                {
                    print "FAILED\n  Unknown map piece 0x".sprintf("%02x", $c)." at ($i, $j)\n";
		    return;
                }

                $map[$i][$j] = $m;
            }
	}
    }

    if($$level{"mode"} eq "atari")
    {
        for($j = 0; $j < 32; $j ++)
        {
  	    for($i = 0; $i < 32; $i ++)
	    {
	        $p = "?";
	        $c = byte(\$level_data, $j*32+$i);

	        $p = " " if($c == 0x00);
    	        $p = "1" if($c == 0x02);
	        $p = "2" if($c == 0x03);
	        $p = "M" if($c == 0x04);
	        $p = "-" if($c == 0x05);
	        $p = "|" if($c == 0x06);
	        $p = "d" if($c == 0x07 || $c == 0x13 || $c == 0x16 || $c == 0x17 || $c == 0x1a);
	        $p = "a" if($c == 0x08 || $c == 0x14 || $c == 0x15 || $c == 0x18 || $c == 0x19);
	        $p = "D" if($c == 0x09);
	        $p = "A" if($c == 0x0a);
	        $p = "*" if($c == 0x0b);
	        $p = "/" if($c == 0x0c);
	        $p = "o" if($c == 0x0d);
	        $p = "m" if($c == 0x0e);
	        $p = "N" if($c == 0x0f);
	        $p = "n" if($c == 0x10);
	        $p = "T" if($c == 0x11);
	        $p = "S" if($c == 0x12);

	        $p = "%" if($c == 0x55 || $c==0x56 || $c == 0x57 || $c == 0x58); # 1
    	        $p = "%" if($c == 0x49 || $c==0x4a || $c == 0x4b || $c == 0x4c); # 2
	        $p = "%" if($c == 0x4d || $c==0x4e || $c == 0x4f || $c == 0x50); # 3
	        $p = "%" if($c == 0x51 || $c==0x52 || $c == 0x53 || $c == 0x54); # 4
	        $p = "%" if($c == 0x2d || $c==0x2e || $c == 0x2f || $c == 0x30 || $c == 0x62); # 5 
	        $p = "%" if($c == 0x31 || $c==0x32 || $c == 0x33 || $c == 0x34); # 6
	        $p = "%" if($c == 0x27 || $c==0x28 || $c == 0x29 || $c == 0x2a); # 7
	        $p = "%" if($c == 0x25 || $c==0x26 || $c == 0x2b || $c == 0x2c); # 8
	        $p = "%" if($c == 0x01 || $c == 0x21 || $c==0x22 || $c == 0x23 || $c == 0x24); # 9
	        $p = "%" if($c == 0x35 || $c==0x36 || $c == 0x37 || $c == 0x38); # 10
	        $p = "%" if($c == 0x39 || $c==0x3a || $c == 0x3b || $c == 0x3c); # 11
	        $p = "%" if($c == 0x3d || $c==0x3e || $c == 0x3f || $c == 0x40); # 12
	        $p = "%" if($c == 0x41 || $c==0x42 || $c == 0x43 || $c == 0x44); # 13
	        $p = "%" if($c == 0x45 || $c==0x46 || $c == 0x47 || $c == 0x48); # 14
	        $p = "%" if($c == 0x59 || $c==0x5a || $c == 0x5b || $c == 0x5c); # 15

	        $p = "?" if($c == 0x61);

	        $teleports ++ if($p eq "T");

	        $map[$i][$j] = $p;
	    }
        }
    }

    # Populate players
    for($i = 1; $i <=2; $i ++)
    {
	$p = $map[$$level{"player".$i."x"}][$$level{"player".$i."y"}];
	if($p ne $i && $p ne " " && $p ne "?")
        {
	    print "FAILED\n  player $i is unmatched (map says '".$map[$$level{"player".$i."x"}][$$level{"player".$i."y"}]."')\n";
	    return;
        }
        $map[$$level{"player".$i."x"}][$$level{"player".$i."y"}] = $i;
    }

    # Populate maps
    if($$level{"mode"} eq "8bit")
    {
        foreach $i ("TL", "TR", "BL", "BR")
        {
            if($map[$$level{"map".$i."x"}][$$level{"map".$i."y"}] ne "?")
            {
    	        print "FAILED\n  map $i is unmatched (map says '".$map[$$level{"map".$i."x"}][$$level{"map".$i."y"}]."')\n";
	        return;
            }
        }
        $map[$$level{"mapTLx"}][$$level{"mapTLy"}] = "M";
        $map[$$level{"mapTRx"}][$$level{"mapTRy"}] = "m";
        $map[$$level{"mapBLx"}][$$level{"mapBLy"}] = "N";
        $map[$$level{"mapBRx"}][$$level{"mapBRy"}] = "n";
    }

    # Sanity check
    $failed = 0;
    for($j = 1; $j < 31; $j ++)
    {   
        for($i = 1; $i < 31; $i ++)
        {   
            if($map[$i][$j] eq "<" || $map[$i][$j] eq "Z")
            {
                if($map[$i-1][$j] eq " " || $map[$i-1][$j] eq "-")
                {
                    $failed ++;
                }
            }
            if($map[$i][$j] eq "v" || $map[$i][$j] eq "X")
            {
                if($map[$i][$j+1] eq " " || $map[$i][$j+1] eq "|")
                {
                    $failed ++;
                }
            }
        }
    }
    if($failed > 0)
    {
        print "FAILED\n  $failed unsupported pieces\n";
        return;
    }

    open(OUTPUT, ">".$file_output) || die "Unable to open file ($file_output) for writing: $!\n";

    print OUTPUT "chroma level\n";
    print OUTPUT "mode: xor\n\n";
    print OUTPUT "title: XOR: ".$$level{"title"}."\n";
    print OUTPUT "size: 32 32\n";
    print OUTPUT "player: ".$$level{"player"}."\n" if($$level{"player"});
    if($title2number{$$level{"title"}})
    {
        print OUTPUT "level: ".$title2number{$$level{"title"}}."\n";
    }
    print OUTPUT "view1: ".$$level{"view1x"}." ".$$level{"view1y"}."\n";
    print OUTPUT "view2: ".$$level{"view2x"}." ".$$level{"view2y"}."\n";

    if($teleports > 0)
    {
        print OUTPUT "viewteleport1: ".$$level{"viewteleport1x"}." ".$$level{"viewteleport1y"}." (".$$level{"teleport1x"}." ".$$level{"teleport1y"}.")\n";
        print OUTPUT "viewteleport2: ".$$level{"viewteleport2x"}." ".$$level{"viewteleport2y"}." (".$$level{"teleport2x"}." ".$$level{"teleport2y"}.")\n";
    }

    if($message ne "")
    {
	print OUTPUT "message: $message\n";
    }

    print OUTPUT "\ndata:\n";

    for($j = 0; $j < 32; $j ++)
    {
        for($i = 0; $i < 32; $i ++)
        {
            print OUTPUT $map[$i][$j];
        }
        print OUTPUT "\n";
    }

    close(OUTPUT);

    print "OK\n";
}

sub create_set
{
    my $file_output = shift(@_);
    my $title = shift(@_);

    open(OUTPUT, ">".$file_output) || die "Unable to open file ($file_output) for writing: $!\n";

    print OUTPUT << "EOF";
<chroma type="set">
<head>
<title>$title</title>
</head>
</chroma>
EOF
    close(OUTPUT);
}

sub create_bmp
{
    my $image = shift(@_);
    my $file_output = shift(@_);
    my $scale_x = shift(@_);
    my $scale_y = shift(@_);

    $bpp = $$image{"colours"} <= 16 ? 4 : 8;

    $size_x = $$image{"x"} * $scale_x;
    $size_y = $$image{"y"} * $scale_y;
    $offset = 54 + ($$image{"colours"} * 4);
    $size_data = $size_x * $size_y * $bpp / 8;
    $size_file = $size_data + $offset;

    print "Creating graphic '$file_output'\n";

    open(BMP, ">".$file_output) || die "Unable to open file ($file_output) for writing: $!\n";
    binmode BMP;

    print BMP "BM";			# header
    print BMP pack("L", $size_file);	# file length
    print BMP pack("S", 0);		# reserved
    print BMP pack("S", 0);		# reserved
    print BMP pack("L", $offset);	# offset to data

    print BMP pack("L", 0x28);		        # size of BITMAPINFOHEADER
    print BMP pack("L", $size_x);	        # width
    print BMP pack("L", $size_y);	        # height
    print BMP pack("S", 1);		        # number of planes
    print BMP pack("S", $bpp);		        # bits per pixel
    print BMP pack("L", 0);		        # compression
    print BMP pack("L", $size_data);	        # image size in bytes
    print BMP pack("L", 0xb12); 	        # pixels per metre horizontal
    print BMP pack("L", 0xb12); 	        # pixels per metre vertical
    print BMP pack("L", $$image{"colours"});	# colours used
    print BMP pack("L", $$image{"colours"});	# colours important

    for($i = 0; $i < $$image{"colours"}; $i ++)
    {
	print BMP pack("L", $image{"palette"}[$i]);
    }

    for($y = $$image{"y"} - 1; $y >= 0; $y --)
    {
	for($ys = 0; $ys < $scale_y; $ys ++)
	{
	    $byte = 0; $bits = 0;
	    for($x = 0; $x < $$image{"x"} ; $x ++ )
	    {
		for($xs = 0; $xs < $scale_x; $xs ++)
		{
		    $byte = $byte * (2 ** $bits);
		    $byte += $$image{"pixels"}[$x][$y];
		    $bits += $bpp;

		    if($bits == 8)
		    {
		        print BMP pack("C", $byte);
			$byte = 0;
			$bits = 0;
		    }
		}
	    }
	    if($bits != 0)
	    {
		print BMP pack("C", $byte);
	    }
	}
    }

    close BMP;
}

sub create_graphics_set
{
    my $file_output = shift(@_);
    my $stub = shift(@_);
    my $title = shift(@_);
    my $size = shift(@_);

    open(OUTPUT, ">".$file_output) || die "Unable to open file ($file_output) for writing: $!\n";

    print OUTPUT << "EOF";
<chroma type="graphics">

<head>

<title>$title</title>

<sizes>
<!-- Everything is scaled from 24x24 -->
<size x="12" y="12" small="yes" pieces="yes" />
<size x="24" y="24" small="yes" pieces="yes" />
<size x="48" y="48" small="yes" pieces="yes" />
<size x="72" y="72" small="yes" pieces="yes" />
<size x="96" y="96" small="yes" pieces="yes" />
<size x="144" y="144" small="yes" pieces="yes" />
</sizes>

</head>

<background colour="#808080" />

<pieces path="$stub">

<piece name="space" scale="yes">
<image file="$stub\_00.bmp" />
</piece>

<piece name="wall" scale="yes">
<image file="$stub\_01.bmp" />
</piece>

<piece name="player_one" scale="yes">
<image file="$stub\_02.bmp" />
<image type="small" file="$stub\_02.bmp" />
</piece>

<piece name="player_two" scale="yes">
<image file="$stub\_03.bmp" />
<image type="small" file="$stub\_03.bmp" />
</piece>

<piece name="map_top_left" scale="yes">
<image file="$stub\_04.bmp" />
</piece>

<piece name="map_top_right" scale="yes">
<image file="$stub\_04.bmp" />
</piece>

<piece name="map_bottom_left" scale="yes">
<image file="$stub\_04.bmp" />
</piece>

<piece name="map_bottom_right" scale="yes">
<image file="$stub\_04.bmp" />
</piece>

<piece name="dots_x" scale="yes">
<image file="$stub\_05.bmp" />
</piece>

<piece name="dots_y" scale="yes">
<image file="$stub\_06.bmp" />
</piece>

<piece name="arrow_red_down" scale="yes">
<image file="$stub\_07.bmp" />
</piece>

<piece name="arrow_red_left" scale="yes">
<image file="$stub\_08.bmp" />
</piece>

<piece name="bomb_red_down" scale="yes">
<image file="$stub\_09.bmp" />
</piece>

<piece name="bomb_red_left" scale="yes">
<image file="$stub\_10.bmp" />
</piece>

<piece name="star" scale="yes">
<image file="$stub\_11.bmp" />
<image type="small" file="$stub\_11.bmp" />
</piece>

<piece name="door" scale="yes">
<image file="$stub\_12.bmp" />
<image type="small" file="$stub\_12.bmp" />
</piece>

<piece name="circle" scale="yes">
<image file="$stub\_13.bmp" />
</piece>

<piece name="teleport" scale="yes">
<image file="$stub\_14.bmp" />
</piece>

<piece name="switch" scale="yes">
<image file="$stub\_15.bmp" />
</piece>

<piece name="explosion_red_left" scale="yes">
<image colour="#ffffff" />
</piece>
<piece name="explosion_red_horizontal">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_right">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_top">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_vertical">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_bottom">
<clone piece="explosion_red_left" />
</piece>

<piece name="darkness" scale="yes">
<image colour="#000000" />
</piece>

</pieces>
</chroma>
EOF

    close(OUTPUT);
}

###############################################################################

sub convert_spectrum_levels
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    $data = read_file($file_input);

    # spectrum 0x9748
    $base = 0xe500; # Spectrum
    $offset_locations = locate_string(\$data, "\x47\x22\xe5\x12\x01\x32\xe5\x02") - 16;
    if($offset_locations < 0)
    {
	$base = 0x3c00; # CPC
        $offset_locations = locate_string(\$data, "\x47\x22\x3c\x12\x01\x32\x3c\x02") - 16;
	if($offset_locations < 0)
	{
	    die "Unable to locate locations in input file\n";
	}
    }

    # spectrum 0x6400
    $offset_maps= locate_string(\$data, "\x11\x11\x01\x10\x6b\x00\x40\x00") - 27;
    if($offset_maps < 0)
    {
	die "Unable to locate maps in input file\n";
    }

    for($number = 1; $number < 16; $number ++)
    {
	undef %level;
	$level{"map"} = substr($data, $offset_maps + (($number - 1) * 0x1c2), 0x1c2);
	$locations = substr($data, $offset_locations + (($number - 1) * 0x2f), 0x2f);

	$level{"player1x"} = byte(\$locations, 0x00);
	$level{"player1y"} = byte(\$locations, 0x01);
	convert_address(short(\$locations, 0x06) - $base, \$level{"view1x"}, \$level{"view1y"});
	$level{"player2x"} = byte(\$locations, 0x08);
	$level{"player2y"} = byte(\$locations, 0x09);
	convert_address(short(\$locations, 0x0e) - $base, \$level{"view2x"}, \$level{"view2y"});

	$level{"teleport1x"} = byte(\$locations, 0x13);
	$level{"teleport1y"} = byte(\$locations, 0x14);
	convert_address(short(\$locations, 0x19) - $base, \$level{"viewteleport1x"}, \$level{"viewteleport1y"});

	$level{"teleport2x"} = byte(\$locations, 0x1d);
	$level{"teleport2y"} = byte(\$locations, 0x1e);
	convert_address(short(\$locations, 0x23) - $base, \$level{"viewteleport2x"}, \$level{"viewteleport2y"});

	convert_address(short(\$locations, 0x25) - $base, \$level{"mapTLx"}, \$level{"mapTLy"});
	convert_address(short(\$locations, 0x27) - $base, \$level{"mapTRx"}, \$level{"mapTRy"});
	convert_address(short(\$locations, 0x29) - $base, \$level{"mapBLx"}, \$level{"mapBLy"});
	convert_address(short(\$locations, 0x2b) - $base, \$level{"mapBRx"}, \$level{"mapBRy"});

	$level{"title"} = $titles[$number - 1];

	$file_output = sprintf("%s%02d%s", $output_prefix, $number, ".chroma");

	$level{"mode"} = "8bit";

	create_level(\%level, $file_output);
    }
}

sub convert_bbc_levels
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    $data = read_file($file_input);

    $offset_data = locate_string(\$data, "\xb7\x09\x48\x09\x9d\x0c\x85\x0a") - 0x1c2;
    if($offset_data < 0)
    {
	die "Unable to locate maps in input file\n";
    }
    $offset_title = locate_string(\$data, "   DOTS AND WAVES   ");
    $offset_title ++;

    for($number = 1; $number < 16; $number ++)
    {
	$level_data = substr($data, $offset_data, 0x200);
        $offset_data -= 0x200;

	if($offset_title != 0)
	{
	    $level_data .= substr($data, $offset_title, 18);
	    $offset_title += 0x17;
	}
	else
	{
	    $level_data .= pack("A18", $titles[$number - 1]);
	}

        $file_output = sprintf("%s%02d%s", $output_prefix, $number, ".chroma");

        convert_bbc_level($level_data, $file_output);
    }

}

sub convert_designer_levels
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    $data = read_file($file_input);

    if($data =~ /(.{512}  DOTS AND WAVES  .{114})/s)
    {   
        convert_bbc_level($1, $output_prefix."01.chroma");
    }
    if($data =~ /(.{512} SOMETHING FISHY  .{114})/s)
    {   
        convert_bbc_level($1, $output_prefix."02.chroma");
    }
    if($data =~ /(.{512} CHICKEN SUPREME  .{114})/s)
    {   
        convert_bbc_level($1, $output_prefix."03.chroma");
    }
    if($data =~ /(.{512}EXPLOSIVE MIXTURE .{114})/s)
    {   
        convert_bbc_level($1, $output_prefix."04.chroma");
    }
    if($data =~ /(.{512}  HENRYS ANGUISH  .{114})/s)
    {   
        convert_bbc_level($1, $output_prefix."05.chroma");
    }
    if($data =~ /(.{512} THE DOLLS HOUSE  .{114})/s)
    {   
        convert_bbc_level($1, $output_prefix."06.chroma");
    }
    if($data =~ /(.{512}  DOLLYS REVENGE  .{114})/s)
    {   
        convert_bbc_level($1, $output_prefix."07.chroma");
    }
    if($data =~ /(.{512}  ENLIGHTENMENT   .{114})/s)
    {
        convert_bbc_level($1, $output_prefix."08.chroma");
    }
    if($data =~ /(.{512}  THE CHALLENGE   .{114})/s)
    {
        convert_bbc_level($1, $output_prefix."09.chroma");
    }
    if($data =~ /(.{512} PATIENCE PENDING .{114})/s)
    {
        convert_bbc_level($1, $output_prefix."10.chroma");
    }
    if($data =~ /(.{512}    RAZOR EDGE    .{114})/s)
    {
        convert_bbc_level($1, $output_prefix."11.chroma");
    }
    if($data =~ /(.{512}  THE HAPPY HOUR  .{114})/s)
    {
        convert_bbc_level($1, $output_prefix."12.chroma");
    }
    if($data =~ /(.{512}     DEJA VU      .{114})/s)
    {
        convert_bbc_level($1, $output_prefix."13.chroma");
    }
    if($data =~ /(.{512}   PENULTIMATE    .{114})/s)
    {
        convert_bbc_level($1, $output_prefix."14.chroma");
    }
    if($data =~ /(.{512} WHEEL OF FORTUNE .{114})/s)
    {
        convert_bbc_level($1, $output_prefix."x1.chroma");
    }
    if($data =~ /(.{512}THE HIDDEN CAVERN .{114})/s)
    {
        convert_bbc_level($1, $output_prefix."x2.chroma");
    }
}

sub convert_designer_level
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    $data = read_file($file_input);

    convert_bbc_level($data, $file_output);
}

sub convert_bbc_level
{
    my $level_data = shift(@_);
    my $file_output = shift(@_);

    undef %level;

    $level{"map"} = substr($level_data, 0, 0x1c2);

    $base = 0x900;
    convert_address(short(\$level_data, 0x1c2) - $base + 32, \$level{"mapTLx"}, \$level{"mapTLy"});
    convert_address(short(\$level_data, 0x1c4) - $base + 32, \$level{"mapTRx"}, \$level{"mapTRy"});
    convert_address(short(\$level_data, 0x1c6) - $base + 32, \$level{"mapBLx"}, \$level{"mapBLy"});
    convert_address(short(\$level_data, 0x1c8) - $base + 32, \$level{"mapBRx"}, \$level{"mapBRy"});

    convert_address(short(\$level_data, 0x1ca) - $base + 32, \$level{"player1x"}, \$level{"player1y"});
    convert_address(short(\$level_data, 0x1d0) - $base, \$level{"view1x"}, \$level{"view1y"});
    convert_address(short(\$level_data, 0x1d2) - $base + 32, \$level{"player2x"}, \$level{"player2y"});
    convert_address(short(\$level_data, 0x1d8) - $base, \$level{"view2x"}, \$level{"view2y"});

    convert_address(short(\$level_data, 0x1e2) - $base, \$level{"teleport1x"}, \$level{"teleport1y"});
    convert_address(short(\$level_data, 0x1e8) - $base, \$level{"viewteleport1x"}, \$level{"viewteleport1y"});
    convert_address(short(\$level_data, 0x1ea) - $base, \$level{"teleport2x"}, \$level{"teleport2y"});
    convert_address(short(\$level_data, 0x1f0) - $base, \$level{"viewteleport2x"}, \$level{"viewteleport2y"});

    $title = substr($level_data, 0x200, 18);
    $title = " ".$title." ";
    $title = lc($title);
    $title =~ s/ (\w)/' '.uc($1)/eg;
    $title =~ s/^\s+//;
    $title =~ s/\s+$//;
    $level{"title"} = $title;

    $cryptedmessage = substr($level_data, 0x220, 100);
    $message = "";
    if(ord($cryptedmessage) != 0)
    {
        for($i = 0; $i < length($cryptedmessage); $i++)
        {   
            $c = byte(\$cryptedmessage, $i);
            $c = $c ^ 0x7e;
            if($c > 31 && $c <127)
            {   
                $message .= chr($c);
            }
            else
            {   
                last;
            }
        }
    }
    if($message ne "")
    {
        $level{"message"} = $message;
    }

    $level{"mode"} = "8bit";

    $level{"player"} = 2;

    create_level(\%level, $file_output)
}

sub convert_amiga_levels
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    $data = read_file($file_input);

    for($number = 1; $number< 16; $number++)
    {   
        $level_data= substr($data, ($number- 1) * 0x464, 0x464);
        $file_output = sprintf("%s%02d%s", $output_prefix, $number, ".chroma");

        convert_amiga_level($level_data, $file_output);
    }
}

sub convert_amiga_level
{
    my $level_data = shift(@_);
    my $file_output = shift(@_);

    undef %level;

    $offset = 0;
    while(byte(\$level_data, $offset) > 31)
    {
	$level{"title"} .= substr($level_data, $offset, 1);
	$offset ++;
    }

    $level{"map"} = substr($level_data, 0x1f, 0x400);

    $level{"view1x"} = long(\$level_data, 0x420);
    $level{"view1y"} = long(\$level_data, 0x424);
    $level{"player1x"} = long(\$level_data, 0x428);
    $level{"player1y"} = long(\$level_data, 0x42c);
    $level{"view2x"} = long(\$level_data, 0x430);
    $level{"view2y"} = long(\$level_data, 0x434);
    $level{"player2x"} = long(\$level_data, 0x438);
    $level{"player2y"} = long(\$level_data, 0x43c);
    $level{"viewteleport1x"} = long(\$level_data, 0x444);
    $level{"viewteleport1y"} = long(\$level_data, 0x448);
    $level{"teleport1x"} = long(\$level_data, 0x44c);
    $level{"teleport1y"} = long(\$level_data, 0x450);
    $level{"viewteleport2x"} = long(\$level_data, 0x454);
    $level{"viewteleport2y"} = long(\$level_data, 0x458);
    $level{"teleport2x"} = long(\$level_data, 0x45c);
    $level{"teleport2y"} = long(\$level_data, 0x460);
    $level{"player"} = 2;

    $level{"mode"} = "amiga";

    create_level(\%level, $file_output);
}

sub convert_atari_levels
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    $data = read_file($file_input);

    $number = 1;

    foreach $title (
	    "    DOTS AND WAVES    ",
            "   SOMETHING FISHY    ",
            "   CHICKEN SUPREME    ",
            "  EXPLOSIVE MIXTURE   ",
            "    HENRYS ANGUISH    ",
            "    THE DOLLS HOUSE   ",
            "    DOLLYS REVENGE    ",
            "    ENLIGHTENMENT     ",
            "    THE CHALLENGE     ",
            "   PATIENCE PENDING   ",
            "      RAZOR EDGE      ",
            "   THE HAPPY HOUR     ",
            "       DEJA VU        ",
            "     PENULTIMATE      ",
            "     THE DECODER      "
	    )
    {

	$offset_title = locate_string(\$data, $title);
	if($offset_title > 0)
	{
	    $level_data = substr($data, $offset_title - 1074, 1096);
            $file_output = sprintf("%s%02d%s", $output_prefix, $number, ".chroma");
	    convert_atari_level($level_data, $file_output);
	}

	$number ++;
    }
}

sub convert_atari_level
{
    my $level_data = shift(@_);
    my $file_output = shift(@_);

    my $title;

    $level{"view1x"} = byte(\$level_data, 0x401);
    $level{"view1y"} = byte(\$level_data, 0x403);
    $level{"player1x"} = $level{"view1x"} + byte(\$level_data, 0x405);
    $level{"player1y"} = $level{"view1y"} + byte(\$level_data, 0x407);

    $level{"view2x"} = byte(\$level_data, 0x40b);
    $level{"view2y"} = byte(\$level_data, 0x40d);
    $level{"player2x"} = $level{"view2x"} + byte(\$level_data, 0x40f);
    $level{"player2y"} = $level{"view2y"} + byte(\$level_data, 0x411);

    $level{"viewteleport1x"} = byte(\$level_data, 0x417);
    $level{"viewteleport1y"} = byte(\$level_data, 0x419);
    $level{"viewteleport2x"} = byte(\$level_data, 0x423);
    $level{"viewteleport2y"} = byte(\$level_data, 0x425);

    $level{"teleport1x"} = $level{"viewteleport1x"} + byte(\$level_data, 0x41f);
    $level{"teleport1y"} = $level{"viewteleport1y"} + byte(\$level_data, 0x421);
    $level{"teleport2x"} = $level{"viewteleport2x"} + byte(\$level_data, 0x42b);
    $level{"teleport2y"} = $level{"viewteleport2y"} + byte(\$level_data, 0x42d);

    $level{"map"} = substr($level_data, 0, 0x400);

    $title = substr($level_data, 0x432, 20);
    $title = " ".$title." ";
    $title = lc($title);
    $title =~ s/ (\w)/' '.uc($1)/eg;
    $title =~ s/^\s+//;
    $title =~ s/\s+$//;
    $level{"title"} = $title;

    $cryptedmessage = substr($level_data, 0x220, 100);

    $level{"mode"} = "atari";

    create_level(\%level, $file_output);
}

###############################################################################

sub convert_bbc_graphics
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);
    my $mode = shift(@_);

    $data = read_file($file_input);

    if($data =~ /(\x00{72}\x1d\x1d\xdd\x00\x47\x47\x77\x00.{208})/s)
    {
	$walls = $1;
	$walls1 = substr($walls, 0, 144);
	$walls2 = substr($walls, 144, 144);
	if($walls2 =~ /^\x00\x70\x70\x70/s)
	{
	    $walls2 = "";
	}
    }
    else
    {
	die "Unable to locate graphics in input file\n";
    }

    if($data =~ /(\x00\x70\x70\x70\x70\x71\x73\x77\x00\xf0\xf0\xf4\xfe\xff\xfb\xf1.{992})/s)
    {
	$pieces = $1;
	$match = $walls1.$pieces.$walls2;
        convert_bbc_graphics_data($match, $output_prefix, "bbc");
    }
    else
    {
	die "Unable to locate graphics in input file\n";
    }

}  

sub convert_c64_graphics
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    $data = read_file($file_input);

    if($data =~ /(\x53\x53\xf3\x00.{68}\x55\x55\x15\x15.{68}\x00\x1c\x57\x55.{68})/)
    {
	$walls = substr($1, 72, 144);
    }
    else
    {
	$walls = "";
    }

    if($data =~ /(\x00{72}\x53\x53\xf3\x00\x35\x35\x3f\x00.{1072})/s)
    {   
        $pieces = $1.$walls;
        convert_bbc_graphics_data($pieces, $output_prefix, "c64");
    }
    else
    {
	die "Unable to locate graphics in input file\n";
    }

}

sub convert_bbc_graphics_file
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    $data = read_file($file_input);

    convert_bbc_graphics_data($data, $output_prefix, "bbc");
}

sub convert_bbc_graphics_data
{
    my $data = shift(@_);
    my $output_prefix = shift(@_);
    my $mode = shift(@_);
    my $offset;

    $x = 0;
    $y = 0;
    $j = 0;
    $offset = 0;
    $number = 0;

    $levels = 1 if($levels < 1);
    for($level = 0; $level < $levels; $level ++)
    { # 1 2 4
        @colours =( [0x00000000, 0x00ff0000, 0x000000ff, 0x00ffffff], # 1
                    [0x00000000, 0x00ff0000, 0x000000ff, 0x00ffffff], # 2
                    [0x00000000, 0x00ff0000, 0x000000ff, 0x00ffffff], # 3
                    [0x00000000, 0x000000ff, 0x00ff00ff, 0x00ffffff], # 4
                    [0x00000000, 0x000000ff, 0x00ff00ff, 0x00ffffff], # 5
                    [0x00000000, 0x000000ff, 0x00ff00ff, 0x00ffffff], # 6
                    [0x00000000, 0x000000ff, 0x00ff0000, 0x00ffffff], # 7
                    [0x00000000, 0x000000ff, 0x00ff0000, 0x00ffffff], # 8
                    [0x00000000, 0x000000ff, 0x00ff0000, 0x00ffffff], # 9
                    [0x00000000, 0x0000ff00, 0x00ff0000, 0x00ffffff], # 10
                    [0x00000000, 0x0000ff00, 0x00ff0000, 0x00ffffff], # 11
                    [0x00000000, 0x00ff00ff, 0x000000ff, 0x0000ff00], # 12
                    [0x00000000, 0x00ff00ff, 0x000000ff, 0x0000ff00], # 13
                    [0x000000ff, 0x00ff0000, 0x00000000, 0x00ffffff], # 14
                    [0x000000ff, 0x00ff0000, 0x00000000, 0x00ffffff]  # 15
                );

        $image{"colours"} = 4; #   RRGGBB
        $image{"palette"}[0] = $colours[$level][0];
        $image{"palette"}[1] = $colours[$level][1];
        $image{"palette"}[2] = $colours[$level][2];
        $image{"palette"}[3] = $colours[$level][3];

        $image{"x"} = 12;
        $image{"y"} = 24;

        $number = 0;
        $o = $offset;
        while($o < length($data))
        {
    	    $c = ord(substr($data, $o, 1));

	    if($mode eq "bbc")
	    {
	        $b3 = ($c & 1 ? 1 : 0) + ($c & 16 ? 2 : 0);
	        $b2 = ($c & 2 ? 1 : 0) + ($c & 32 ? 2 : 0);
	        $b1 = ($c & 4 ? 1 : 0) + ($c & 64 ? 2 : 0);
	        $b0 = ($c & 8 ? 1 : 0) + ($c & 128 ? 2 : 0);
	    }
	    if($mode eq "c64")
	    {
	        $b3 = ($c & 1 ? 1 : 0) + ($c & 2 ? 2 : 0);
	        $b2 = ($c & 4 ? 1 : 0) + ($c & 8 ? 2 : 0);
	        $b1 = ($c & 16 ? 1 : 0) + ($c & 32 ? 2 : 0);
	        $b0 = ($c & 64 ? 1 : 0) + ($c & 128 ? 2 : 0);
	    }
	
	    $image{"pixels"}[$x + 0][$y + $j] = $b0;
	    $image{"pixels"}[$x + 1][$y + $j] = $b1;
	    $image{"pixels"}[$x + 2][$y + $j] = $b2;
	    $image{"pixels"}[$x + 3][$y + $j] = $b3;

	    $o++;

	    $j++;

	    if($j == 8)
	    {
	        $j = 0;
	        $x += 4; 
	    }
	    if($x == 12)
	    {
	        $x = 0;
	        $y += 8;
	    }
	    if($y == 24)
	    {
                if($levels > 1)
                {
     	            $file_output = sprintf("%s%d_%02d%s", $output_prefix, $level + 1, $number, ".bmp");
                }
                else
                {
     	            $file_output = sprintf("%s%02d%s", $output_prefix, $number, ".bmp");
                }
	        create_bmp(\%image, $file_output, 2, 1);

	        $y = 0;
	        $x = 0;
	        $j = 0;
	        $number ++;
	    }
        }
    }
}

sub convert_cpc_graphics
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    $data = read_file($file_input);

    if($data =~ /(\xcc{144}\x0c\xc8\x0c\x0c.{2156})/s)
    {   
	convert_cpc_graphics_data($1, $output_prefix);
    }
    else
    {
	die "Unable to locate graphics in input file\n";
    }

}

sub convert_cpc_graphics_data
{
    my $data = shift(@_);
    my $output_prefix = shift(@_);

    my $offset;

    $x = 0;
    $y = 0;
    $offset = 0;
    $number = 0;

    $image{"colours"} = 16;   #  RRGGBB
    $image{"palette"}[0x0] = 0x00000081;
    $image{"palette"}[0x1] = 0x00ff8100;
    $image{"palette"}[0x2] = 0x00ff0000;
    $image{"palette"}[0x3] = 0x00ff0083;
    $image{"palette"}[0x4] = 0x008300ff;
    $image{"palette"}[0x5] = 0x00830000;
    $image{"palette"}[0x6] = 0x00ffff00;
    $image{"palette"}[0x7] = 0x00ff81ff;
    $image{"palette"}[0x8] = 0x00ffffff;
    $image{"palette"}[0x9] = 0x000000ff;
    $image{"palette"}[0xa] = 0x00000000;
    $image{"palette"}[0xb] = 0x00838183;
    $image{"palette"}[0xc] = 0x008381ff;
    $image{"palette"}[0xd] = 0x00ff8183;
    $image{"palette"}[0xe] = 0x00ffff83;
    $image{"palette"}[0xf] = 0x00ffffff;

    $image{"x"} = 12;
    $image{"y"} = 24;

    while($offset < length($data))
    {
	$c = byte(\$data, $offset);

        $b1 = ($c & 1 ? 1 : 0) + ($c & 4 ? 2 : 0) + ($c & 16 ? 4 : 0) + ($c & 64 ? 8 : 0);
        $b0 = ($c & 2 ? 1 : 0) + ($c & 8 ? 2 : 0) + ($c & 32 ? 4 : 0) + ($c & 128 ? 8 : 0);
	
	$image{"pixels"}[$x + 0][$y] = $b0;
	$image{"pixels"}[$x + 1][$y] = $b1;

	$offset ++;

	$x+=2;

	if($x == 12)
	{
	    $x = 0;
	    $y += 1;
	}
	if($y == 24)
	{
            $file_output = sprintf("%s%02d%s", $output_prefix, $number, ".bmp");
            create_bmp(\%image, $file_output, 2, 1);

	    $y = 0;
	    $x = 0;
	    $number ++;
	}
    }
}

sub convert_spectrum_graphics
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    $data = read_file($file_input);

    if($data =~ /(\x00{81}\x00\xc0\x00\x00\xc0\x00.{1209})/s)
    {   
	convert_spectrum_graphics_data($1, $output_prefix);
    }
    else
    {
	die "Unable to locate graphics in input file\n";
    }
}

sub convert_spectrum_graphics_data
{
    my $data = shift(@_);
    my $output_prefix = shift(@_);
    my $mode = shift(@_);

    my $offset;

    $x = 0;
    $y = 0;
    $offset = 0;
    $number = 0;

    $image{"x"} = 24;
    $image{"y"} = 24;

    $image{"colours"} = 16;   #  RRGGBB
    $image{"palette"}[0x0] = 0x00000000;
    $image{"palette"}[0x1] = 0x0000007f;
    $image{"palette"}[0x2] = 0x007f0000;
    $image{"palette"}[0x3] = 0x007f007f;
    $image{"palette"}[0x4] = 0x00007f00;
    $image{"palette"}[0x5] = 0x00007f7f;
    $image{"palette"}[0x6] = 0x007f007f;
    $image{"palette"}[0x7] = 0x007f7f7f;
    $image{"palette"}[0x8] = 0x00000000;
    $image{"palette"}[0x9] = 0x000000ff;
    $image{"palette"}[0xa] = 0x00ff0000;
    $image{"palette"}[0xb] = 0x00ff00ff;
    $image{"palette"}[0xc] = 0x0000ff00;
    $image{"palette"}[0xd] = 0x0000ffff;
    $image{"palette"}[0xe] = 0x00ffff00;
    $image{"palette"}[0xf] = 0x00ffffff;

    while($offset < length($data))
    {
	for($y = 0; $y < 24; $y ++)
	{
	    for($x = 0; $x < 3; $x ++)
	    {
		$c = ord(substr($data, $offset + $y * 3 + $x, 1));

		$px = $x * 8;
		if($y % 2 == 1) { $px = 16 - $px; }

		$col = ord(substr($data, $offset + 72 + (int($y / 8)*3) + $x));
		$fg = $col % 8;
		$bg = int($col / 8) % 8;

		if($col & 64)
		{
		    $fg +=8; $bg+=8;
		}
		
		$image{"pixels"}[$px + 0][$y] = ($c & 128 ? $fg : $bg);
		$image{"pixels"}[$px + 1][$y] = ($c & 64 ? $fg : $bg);
		$image{"pixels"}[$px + 2][$y] = ($c & 32 ? $fg : $bg);
		$image{"pixels"}[$px + 3][$y] = ($c & 16 ? $fg : $bg);
		$image{"pixels"}[$px + 4][$y] = ($c & 8 ? $fg : $bg);
		$image{"pixels"}[$px + 5][$y] = ($c & 4 ? $fg : $bg);
		$image{"pixels"}[$px + 6][$y] = ($c & 2 ? $fg : $bg);
		$image{"pixels"}[$px + 7][$y] = ($c & 1 ? $fg : $bg);
	    }
	}

        $file_output = sprintf("%s%02d%s", $output_prefix, $number, ".bmp");
        create_bmp(\%image, $file_output, 1, 1);

        $offset += 81;
        $number++;
    }
}

sub convert_amiga_graphics
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    for($level = 0; $level < 15; $level ++)
    {
	$file_in = $file_input . "/vimages".$level;
	$file_out = sprintf("%s/back_%d_", $output_prefix, $level + 1);
	convert_amiga_graphics_file($file_in, $file_out, 0);
    }

    $file_in = $file_input . "/cimages";
    $file_out = sprintf("%s/char_", $output_prefix);
    convert_amiga_graphics_file($file_in, $file_out, 1);
}

sub convert_amiga_graphics_file
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);
    my $mode = shift(@_);
    
    open(INPUT, $file_input) || die "Couldn't open input file ($file_input): $!\n";
    binmode INPUT;
    undef $/;
    $data = <INPUT>;
    close INPUT;

    $image{"x"} = 24;
    $image{"y"} = 24;
    $image{"colours"} = 32;

    # Read palette
    if($mode == 0)
    {
        for($i = 0; $i < 32; $i ++)
        {
	    $p = 256 * ord(substr($data, $i * 2, 1)) + ord(substr($data, $i * 2 + 1, 1));

	    $blue = $p % 16;
	    $green = (($p - ($p % 16)) / 16) % 16;
	    $red= (($p - ($p % 256)) / 256) % 16;

	    $blue *= 0x11;
	    $green *= 0x11;
	    $red *= 0x11;

	    $image{"palette"}[$i] = $blue + ($green * 0x100) + ($red * 0x10000);
        }
    }

    if($mode == 0)
    {
	$sprites = 20;
    }
    else
    {
	$sprites = 60;
    }

    for($sprite = 0; $sprite < $sprites; $sprite ++)
    {
	undef $image{"pixels"};
	
        for($colour = 0; $colour < 5; $colour ++)
        {
            for($row = 0; $row < 24; $row ++)
            {
    	        if($mode == 0)
	        {
	            $block = ($sprite - ($sprite % 10)) / 10;
	            $block += 2 * $colour;
                    $offset = 64 + ($sprite % 10) * 4 + $row * 40 + $block * 960;
	        }
	        else
	        {
                    $block = ($sprite - ($sprite % 10)) / 10;
                    $block += 6 * $colour;
                    $offset = 0 + ($sprite % 10) * 4 + $row * 40 + $block * 960;
	        }

	        for($byte = 0; $byte < 3; $byte ++)
	        {
	            $b = ord(substr($data, $offset + $byte, 1));
	            for($bit = 0; $bit < 8; $bit ++)
	            {
                        if($b & (2 ** (7 - $bit)))
		        {
		            $image{"pixels"}[$byte * 8 + $bit][$row] |= 2 ** $colour;
		        }
	            }
	        }
            }
        }

	$file_output = sprintf("%s%02d%s", $output_prefix, $sprite, ".bmp");
	create_bmp(\%image, $file_output, 1, 1);
    }
    
}

sub convert_atari_graphics
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    $data = read_file($file_input);

    $palette = "\x02\x27\x00\x40\x00\x00\x03\x33\x07\x44\x07\x11\x03\x37\x07\x70\x02\x25\x05\x57\x07\x50\x03\x53\x04\x44\x05\x55\x06\x66\x07\x77";

    $ok = 0;
    $offset_palette = 0;
    while(!$ok && $offset_palette != -1)
    {
        $offset_palette = index($data, $palette, $offset_palette);
        $offset_palette2 = index($data, $palette, $offset_palette);

	if($offset_palette2 = $offset_palette + 96)
	{
	    $ok = 1;
	}
    }
    if($ok == 0)
    {
	die "Unable to locate palette data in file";
    }

    $offset_base = $offset_palette - 0x1b6e4;
    $offset_pieces = $offset_base + 0x68000;
    $offset_backgrounds = $offset_base + 0x60000;

    @graphics_pieces = (
	  # ["space", 0, 0, 0],
	  # ["wall", 2, 0, 0],
          # ["player_one", 4, 0, 1]
          # ["player_two", 6, 0, 1]
	    ["map_top_left", 8, 0, 1],
	    ["dots", 10, 0, 1],

	    ["waves", 0, 1, 1],
	    ["fish", 2, 1, 1],
	    ["chicken", 4, 1, 1],
	    ["h_bomb", 6, 1, 1],
	    ["v_bomb", 8, 1, 1],
	    ["mask", 10, 1, 1],

	    ["door", 0, 2, 1],
	    ["doll", 2, 2, 1],
	    ["map_top_right", 4, 2, 1],
	    ["map_bottom_left", 6, 2, 1],
	    ["map_bottom_right", 8, 2, 1],
	    ["teleport", 10, 2, 1],

	    ["switch", 0, 3, 1],
	    ["fish_2", 8, 3, 1],
	    ["chicken_2", 10, 3, 1],

	    ["chicken_3", 0, 4, 1],
	    ["fish_3", 2, 4, 1],
	    ["fish_4", 4, 4, 1],
	    ["chicken_4", 6, 4, 1],
	    ["chicken_5", 8, 4, 1],
	    ["fish_5", 10, 4, 1],

	    ["explosion_0", 0, 5, 1],
	    ["explosion_1", 2, 5, 1],
	    ["explosion_2", 4, 5, 1],
	    ["explosion_3", 8, 5, 1],
	    ["explosion_4", 10, 5, 1],

	    ["player_one_1", 0, 6, 2],
	    ["player_two_1", 1, 6, 2],
	    ["player_one_2", 2, 6, 2],
	    ["player_two_2", 3, 6, 2],
	    ["player_one_3", 4, 6, 2],
	    ["player_two_3", 5, 6, 2],
	    ["player_one_4", 6, 6, 2],
	    ["player_two_4", 7, 6, 2],
	    ["player_one_5", 8, 6, 2],
	    ["player_two_5", 9, 6, 2],
	    ["player_one_6", 10, 6, 2],
	    ["player_two_6", 11, 6, 2],

	    ["player_one_7", 0, 7, 2],
	    ["player_two_7", 1, 7, 2],
	    ["player_one_8", 2, 7, 2],
	    ["player_two_8", 3, 7, 2],
	    ["player_one_9", 4, 7, 2],
	    ["player_two_9", 5, 7, 2],
	    ["player_one_10", 6, 7, 2],
	    ["player_two_10", 7, 7, 2],
	    ["player_one_11", 8, 7, 2],
	    ["player_two_11", 9, 7, 2],
	    ["player_one_12", 10, 7, 2],
	    ["player_two_12", 11, 7, 2],

	    ["player_one_13", 0, 8, 2],
	    ["player_two_13", 1, 8, 2],
	    ["player_one_14", 2, 8, 2],
	    ["player_two_14", 3, 8, 2],
	    ["player_one_15", 4, 8, 2],
	    ["player_two_15", 5, 8, 2],
	    );

    for($graphic = 0; $graphic <= $#graphics_pieces; $graphic ++)
    {
	convert_atari_graphic($output_prefix."/".$graphics_pieces[$graphic][0], \$data, $offset_pieces, $graphics_pieces[$graphic][1] * 24, $graphics_pieces[$graphic][2] * 22, 24, 22, $graphics_pieces[$graphic][3]);
    }

    @graphics_backgrounds = (
	    ["background_1", 0, 0, 0],
	    ["background_2", 2, 0, 0],
	    ["background_3", 4, 0, 0],
	    ["background_4", 6, 0, 0],
	    ["background_5", 8, 0, 0],
	    ["background_6", 10, 0, 0],
	    ["background_7", 0, 2, 0],
	    ["background_8", 2, 2, 0],
	    ["background_9", 4, 2, 0],
	    ["background_10", 6, 2, 0],
	    ["background_11", 8, 2, 0],
	    ["background_12", 10, 2, 0],
	    ["background_13", 6, 4, 0],
	    ["background_14", 8, 4, 0],
	    ["background_15", 10, 4, 0],
	    );

    for($graphic = 0; $graphic <= $#graphics_backgrounds; $graphic ++)
    {
	convert_atari_graphic($output_prefix."/".$graphics_backgrounds[$graphic][0], \$data, $offset_backgrounds, $graphics_backgrounds[$graphic][1] * 24, $graphics_backgrounds[$graphic][2] * 22, 48, 44, $graphics_backgrounds[$graphic][3]);
    }

    @graphics_walls = (
	    ["wall_1_1", 5, 8, 3],
	    ["wall_1_2", 6, 8, 3],
	    ["wall_1_3", 7, 8, 3],
	    ["wall_1_4", 8, 8, 3],

	    ["wall_2_1", 7, 6, 3],
	    ["wall_2_2", 8, 6, 3],
	    ["wall_2_3", 7, 7, 3],
	    ["wall_2_4", 8, 7, 3],

	    ["wall_3_1", 9, 6, 3],
	    ["wall_3_2", 10, 6, 3],
	    ["wall_3_3", 9, 7, 3],
	    ["wall_3_4", 10, 7, 3],

	    ["wall_4_1", 11, 6, 3],
	    ["wall_4_2", 12, 6, 3],
	    ["wall_4_3", 11, 7, 3],
	    ["wall_4_4", 12, 7, 3],

	    ["wall_5_1", 12, 0, 4],
	    ["wall_5_2", 12, 1, 4],
	    ["wall_5_3", 12, 2, 4],
	    ["wall_5_4", 12, 3, 4],
	    ["wall_5_5", 12, 5, 3],

	    ["wall_6_1", 12, 4, 4],
	    ["wall_6_2", 12, 5, 4],
	    ["wall_6_3", 12, 6, 4],
	    ["wall_6_4", 12, 7, 4],

	    ["wall_7_1", 6, 8, 4],
	    ["wall_7_2", 7, 8, 4],
	    ["wall_7_3", 8, 8, 4],
	    ["wall_7_4", 9, 8, 4],

	    ["wall_8_1", 6, 3, 4],
	    ["wall_8_2", 7, 3, 4],
	    ["wall_8_3", 10, 8, 4],
	    ["wall_8_4", 11, 8, 4],

	    ["wall_9_1", 2, 3, 4],
	    ["wall_9_2", 3, 3, 4],
	    ["wall_9_3", 4, 3, 4],
	    ["wall_9_4", 5, 3, 4],

	    ["wall_10_1", 12, 8, 4],
	    ["wall_10_2", 2, 8, 3],
	    ["wall_10_3", 3, 8, 3],
	    ["wall_10_4", 4, 8, 3],

	    ["wall_11_1", 0, 4, 3],
	    ["wall_11_2", 1, 4, 3],
	    ["wall_11_3", 0, 5, 3],
	    ["wall_11_4", 1, 5, 3],

	    ["wall_12_1", 2, 4, 3],
	    ["wall_12_2", 3, 4, 3],
	    ["wall_12_3", 2, 5, 3],
	    ["wall_12_4", 3, 5, 3],

	    ["wall_13_1", 4, 4, 3],
	    ["wall_13_2", 5, 4, 3],
	    ["wall_13_3", 4, 5, 3],
	    ["wall_13_4", 5, 5, 3],

	    ["wall_14_1", 5, 6, 3],
	    ["wall_14_2", 6, 6, 3],
	    ["wall_14_3", 5, 7, 3],
	    ["wall_14_4", 6, 7, 3],

	    ["wall_15_1", 9, 8, 3],
	    ["wall_15_2", 10, 8, 3],
	    ["wall_15_3", 11, 8, 3],
	    ["wall_15_4", 12, 8, 3],

	    );

    for($graphic = 0; $graphic <= $#graphics_walls; $graphic ++)
    {
	$size_x = defined($graphics_walls[$graphic][4]) ? $graphics_walls[$graphic][4] : 1;
	$size_y = defined($graphics_walls[$graphic][5]) ? $graphics_walls[$graphic][5] : 1;

	if($graphics_walls[$graphic][3] == 3)
	{
	    convert_atari_graphic($output_prefix."/".$graphics_walls[$graphic][0], \$data, $offset_backgrounds, $graphics_walls[$graphic][1] * 24, $graphics_walls[$graphic][2] * 22, $size_x * 24, $size_y * 22, 0);
	}
	elsif($graphics_walls[$graphic][3] == 4)
	{
	    convert_atari_graphic($output_prefix."/".$graphics_walls[$graphic][0], \$data, $offset_pieces, $graphics_walls[$graphic][1] * 24, $graphics_walls[$graphic][2] * 22, $size_x * 24, $size_y * 22, 0);
	}
    }

}

sub convert_atari_graphic
{
    my $file_output = shift(@_);
    my $data = shift(@_);
    my $offset = shift(@_);
    my $offset_x = shift(@_);
    my $offset_y = shift(@_);
    my $size_x = shift(@_);
    my $size_y = shift(@_);
    my $mask = shift(@_);

    @colour_map = (0, 36, 73, 109, 146, 182, 219, 255);

    $image{"colours"} = 16;
    $image{"x"} = $size_x;
    $image{"y"} = $size_y;

    # Setup palette
    for($i = 0; $i < 16; $i ++)
    {
	$c = byte(\$palette, $i * 2) * 256 + byte(\$palette, $i * 2 + 1);
	$r = $colour_map[int($c / 256) % 8];
	$g = $colour_map[int($c / 16) % 8];
	$b = $colour_map[$c % 8];
	$image{"palette"}[$i] = $r * 0x10000 + $g * 0x100 + $b;
    }

    # Transparent colour
    if($mask != 0)
    {
	$image{"colours"} = 17;
	$image{"palette"}[16] = 0x00deface; 
    }

    undef $image{"pixels"};

    for($y = 0; $y < $size_y; $y ++)
    {
	for($x = 0; $x < $size_x; $x += 8)
	{
	    $o = $offset + ($y + $offset_y) * 160 + 8 * int(($x + $offset_x) / 16);
	    $o++ if(($x + $offset_x) % 16 == 8);

	    for($colour = 0; $colour < 4; $colour ++)
	    {
	        $byte = byte(\$$data, $o + $colour * 2);
	        for($b = 0; $b < 8; $b ++)
	        {
                    if($byte & (2 ** (7 - $b)))
                    {   
                        $image{"pixels"}[$x + $b][$y] |= 2 ** $colour;
                    }
	        }
	    }
	}
    }

    if($mask)
    {
	if($mask == 2)
	{
	    $mask_x = 5 * 24;
	    $mask_y = 0;
	}
	else
	{
	    $mask_x = $offset_x + 24;
	    $mask_y = $offset_y;
	}

        for($y = 0; $y < $size_y; $y ++)
        {
    	    for($x = 0; $x < $size_x; $x += 8)
	    {
	        $o = $offset + ($y + $mask_y) * 160 + 8 * int(($x + $mask_x) / 16);
	        $o++ if(($x + $mask_x) % 16 == 8);

	        $byte = byte(\$$data, $o);

	        for($b = 0; $b < 8; $b ++)
	        {
                    if($byte & (2 ** (7 - $b)))
                    {   
                        $image{"pixels"}[$x + $b][$y] = 16;
                    }
	        }
	    }
        }
    }

    create_bmp(\%image, $file_output.".bmp", 1, 1);
}
    
###############################################################################

sub convert_bbc_music
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    # If debug is set, the notes are displayed on screen
    $debug = 0;

    # Translation table for displaying notes on screen
    $debug_table[0x00] = "---";
    $debug_table[0x11] = "D#2";
    $debug_table[0x1d] = "F#2";
    $debug_table[0x25] = "G#2";
    $debug_table[0x2d] = "A#2";
    $debug_table[0x35] = "C-3";
    $debug_table[0x39] = "C#3";
    $debug_table[0x41] = "D#3";
    $debug_table[0x49] = "F-3";
    $debug_table[0x4d] = "F#3";
    $debug_table[0x55] = "G#3";
    $debug_table[0x5d] = "A#3";
    $debug_table[0x65] = "C-4";
    $debug_table[0x69] = "C#4";
    $debug_table[0x71] = "D#4";
    $debug_table[0x79] = "F-4";
    $debug_table[0x7d] = "F#4";
    $debug_table[0x85] = "G#4";
    $debug_table[0x8d] = "A#4";

    # Translation table between BBC SOUND values and note values
    $note_table[0x00] = 0;
    $note_table[0x11] = 0x1c;
    $note_table[0x1d] = 0x1f;
    $note_table[0x25] = 0x21;
    $note_table[0x2d] = 0x23;
    $note_table[0x35] = 0x25;
    $note_table[0x39] = 0x26;
    $note_table[0x41] = 0x28;
    $note_table[0x49] = 0x2a;
    $note_table[0x4d] = 0x2b;
    $note_table[0x55] = 0x2d;
    $note_table[0x5d] = 0x2f;
    $note_table[0x65] = 0x31;
    $note_table[0x69] = 0x32;
    $note_table[0x71] = 0x34;
    $note_table[0x79] = 0x36;
    $note_table[0x7d] = 0x37;
    $note_table[0x85] = 0x39;
    $note_table[0x8d] = 0x3b;

    $data = read_file($file_input);

    # Search for various data structures in file
    if($data =~ /(\x11\x35\x1d\x35.{75}\x39)/s)
    {
        $data_channel12barnotes = $1;
    }
    else
    {
        print "ERROR: Couldn't locate data in input file (channel12barnotes)\n";
        return;
    }

    if($data =~ /(\x00\x00\x46\x0a.{50}\x50)/s)
    {
        $data_channel12barorder = $1;
    }
    else
    {
        print "ERROR: Couldn't locate data in input file (channel12barorder)\n";
        return;
    }

    if($data =~ /(\x69\x0c\x65\x0c.{204}\xff)/s)
    {
        $data_channel3barnotes = $1;
    }
    else
    {
        print "ERROR: Couldn't locate data in input file (channel3barnotes)\n";
        return;
    }

    if($data =~ /(\x21\x4f\x21\x4f.{47}\x4f)/s)
    {
        $data_channel3barorder = $1;
    }
    else
    {
        print "ERROR: Couldn't locate data in input file (channel3barorder)\n";
        return;
    }

    $duration = 648;
    $patterns = ($duration / 64) + 1;

    open(OUTPUT, ">".$file_output) || die "Couldn't open file for output: $!\n";
    binmode OUTPUT;

    # Module header
    print OUTPUT "Extended Module: ";
    print OUTPUT pack("Z20", "XOR Theme");		# Module name
    print OUTPUT pack("c", 0x1a);
    print OUTPUT pack("Z20", "convert2chroma.pl"); 	# Tracker name
    print OUTPUT pack("v", 0x0104);			# Version
    print OUTPUT pack("V", 0x114);			# Header size
    print OUTPUT pack("v", $patterns);	        	# Song length
    print OUTPUT pack("v", 0);				# Restart position
    print OUTPUT pack("v", 4);				# Channels
    print OUTPUT pack("v", $patterns); 			# Patterns
    print OUTPUT pack("v", 2);				# Instruments
    print OUTPUT pack("v", 1);				# Flags (linear frequency table)
    print OUTPUT pack("v", 6);				# Default tempo
    print OUTPUT pack("v", 125);			# Default BPM

    # Pattern table
    for($i = 0; $i < 256; $i ++)
    {
	print OUTPUT pack("c", $i < ($patterns - 1) ? $i : 0); 
    }

    # Generate patterns from tune
    $time = 0;
    $time_channel3 = 24;

    $offset_channel12 = 0;
    $offset_channel12bar = 0;
    $offset_channel3 = 0;
    $offset_channel3bar = 0;

    for($time = 0; $time < $duration; $time ++)
    {
        $play_1 = 0;
        $play_2 = 0;
        $play_3 = 0;

        if($time % 3 == 0)
        {
    	    $note = $offset_channel12 + ($time % 12) / 3;
	    $play_1 = ord(substr($data_channel12barnotes, $note, 1));
        }

        if($time % 2 == 0)
        {
	    $note = $offset_channel12 + 4 + ($time % 12) / 2;
	    $play_2 = ord(substr($data_channel12barnotes, $note, 1));
        }

        if($time_channel3 == 0)
        {
    	    $play_3 = ord(substr($data_channel3barnotes, $offset_channel3, 1));
	    if($play_3 == 255)
	    {
	        $offset_channel3 = ord(substr($data_channel3barorder, $offset_channel3bar, 1));

	        $offset_channel3bar += 2;
		if($offset_channel3bar > length($data_channel3barorder))
		{
                    # End of channel 3 bar data
		    # This should happen just after the end of the tune, so never gets called.
		    print "ERROR: Premature end of channel 3 bar data\n";
		}

	        $play_3 = ord(substr($data_channel3barnotes, $offset_channel3, 1));

	    }
	    $offset_channel3 ++;

	    $time_channel3 = ord(substr($data_channel3barnotes, $offset_channel3, 1));
	    $offset_channel3 ++;

        }

        $time_channel3 --;

	if($play_1 != 0) { $pattern[1][$time % 64] = $note_table[$play_1]; }
	if($play_2 != 0) { $pattern[2][$time % 64] = $note_table[$play_2]; }
	if($play_3 != 0) { $pattern[3][$time % 64] = $note_table[$play_3]; }

	if($debug)
	{
            printf("%8d%8s%8s%8s\n", $time, $debug_table[$play_1], $debug_table[$play_2], $debug_table[$play_3]);
	}

        if($time % 12 == 11)
        {
	    if($debug)
	    {
		# Bar separator
		print "\n";
	    }

	    $offset_channel12bar ++;
	    $offset_channel12 = ord(substr($data_channel12barorder, $offset_channel12bar, 1));
	    if($offset_channel12 == 0x50)
	    {
	        # End of channel 1/2 bar data
		# This should happen at the very end of the tune. 
	    }
        }

	if($time % 64 == 63)
	{
	    dump_pattern(0);
	}
    }

    # Dump any partial final pattern
    if($duration % 64 != 0)
    {
        dump_pattern(1);
    }

    for($instrument =0; $instrument < 2; $instrument ++)
    {
        # Instrument
        print OUTPUT pack("V", 0x107); 			# Instrument size
        print OUTPUT pack("Z22", "Instrument $instrument");	# Instrument name
        print OUTPUT pack("c", 0); 			# Instrument type
        print OUTPUT pack("v", 1); 			# Number of samples

        print OUTPUT pack("V", 40); 			# Sample header size
        print OUTPUT pack("Z96", ""); 			# Sample number for each note
        print OUTPUT pack("Z48", "\x00\x00\x40");	# Points for volume envelope
        print OUTPUT pack("Z48", "\x00\x00\x20");	# Points for panning envelope
        print OUTPUT pack("c", 1); 			# Number of volume points
        print OUTPUT pack("c", 1);  			# Number of panning points
        print OUTPUT pack("c", 0); 			# Volume sustain point
        print OUTPUT pack("c", 0);  			# Volume loop start point
        print OUTPUT pack("c", 0);  			# Volume loop end point
        print OUTPUT pack("c", 0);  			# Panning sustain point
        print OUTPUT pack("c", 0);  			# Panning loop start point
        print OUTPUT pack("c", 0);  			# Panning loop end point
        print OUTPUT pack("c", 0);  			# Volume type
        print OUTPUT pack("c", 0);  			# Panning type
        print OUTPUT pack("c", 0);  			# Vibrato type
        print OUTPUT pack("c", 0);  			# Vibrato sweep
        print OUTPUT pack("c", 0);  			# Vibrato depth
        print OUTPUT pack("c", 0);  			# Vibrato rate
        print OUTPUT pack("v", 0);  			# Volume fadeout
        print OUTPUT pack("v", 0);  			# Reserved
        print OUTPUT pack("Z20", "");			# ?

        # Sample
        if($instrument == 0)
        {
            # Used for channels 1 and 2 for a backing beat
            $sample_length = 2048;
	    $phase_attack = 0.0;
	    $phase_decay = 0.0;
	    $relative_note = 12;
        }
        else
        {
	    # Used for channel 3 for the melody
            $sample_length = 4096;
	    $phase_attack = 0.05;
	    $phase_decay = 0.1;
	    $relative_note = 0;
        }

        print OUTPUT pack("V", $sample_length);		# Sample length
        print OUTPUT pack("V", 0);			# Sample loop start
        print OUTPUT pack("V", 1);			# Sample loop end
        print OUTPUT pack("c", 64);			# Volume
        print OUTPUT pack("c", 0 );			# Finetune
        print OUTPUT pack("c", 0);			# Type
        print OUTPUT pack("c", 128);			# Panning
        print OUTPUT pack("c", $relative_note);		# Relative note number
        print OUTPUT pack("c", 0);			# Reserved
        print OUTPUT pack("Z22", "Sample $instrument");	# Sample name

        # Sample data
        $sample = "";
        $previous = 0;
        for($i = 0; $i < $sample_length; $i ++)
        {
            # The XM format documentation gives 8363Hz for C-4. Conversely, xmp's
	    # output suggests 8287Hz for C-4. C-0 should be 262Hz. Either way, that
	    # doesn't affect sample generation, as it is independent of C-4s frequency:
	    # 
	    # $value = sin($PI * ($C4 / 8 ) * ( 2 ** - ($relative_note / 12) * $i / $C4));
	    
	    $value = sin(3.14159265 * (2 ** - ($relative_note / 12)) * $i / 8);

	    # Convert to a square wave 
	    $value = $value > 0 ? 1 : -1;

            # Volume envelope
	    $volume = 124;

	    $phase = ($i / $sample_length);

	    if($phase < $phase_attack)
	    {
	        $value = $volume * $value * $phase / $phase_attack;
	    }
	    elsif($phase < $phase_decay)
	    {
	        $value = $volume * $value;
	    }
	    else
	    {
	        $value = $volume * $value * (1 - (($phase - $phase_decay) / (1 - $phase_decay)));
	    }

            # Convert to delta sample data
	    $value = int($value);
	    $delta = $value - $previous;
	    $previous = $value;

	    $sample .= pack("c", $delta);
        }

        print OUTPUT $sample;
    }

    close(OUTPUT);
}

sub dump_pattern
{
    my $final = shift(@_);

    $pattern_data = "";

    for($row = 0; $row < 64; $row ++)
    {
	for($channel = 1; $channel <=4; $channel ++)
	{
	    if($pattern[$channel][$row] != 0)
	    {
		$pattern_data .= pack("c3", 0x83, $pattern[$channel][$row], $channel == 3 ? 0x2 : 0x1);
	    }
	    else
	    {
		if($final && $row == (($duration - 1) % 64) && $channel == 4)
		{
                    # Loop back to pattern 0 at end of tune
		    $pattern_data .= pack("cc", 0x88, 0xb);
		}
		else
		{
		    $pattern_data .= pack("c", 0x80);
		}
	    }
	}
    }

    print OUTPUT pack("V", 9);     			# Pattern header size
    print OUTPUT pack("c", 0);				# Packing type
    print OUTPUT pack("v", 64); 			# Number of rows
    print OUTPUT pack("v", length($pattern_data)); 	# Packed pattern size
    print OUTPUT $pattern_data;

    undef @pattern;
}

###############################################################################

sub convert_bbc_all
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    make_directory($output_prefix);
    make_directory($output_prefix."/levels/");
    make_directory($output_prefix."/levels/xor_bbc/");
    make_directory($output_prefix."/graphics/");
    make_directory($output_prefix."/graphics/xor_bbc/");

    convert_bbc_levels($file_input, $output_prefix."/levels/xor_bbc/xor_bbc_");
    create_set($output_prefix."/levels/xor_bbc/set.chroma", "XOR (BBC)");

    $levels = 15;
    convert_bbc_graphics($file_input, $output_prefix."/graphics/xor_bbc/xor_bbc_");

    open(FILE, ">".$output_prefix."/graphics/xor_bbc.chroma");

print FILE << "EOF";
<chroma type="graphics">

<head>

<title>XOR (BBC)</title>

<sizes>
<!-- Everything is scaled from 24x24 -->
<size x="12" y="12" small="yes" pieces="yes" />
<size x="24" y="24" small="yes" pieces="yes" />
<size x="48" y="48" small="yes" pieces="yes" />
<size x="72" y="72" small="yes" pieces="yes" />
<size x="96" y="96" small="yes" pieces="yes" />
<size x="144" y="144" small="yes" pieces="yes" />
</sizes>

</head>

<background colour="#808080" />

<pieces path="xor_bbc" levels="15">

<piece name="space" scale="yes">
<image file="xor_bbc_%l_00.bmp" />
</piece>

<piece name="wall" scale="yes">
<if condition="l=1">
<image file="xor_bbc_%l_01.bmp" />
</if>
<if condition="l=2">
<image file="xor_bbc_%l_16.bmp" />
</if>
<if condition="l=3">
<image file="xor_bbc_%l_17.bmp" />
</if>
<if condition="l=4">
<image file="xor_bbc_%l_01.bmp" />
</if>
<if condition="l=5">
<image file="xor_bbc_%l_16.bmp" />
</if>
<if condition="l=6">
<image file="xor_bbc_%l_17.bmp" />
</if>
<if condition="l=7">
<image file="xor_bbc_%l_01.bmp" />
</if>
<if condition="l=8">
<image file="xor_bbc_%l_16.bmp" />
</if>
<if condition="l=9">
<image file="xor_bbc_%l_17.bmp" />
</if>
<if condition="l=10">
<image file="xor_bbc_%l_17.bmp" />
</if>
<if condition="l=11">
<image file="xor_bbc_%l_17.bmp" />
</if>
<if condition="l=12">
<image file="xor_bbc_%l_16.bmp" />
</if>
<if condition="l=13">
<image file="xor_bbc_%l_16.bmp" />
</if>
<if condition="l=14">
<image file="xor_bbc_%l_01.bmp" />
</if>
<if condition="l=15">
<image file="xor_bbc_%l_01.bmp" />
</if>
</piece>

<piece name="player_one" scale="yes">
<image file="xor_bbc_%l_02.bmp" />
<image type="small" file="xor_bbc_%l_02.bmp" />
</piece>

<piece name="player_two" scale="yes">
<image file="xor_bbc_%l_03.bmp" />
<image type="small" file="xor_bbc_%l_03.bmp" />
</piece>

<piece name="map_top_left" scale="yes">
<image file="xor_bbc_%l_04.bmp" />
</piece>

<piece name="map_top_right" scale="yes">
<image file="xor_bbc_%l_04.bmp" />
</piece>

<piece name="map_bottom_left" scale="yes">
<image file="xor_bbc_%l_04.bmp" />
</piece>

<piece name="map_bottom_right" scale="yes">
<image file="xor_bbc_%l_04.bmp" />
</piece>

<piece name="dots_x" scale="yes">
<image file="xor_bbc_%l_05.bmp" />
</piece>

<piece name="dots_y" scale="yes">
<image file="xor_bbc_%l_06.bmp" />
</piece>

<piece name="arrow_red_down" scale="yes">
<image file="xor_bbc_%l_07.bmp" />
</piece>

<piece name="arrow_red_left" scale="yes">
<image file="xor_bbc_%l_08.bmp" />
</piece>

<piece name="bomb_red_down" scale="yes">
<image file="xor_bbc_%l_09.bmp" />
</piece>

<piece name="bomb_red_left" scale="yes">
<image file="xor_bbc_%l_10.bmp" />
</piece>

<piece name="star" scale="yes">
<image file="xor_bbc_%l_11.bmp" />
<image type="small" file="xor_bbc_%l_11.bmp" />
</piece>

<piece name="door" scale="yes">
<image file="xor_bbc_%l_12.bmp" />
<image type="small" file="xor_bbc_%l_12.bmp" />
</piece>

<piece name="circle" scale="yes">
<image file="xor_bbc_%l_13.bmp" />
</piece>

<piece name="teleport" scale="yes">
<image file="xor_bbc_%l_14.bmp" />
</piece>

<piece name="switch" scale="yes">
<image file="xor_bbc_%l_15.bmp" />
</piece>

<piece name="explosion_red_left" scale="yes">
<image colour="#ffffff" />
</piece>
<piece name="explosion_red_horizontal">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_right">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_top">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_vertical">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_bottom">
<clone piece="explosion_red_left" />
</piece>

<piece name="darkness" scale="yes">
<image colour="#000000" />
</piece>

</pieces>
</chroma>
EOF

    convert_bbc_music($file_input, $output_prefix."/xor-theme.xm");
}

sub convert_designer_all
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    make_directory($output_prefix);
    make_directory($output_prefix."/levels/");
    make_directory($output_prefix."/levels/xor_bbc_designer/");
    make_directory($output_prefix."/graphics/");
    make_directory($output_prefix."/graphics/xor_bbc_designer/");

    convert_designer_levels($file_input, $output_prefix."/levels/xor_bbc_designer/xor_bbc_designer_");
    create_set($output_prefix."/levels/xor_bbc_designer/set.chroma", "XOR (BBC Designer)");

    convert_bbc_graphics($file_input, $output_prefix."/graphics/xor_bbc_designer/xor_bbc_designer_");
    create_graphics_set($output_prefix."/graphics/xor_bbc_designer.chroma", "xor_bbc_designer", "XOR (BBC Designer)", 24);

    convert_bbc_music($file_input, $output_prefix."/xor-theme.xm");
}


sub convert_c64_all
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    make_directory($output_prefix);
    make_directory($output_prefix."/levels/");
    make_directory($output_prefix."/levels/xor_c64/");
    make_directory($output_prefix."/graphics/");
    make_directory($output_prefix."/graphics/xor_c64/");

    convert_bbc_levels($file_input, $output_prefix."/levels/xor_c64/xor_c64_");
    create_set($output_prefix."/levels/xor_c64/set.chroma", "XOR (C64)");

    convert_c64_graphics($file_input, $output_prefix."/graphics/xor_c64/xor_c64_");
    create_graphics_set($output_prefix."/graphics/xor_c64.chroma", "xor_c64", "XOR (C64)", 24);
}

sub convert_spectrum_all
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    make_directory($output_prefix);
    make_directory($output_prefix."/levels/");
    make_directory($output_prefix."/levels/xor_spectrum/");
    make_directory($output_prefix."/graphics/");
    make_directory($output_prefix."/graphics/xor_spectrum/");

    convert_spectrum_levels($file_input, $output_prefix."/levels/xor_spectrum/xor_spectrum_");
    create_set($output_prefix."/levels/xor_spectrum/set.chroma", "XOR (Spectrum)");

    convert_spectrum_graphics($file_input, $output_prefix."/graphics/xor_spectrum/xor_spectrum_");
    create_graphics_set($output_prefix."/graphics/xor_spectrum.chroma", "xor_spectrum", "XOR (Spectrum)", 24);
}

sub convert_cpc_all
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    make_directory($output_prefix);
    make_directory($output_prefix."/levels/");
    make_directory($output_prefix."/levels/xor_cpc/");
    make_directory($output_prefix."/graphics/");
    make_directory($output_prefix."/graphics/xor_cpc/");

    convert_spectrum_levels($file_input, $output_prefix."/levels/xor_cpc/xor_cpc_");
    create_set($output_prefix."/levels/xor_cpc/set.chroma", "XOR (CPC)");

    convert_cpc_graphics($file_input, $output_prefix."/graphics/xor_cpc/xor_cpc_");
    create_graphics_set($output_prefix."/graphics/xor_cpc.chroma", "xor_cpc", "XOR (CPC)", 24);
}

sub convert_amiga_all
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    make_directory($output_prefix);

    make_directory($output_prefix."/levels/");
    make_directory($output_prefix."/levels/xor_amiga_1/");
    make_directory($output_prefix."/levels/xor_amiga_2/");

    convert_amiga_levels($file_input."/standard.mazes", $output_prefix."/levels/xor_amiga_1/xor_amiga_1_");

    convert_amiga_levels($file_input."/standard2.mazes", $output_prefix."/levels/xor_amiga_2/xor_amiga_2_");

    create_set($output_prefix."/levels/xor_amiga_1/set.chroma", "XOR (Amiga Set 1)");
    create_set($output_prefix."/levels/xor_amiga_2/set.chroma", "XOR (Amiga Set 2)");

    make_directory($output_prefix."/graphics/");
    make_directory($output_prefix."/graphics/xor_amiga/");

    convert_amiga_graphics($file_input, $output_prefix."/graphics/xor_amiga");

    open(FILE, ">".$output_prefix."/graphics/xor_amiga.chroma");
    print FILE << "EOF";
<chroma type="graphics">

<head>

<title>XOR (Amiga)</title>

<sizes>
<!-- Everything is scaled from 24x24 -->
<size x="12" y="12" small="yes" pieces="yes" />
<size x="24" y="24" small="yes" pieces="yes" />
<size x="48" y="48" small="yes" pieces="yes" />
<size x="72" y="72" small="yes" pieces="yes" />
<size x="96" y="96" small="yes" pieces="yes" />
<size x="144" y="144" small="yes" pieces="yes" />
</sizes>

</head>

<background colour="#808080" />

<pieces path="xor_amiga" levels="15">

<piece name="space" scale="yes" level="yes" random="yes">
<image file="back_%l_16.bmp" />
<image file="back_%l_17.bmp" />
<image file="back_%l_18.bmp" />
<image file="back_%l_19.bmp" />
</piece>

<piece name="wall" scale="yes" bevel="16" level="no">
<image file="back_%l_00.bmp" />
<image file="back_%l_01.bmp" />
<image file="back_%l_02.bmp" />
<image file="back_%l_03.bmp" />
<image file="back_%l_04.bmp" />
<image file="back_%l_05.bmp" />
<image file="back_%l_06.bmp" />
<image file="back_%l_07.bmp" />
<image file="back_%l_08.bmp" />
<image file="back_%l_09.bmp" />
<image file="back_%l_10.bmp" />
<image file="back_%l_11.bmp" />
<image file="back_%l_12.bmp" />
<image file="back_%l_13.bmp" />
<image file="back_%l_14.bmp" />
<image file="back_%l_15.bmp" />
</piece>

<piece name="player_one" scale="yes" mover="yes">
<image file="char_01.bmp" key="#000000" />
<image file="char_34.bmp" key="#000000" />
<image file="char_48.bmp" key="#000000" />
<image file="char_37.bmp" key="#000000" />
<image file="char_47.bmp" key="#000000" />
<image type="small" file="char_01.bmp" key="#000000" />
</piece>

<piece name="player_two" scale="yes" mover="yes">
<image file="char_00.bmp" key="#000000" />
<image file="char_30.bmp" key="#000000" />
<image file="char_09.bmp" key="#000000" />
<image file="char_33.bmp" key="#000000" />
<image file="char_08.bmp" key="#000000" />
<image type="small" file="char_00.bmp" key="#000000" />
</piece>

<piece name="map_top_left" scale="yes">
<image file="char_07.bmp" key="#000000" />
</piece>

<piece name="map_top_right" scale="yes">
<image file="char_19.bmp" key="#000000" />
</piece>

<piece name="map_bottom_left" scale="yes">
<image file="char_38.bmp" key="#000000" />
</piece>

<piece name="map_bottom_right" scale="yes">
<image file="char_39.bmp" key="#000000" />
</piece>

<piece name="dots_x" scale="yes">
<image file="char_13.bmp" key="#000000" />
</piece>

<piece name="dots_y" scale="yes">
<image file="char_12.bmp" key="#000000" />
</piece>

<piece name="arrow_red_down" scale="yes">
<image file="char_02.bmp" key="#000000" />
</piece>

<piece name="arrow_red_left" scale="yes">
<image file="char_03.bmp" key="#000000" />
</piece>

<piece name="bomb_red_down" scale="yes">
<image file="char_15.bmp" key="#000000" />
</piece>

<piece name="bomb_red_left" scale="yes">
<image file="char_14.bmp" key="#000000" />
</piece>

<piece name="star" scale="yes">
<image file="char_05.bmp" key="#000000" />
<image type="small" file="char_05.bmp" key="#000000" />
</piece>

<piece name="door" scale="yes">
<image file="char_16.bmp" key="#000000" />
<image type="small" file="char_16.bmp" key="#000000" />
</piece>

<piece name="circle" scale="yes">
<image file="char_04.bmp" key="#000000" />
</piece>

<piece name="teleport" scale="yes">
<image file="char_11.bmp" key="#000000" />
</piece>

<piece name="switch" scale="yes">
<image file="char_06.bmp" key="#000000" />
</piece>

<piece name="explosion_red_left" scale="yes" animate="yes">
<image file="char_20.bmp" key="#000000" />
<image file="char_21.bmp" key="#000000" />
<image file="char_22.bmp" key="#000000" />
<image file="char_23.bmp" key="#000000" />
<image file="char_24.bmp" key="#000000" />
<image file="char_25.bmp" key="#000000" />
<image file="char_26.bmp" key="#000000" />
<image file="char_27.bmp" key="#000000" />
<image file="char_28.bmp" key="#000000" />
<image file="char_29.bmp" key="#000000" />
</piece>
<piece name="explosion_red_horizontal" scale="yes" animate="yes">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_right" scale="yes" animate="yes">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_top" scale="yes" animate="yes">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_vertical" scale="yes" animate="yes">
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_bottom" scale="yes" animate="yes">
<clone piece="explosion_red_left" />
</piece>

<piece name="darkness" scale="yes">
<image colour="#000000" />
</piece>

</pieces>
</chroma>
EOF
    close(FILE);
}

sub convert_atari_all
{
    my $file_input = shift(@_);
    my $output_prefix = shift(@_);

    make_directory($output_prefix);
    make_directory($output_prefix."/levels/");
    make_directory($output_prefix."/levels/xor_atari/");
    make_directory($output_prefix."/graphics/");
    make_directory($output_prefix."/graphics/xor_atari/");

    convert_atari_levels($file_input, $output_prefix."/levels/xor_atari/xor_atari_");
    create_set($output_prefix."/levels/xor_atari/set.chroma", "XOR (Atari ST)");

    convert_atari_graphics($file_input, $output_prefix."/graphics/xor_atari");

    open(FILE, ">".$output_prefix."/graphics/xor_atari.chroma");

print FILE << "EOF";
<chroma type="graphics">

<head>

<title>XOR (Atari ST)</title>

<sizes>
<!-- Everything is scaled from 24x22 -->
<size x="12" y="11" small="yes" pieces="yes" />
<size x="24" y="22" small="yes" pieces="yes" />
<size x="48" y="44" small="yes" pieces="yes" />
<size x="72" y="66" small="yes" pieces="yes" />
<size x="96" y="88" small="yes" pieces="yes" />
<size x="144" y="132" small="yes" pieces="yes" />
</sizes>

</head>

<background colour="#808080" />

<pieces path="xor_atari" levels="15">

<piece name="space" tile="yes" scale="yes" level="yes">
<image file="background_%l.bmp" />
</piece>

<piece name="wall" tile="yes" scale="yes" level="yes">
<image file="wall_%l_1.bmp" />
<image file="wall_%l_2.bmp" />
<image file="wall_%l_3.bmp" />
<image file="wall_%l_4.bmp" />
</piece>

<piece name="player_one" scale="yes" level="yes">
<image file="player_one_%l.bmp" key="#deface" />
<image type="small" file="player_one_%l.bmp" key="#deface" />
</piece>

<piece name="player_two" scale="yes" level="yes">
<image file="player_two_%l.bmp" key="#deface" />
<image type="small" file="player_two_%l.bmp" key="#deface" />
</piece>

<piece name="map_top_left" scale="yes">
<image file="map_top_left.bmp" key="#deface" />
</piece>

<piece name="map_top_right" scale="yes">
<image file="map_top_right.bmp" key="#deface" />
</piece>

<piece name="map_bottom_left" scale="yes">
<image file="map_bottom_left.bmp" key="#deface" />
</piece>

<piece name="map_bottom_right" scale="yes">
<image file="map_bottom_right.bmp" key="#deface" />
</piece>

<piece name="dots_x" scale="yes">
<image file="dots.bmp" key="#deface" />
</piece>

<piece name="dots_y" scale="yes">
<image file="waves.bmp" key="#deface" />
</piece>

<piece name="arrow_red_down" scale="yes" random="yes">
<image file="fish.bmp" key="#deface" />
<image file="fish_2.bmp" key="#deface" />
<image file="fish_3.bmp" key="#deface" />
<image file="fish_4.bmp" key="#deface" />
<image file="fish_5.bmp" key="#deface" />
</piece>

<piece name="arrow_red_left" scale="yes" random="yes">
<image file="chicken.bmp" key="#deface" />
<image file="chicken_2.bmp" key="#deface" />
<image file="chicken_3.bmp" key="#deface" />
<image file="chicken_4.bmp" key="#deface" />
<image file="chicken_5.bmp" key="#deface" />
</piece>

<piece name="bomb_red_down" scale="yes">
<image file="h_bomb.bmp" key="#deface" />
</piece>

<piece name="bomb_red_left" scale="yes">
<image file="v_bomb.bmp" key="#deface" />
</piece>

<piece name="star" scale="yes">
<image file="mask.bmp" key="#deface" />
<image type="small" file="mask.bmp" key="#deface" />
</piece>

<piece name="door" scale="yes">
<image file="door.bmp" key="#deface" />
<image type="small" file="door.bmp" key="#deface" />
</piece>

<piece name="circle" scale="yes">
<image file="doll.bmp" key="#deface" />
</piece>

<piece name="teleport" scale="yes">
<image file="teleport.bmp" key="#deface" />
</piece>

<piece name="switch" scale="yes">
<image file="switch.bmp" key="#deface" />
</piece>

<piece name="explosion_red_left" scale="yes" animate="yes" >
<image file="explosion_0.bmp" key="#deface" />
<image file="explosion_1.bmp" key="#deface" />
<image file="explosion_2.bmp" key="#deface" />
<image file="explosion_3.bmp" key="#deface" />
<image file="explosion_4.bmp" key="#deface" />
</piece>
<piece name="explosion_red_horizontal" scale="yes" animate="yes" >
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_right" scale="yes" animate="yes" >
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_top" scale="yes" animate="yes" >
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_vertical" scale="yes" animate="yes" >
<clone piece="explosion_red_left" />
</piece>
<piece name="explosion_red_bottom" scale="yes" animate="yes" >
<clone piece="explosion_red_left" />
</piece>

<piece name="darkness" scale="yes">
<image colour="#000000" />
</piece>

</pieces>
</chroma>
EOF
    close(FILE);
}

###############################################################################

sub convert_enigma
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    # single file
    if(-f $file_input)
    {
	convert_enigma_file($file_input, $file_output);
	return;
    }

    if(-d $file_input)
    {
	opendir(DIR, $file_input) || die "Couldn't open input directory ($file_input): $!\n";

        $output_prefix = $file_output;
        make_directory($output_prefix);
        make_directory($output_prefix."/levels/");
        make_directory($output_prefix."/levels/enigma/");

        create_set($output_prefix."/levels/enigma/set.chroma", "Enigma");

	while(($entry = readdir(DIR)) ne "")
	{
	    next if($entry eq "." || $entry eq "..");
	    next if($entry !~ /.level$/);

	    $output = $entry;
	    $output =~ s/.level$/.chroma/;

	    convert_enigma_file($file_input."/".$entry, $file_output."/levels/enigma/".$output);
	}

	closedir(DIR);
    }
    else
    {
        die "Couldn't open input directory ($file_input)\n";
    }

}

###############################################################################

sub convert_enigma_file
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    print "Converting '$file_input'\n";

    $title = ""; $height = 0; $width = 0; $map = "";

    open(INPUT, $file_input) || die "Couldn't open input file ($file_input): $!\n";
    while(<INPUT>)
    {
	if(/^Title: (.*)/)
	{
	    $title = $1;
	}
	if(/^Height: (\d+)/)
	{
	    $height = $1;
	}
	if(/^Width: (\d+)/)
	{
	    $width = $1;
	}
	if(/^Map: (.*)/)
	{
	    $line = $1;
	    $line =~ tr/-+|&~@E$<^>vYZWX/%%%%%1\/*abcdABCD/;
	    $map .= $line."\n";
	}
    }

    close(INPUT);

    open(OUTPUT, ">".$file_output) || die "Couldn't open output file ($file_output): $!\n";
    print OUTPUT << "EOF";
chroma level

title: Enigma: $title
mode: enigma
size: $width $height

data:
$map
EOF
    close(OUTPUT);

}

###############################################################################

sub convert_chroma2enigma
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    open(INPUT, $file_input) || die "Couldn't open input file ($file_input): $!\n";

    $size_x = 0;
    $size_y = 0;
    $title = "";

    $state = 0;
    while(<INPUT>)
    {
	$state = 1 if(/^chroma level/);
	next if($state == 0);

	if(/^size: (\d+) (\d+)/)
	{
	    $size_x = $1;
	    $size_y = $2;
	}
	if(/^title: (.*)/)
	{
	    $title = $1;
	}
	if(/^data:/)
	{
	    $state = 2;
	    last;
	}
 
    }

    if($state != 2)
    {
	die "Unable to locate data in input file";
    }

    $x = 0;
    $y = 0;

    while(!eof(INPUT) && $y < $size_y)
    {
	$c = getc(INPUT);
	next if($c eq "\r" || $c eq "\n");

	$map[$x][$y] = $c;
	$x ++;
	if($x == $size_x)
	{
	    $x = 0; $y ++;
	}
    }

    close(INPUT);

    open(OUTPUT, ">".$file_output) || die "Unable to open output file: $!\n";
    print OUTPUT "Title: $title\n";
    print OUTPUT "Height: $size_y\n";
    print OUTPUT "Width: $size_x\n";
    for($y = 0; $y < $size_y; $y ++)
    {
	print OUTPUT "Map: ";
	for($x = 0; $x < $size_x; $x ++)
	{
	    $c = $map[$x][$y];
	    $c =~ tr/1\/*abcdABCD/@E$<^>vYZWX/;
	    print OUTPUT $c;
	}
	print OUTPUT "\n";
    }
    close(OUTPUT);

}

###############################################################################
#
# To replace level 1 of the BBC XOR disc image with the XOR regression tests:
#     convert2chroma.pl --chroma2xor levels/regression/xor-regression.chroma regression.xor xor-bbc.ssd 1

sub convert_chroma2xor
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    open(INPUT, $file_input) || die "Couldn't open input file ($file_input): $!\n";

    $size_x = 0;
    $size_y = 0;
    $title = "";

    $state = 0;
    while(<INPUT>)
    {
	$state = 1 if(/^chroma level/);
	next if($state == 0);

	if(/^size: (\d+) (\d+)/)
	{
	    $size_x = $1;
	    $size_y = $2;
	}
	if(/^title: (.*)/)
	{
	    $title = $1;
	}
	if(/^view1: (\d+) (\d+)/)
	{
	    $data{"view1x"} = $1;
	    $data{"view1y"} = $2;
	}
	if(/^view2: (\d+) (\d+)/)
	{
	    $data{"view2x"} = $1;
	    $data{"view2y"} = $2;
	}
	if(/^viewteleport1: (\d+) (\d+) \((\d+) (\d+)\)/)
	{
	    $data{"viewteleport1x"} = $1;
	    $data{"viewteleport1y"} = $2;
	    $data{"teleport1x"} = $3;
	    $data{"teleport1y"} = $4;
	    $data{"teleports"} ++;
	}
	if(/^viewteleport2: (\d+) (\d+) \((\d+) (\d+)\)/)
	{
	    $data{"viewteleport2x"} = $1;
	    $data{"viewteleport2y"} = $2;
	    $data{"teleport2x"} = $3;
	    $data{"teleport2y"} = $4;
	    $data{"teleports"} ++;
	}
	if(/^data:/)
	{
	    $state = 2;
	    last;
	}
 
    }

    if($state != 2)
    {
	die "Unable to locate data in input file";
    }

    $x = 0;
    $y = 0;

    $base = 0x900;

    while(!eof(INPUT) && $y < $size_y)
    {
	$c = getc(INPUT);
	next if($c eq "\r" || $c eq "\n");

	$p = 0x0;
	$p = 0x1 if($c eq "%");
	$p = 0x2 if($c eq "1");
	$p = 0x3 if($c eq "2");
	$p = 0x4 if($c eq "M" || $c eq "m" || $c eq "N" || $c eq "n");
	$p = 0x5 if($c eq "-");
	$p = 0x6 if($c eq "|");
	$p = 0x7 if($c eq "d");
	$p = 0x8 if($c eq "a");
	$p = 0x9 if($c eq "D");
	$p = 0xa if($c eq "A");
	$p = 0xb if($c eq "*");
	$p = 0xc if($c eq "/");
	$p = 0xd if($c eq "o");
	$p = 0xe if($c eq "T");
	$p = 0xf if($c eq "S");

	if($c eq "M") { $data{"mapTLx"} = $x; $data{"mapTLy"} = $y; }
	if($c eq "m") { $data{"mapTRx"} = $x; $data{"mapTRy"} = $y; }
	if($c eq "N") { $data{"mapBLx"} = $x; $data{"mapBLy"} = $y; }
	if($c eq "n") { $data{"mapBRx"} = $x; $data{"mapBRy"} = $y; }
	if($c eq "1") { $data{"player1x"} = $x; $data{"player1y"} = $y; }
	if($c eq "2") { $data{"player2x"} = $x; $data{"player2y"} = $y; }
	if($c eq "*") { $data{"stars"} ++; }
	if($c eq "T" && $data{"teleports"} != 2)
	{
	    $data{"teleports"} ++;
	    $data{"teleport".$data{"teleports"}."x"} = $x;
	    $data{"teleport".$data{"teleports"}."y"} = $y;
	}

	$map[$x][$y] = $p;
	$x ++;
	if($x == $size_x)
	{
	    $x = 0; $y ++;
	}
    }

    close(INPUT);

    open(OUTPUT, ">".$file_output) || die "Unable to open output file: $!\n";
    binmode OUTPUT;

    for($y = 1; $y < 31; $y ++)
    {
	for($x = 1; $x < 31; $x += 2)
	{
	    print OUTPUT chr($map[$x][$y] + 16 * $map[$x + 1][$y]);
	}
    }

    # Sanitise viewpoints
    if(!defined($data{"view1x"}))
    {
	$data{"view1x"} = $data{"player1x"} - 3; 
	$data{"view1y"} = $data{"player1y"} - 3; 
    }
    if(!defined($data{"view2x"}))
    {
	$data{"view2x"} = $data{"player2x"} - 3; 
	$data{"view2y"} = $data{"player2y"} - 3; 
    }
    if(!defined($data{"viewteleport1x"}))
    {
	$data{"viewteleport1x"} = $data{"teleport1x"} - 3; 
	$data{"viewteleport1y"} = $data{"teleport1y"} - 3; 
    }
    if(!defined($data{"viewteleport2x"}))
    {
	$data{"viewteleport2x"} = $data{"teleport2x"} - 3; 
	$data{"viewteleport2y"} = $data{"teleport2y"} - 3; 
    }

    # Ensure viewpoints are not offscreen
    foreach $i ("view1x", "view1y", "view2x", "view2y", "viewteleport1x", "viewteleport1y", "viewteleport2x", "viewteleport2y")
    {
	if($data{$i} < 0)
	{
	    $data{$i} = 0;
	}
	if($data{$i} > 24)
	{
	    $data{$i} = 24;
	}
    }

    # Sanitise title
    $title =~ s/^XOR: //;
    $title =~ tr/a-z/A-Z/;
    $title =~ tr/A-Z ?//cd;
    $title = substr($title, 0, 19);
    if(length($title < 19))
    {
        $l = length($title);
        $title = " "x((19 - $l)/2).$title." "x((19 - $l)/2);
        $title .= " " if(length($title != 19));
    }
    $title .= "\r";

    # Map data
    foreach $i ("TL", "TR", "BL", "BR")
    { 
	if(defined($data{"map".$i."x"}))
	{
            print OUTPUT pack("v", $base + $data{"map".$i."x"} + $data{"map".$i."y"} * 32 - 32);
	}
	else
	{
	    print OUTPUT pack("v", 0);
	}
    }

    # Player and view data
    for($i = 1; $i <= 2; $i ++)
    {
        print OUTPUT pack("v", $base + $data{"player".$i."x"} + $data{"player".$i."y"} * 32 - 32);
        print OUTPUT pack("c", $data{"player".$i."x"} - $data{"view".$i."x"});
        print OUTPUT pack("c", $data{"player".$i."y"} - $data{"view".$i."y"});
        print OUTPUT pack("c", $data{"view".$i."x"});
        print OUTPUT pack("c", $data{"view".$i."y"});
        print OUTPUT pack("v", $base + $data{"view".$i."x"} + $data{"view".$i."y"} * 32);
    }

    # Star count and padding
    print OUTPUT pack("c", 0x02);
    print OUTPUT pack("c", 0x02);
    print OUTPUT pack("c", $data{"stars"});
    print OUTPUT pack("c", 0x00);
    print OUTPUT pack("c", 0x00);
    print OUTPUT pack("c", 0x00);
    print OUTPUT pack("c", 0x00);
    print OUTPUT pack("c", 0x00);

    # Teleport data
    if($data{"teleports"} != 2 )
    {
	print OUTPUT pack("Z30", "");
    }
    else
    {
	for($i = 1; $i <= 2; $i ++)
	{
            print OUTPUT pack("v", $base + $data{"teleport".$i."x"} + $data{"teleport".$i."y"} * 32);
            print OUTPUT pack("c", $data{"teleport".$i."x"} - $data{"viewteleport".$i."x"});
            print OUTPUT pack("c", $data{"teleport".$i."y"} - $data{"viewteleport".$i."y"});
            print OUTPUT pack("c", $data{"viewteleport".$i."x"});
            print OUTPUT pack("c", $data{"viewteleport".$i."y"});
            print OUTPUT pack("v", $base + $data{"viewteleport".$i."x"} + $data{"viewteleport".$i."y"} * 32);
	}

	print OUTPUT pack("Z14", "");
    }

    close(OUTPUT);

    # Optionally insert the file into the BBC XOR disc image
    if($ARGV[3] ne "")
    {
	$file_discimage = $ARGV[3];

	if($ARGV[4] ne "")
	{
	    $level = $ARGV[4];
	}
	if($level < 1 || $level >= 15)
	{
	    $level = 1;
	}

	$xor = read_file($file_discimage);
	$insert = read_file($file_output);

	$index_numbers  = locate_string(\$xor, "\x00\x4d\x00\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00\x09\x01\x00\x01\x01\x01\x02\x01\x03\x01\x04\x01\x05");

        # BBC XOR
	if($index_numbers >= 0)
	{
	    $offset = $index_numbers + 0x179 + (15 - $level) * 0x200;
	    insert(\$xor, \$insert, $offset);

	    $offset = $index_numbers + 0x22 + ($level - 1) * 23;
	    insert(\$xor, \$title, $offset);

	    open(OUTPUT, ">".$file_discimage) || die "Unable to open output file: $!\n";
	    binmode OUTPUT;
	    print OUTPUT $xor;
	    close(OUTPUT);
	}
	else
	{
	    die "Unable to identify file\n";
	}
    }

}

sub insert
{
    my $data = shift(@_);
    my $string = shift(@_);
    my $offset = shift(@_);

    my $tmp = shift(@_);

    $tmp = substr($$data, 0, $offset);
    $tmp .= $$string;
    $tmp .= substr($$data, $offset + length($$string));

    $$data = $tmp;
}


###############################################################################

sub convert_bbc_patch
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    $xor = read_file($file_input);

    $index_skip = locate_string(\$xor, "\xad\x6e\x25\xc9\x0b\x30\x06\xee\x6e\x25");
    # BBC XOR
    if($index_skip != 0)
    {
        $xor =~ s/\xad\x6e\x25\xc9\x0b\x30\x06\xee\x6e\x25/\xad\x6e\x25\xc9\x0b\x30\x06\xea\xea\xea/;
        $xor =~ s/\xc9\x0b\x30\x07\x29\x01\xf0\x03\xce\x6e\x25/\xc9\x0b\x30\x07\x29\x01\xf0\x03\xea\xea\xea/;
	open(OUTPUT, ">".$file_output) || die "Unable to open output file: $!\n";
	binmode OUTPUT;
	print OUTPUT $xor;
	close(OUTPUT);
    }
    else
    {
	die "Unable to identify file\n";
    }
}


###############################################################################

sub convert_amiga_patch
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    open(IN, $file_input) || die "Unable to open input file: $!\n";
    open(OUT,">".$file_output) || die "Unable to open outfile: $!\n";
    binmode IN;
    binmode OUT;
    while(<IN>)
    {
        $line = $_;
        # Patch protection routine
        if($line =~ /\x00\x00\x00\x06\x8c\xc0\x60\x00\xfc\x84\x4a\xac\x8c\xa8\x6f\x24/)
        {  
            $line =~ s/\x00\x00\x00\x06\x8c\xc0\x60\x00\xfc\x84\x4a\xac\x8c\xa8\x6f\x24/\x00\x00\x00\x06\x8c\xc0\x60\x00\xfc\x84\x4a\xac\x8c\xa8\x60\x24/;
            $ok |= 1;
        }
        # Patch checksum
        if($line =~ /\xc0\xf8\x37\xb4/)
        {  
            $line =~ s/\xc0\xf8\x37\xb4/\xc0\xf8\x46\xb4/;
            $ok |= 2;
        }
        print OUT $line;
    }
    close OUT;
    close IN;

    if($ok != 3)
    {
        die "Unable to identify file\n";
    }
}

###############################################################################
#
# http://www.rvvz.demon.nl/xor/
# A JavaScript version by Rob Veldhuyzen van Zanten
#
sub convert_zanten_level
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    print "Converting '$file_input'\n";

    $title = ""; $height = 0; $width = 0; $map = "";

    open(INPUT, $file_input) || die "Couldn't open input file ($file_input): $!\n";
    while(<INPUT>)
    {
	if(/^spel='(.*)'/)
	{
	    $title = $1;
	}
	if(/^'(.*)'/)
	{
	    $line = $1;
            $width = length($line);
            $line =~ tr/ #UWpqrs\-|<v@omZxBg/ %12MmNn\-|adAD*\/oTS/;
	    $map .= $line."\n";
            $height ++;
	}
    }

    close(INPUT);

    open(OUTPUT, ">".$file_output) || die "Couldn't open output file ($file_output): $!\n";
    print OUTPUT << "EOF";
chroma level

title: XOR: $title
mode: xor
size: $width $height

data:
$map
EOF
    close(OUTPUT);

}

###############################################################################
#
# http://xor.ovine.net
# Ovine's "remake" has the worst XOR engine I've come across - thoroughly
# broken precedence and bombs that can be pushed against their gravity. Quite
# how well levels written for such an engine will work with Chroma is anyone's
# guess, but converting their level files is simple enough:
#
# 0x000 - 0x3ff Map data (32 * 32, bytes as per BBC XOR except for maps)
# 0x400 - 0x401 Masks total
# 0x402 - 0x411 Title
# 0x412 - 0x437 Unused?

sub convert_ovine_level
{
    my $file_input = shift(@_);
    my $file_output = shift(@_);

    print "Converting '$file_input'\n";

    $level_data = read_file($file_input);
    $bad = 1 if(length($level_data) != 0x438);

    $title = ""; $map = "";

    $o = 0;
    for($y = 0; $y < 32; $y ++)
    {
        for($x = 0; $x < 32; $x ++)
        {
            $c = byte(\$level_data, $o);
            $map .= " " if($c == 0x00);
            $map .= "%" if($c == 0x01);
            $map .= "1" if($c == 0x02);
            $map .= "2" if($c == 0x03);
            $map .= "-" if($c == 0x05);
            $map .= "|" if($c == 0x06);
            $map .= "d" if($c == 0x07);
            $map .= "a" if($c == 0x08);
            $map .= "D" if($c == 0x09);
            $map .= "A" if($c == 0x0a);
            $map .= "*" if($c == 0x0b);
            $map .= "/" if($c == 0x0c);
            $map .= "o" if($c == 0x0d);
            $map .= "T" if($c == 0x0e);
            $map .= "S" if($c == 0x0f);
            $map .= "M" if($c == 0x19);
            $map .= "m" if($c == 0x16);
            $map .= "N" if($c == 0x18);
            $map .= "n" if($c == 0x17);
            $bad = 1 if($c >= 0x1a || $c == 0x04);
            $o ++;
        }
        $map .= "\n";
    }

    if($bad)
    {
        print "ERROR: Couldn't convert file\n";
        return;
    }

    $o = 0x402;
    while(byte(\$level_data, $o) != 0 && $o < 0x412)
    {
        $title .= chr(byte(\$level_data, $o));
        $o ++;
    }

    open(OUTPUT, ">".$file_output) || die "Couldn't open output file ($file_output): $!\n";
    print OUTPUT << "EOF";
chroma level

title: XOR: $title
mode: xor
size: 32 32

data:
$map
EOF
    close(OUTPUT);

}
