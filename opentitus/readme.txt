Readme for OpenTitus
Version 0.8.0
http://opentitus.sourceforge.net/

OpenTitus is released under the Gnu GPL version 3 or (at your option) any later version of the license, and it is released "as is"; without any warranty.

The OpenTitus team:
Eirik Stople - Main developer - eirik@pcfood.net
Eitan Tal - Valuable assistant, technical advisor

Thanks to:
Eric Zmiro and his team in Titus Interactive who made the great games Titus the Fox and Moktar!
Jesse, who made a fansite for the original game and an editor and provided me information how the original game files was built up, and assisted me with the makefile and MinGW!
The SDL project (LibSDL, LibSDL_mixer)!
The GNU project (GCC, GDB and GPL)!
MinGW/msys!
Ubuntu (programming environment)!
Linux!
Dosbox!
Dropbox!
GEdit!
Notepad++!
Google search!
Chocolate-Doom's great OPL (adlib) emulator!
Various tutorials/information sources on the net!



You need the original game files to make use of OpenTitus. OpenTitus parses the original files. It works with both Titus the Fox and Moktar.
Place the original game files in the "titus" or "moktar" folder. The files must be placed according to titus.conf.

Runtime dependencies:
SDL runtime library
SDL_mixer runtime library

To run the game, run opentitus (in Linux), opentitus.exe (windows) or opentitus_0.8.0.dge (dingux).

To quickly start up Titus, copy the game files from your original Titus game directory to the "titus" subfolder.
To quickly start up Moktar, copy the game files from your original Moktar game directory to the "moktar" subfolder. Rename "titus_moktar.conf" to "titus.conf" to play Moktar.

Compile instructions and platform specific information are available in the readme file for the target system (_linux, _win32, _dingux).

The game engine are ported from the original game, and modified to fit OpenTitus' level structure. If you find a bug or differences between OpenTitus and the original games, feel free to contact us.

Enjoy!



"Titus the Fox: To Marrakech and Back" (1992) and "Lagaf': Les Aventures de Moktar - Vol 1: La Zoubida" (1991)
was developed by, and is probably copyrighted by Titus Software, which, according to Wikipedia, stopped buisness in 2005.
OpenTitus is not affiliated with Titus Software.

The source code have been developed during several years, and some parts of the code are old. This code doesn't follow the more recent style, such as "unsigned char" instead of "uint8". As the old code still works, it's not that important to fix it.

OpenTitus is configured by a file in the same directory as the executable; "titus.conf". This file is parsed line by line, and lines starting with # will be skipped.

Keywords:
reswidth = Number of pixels wide, default 320 (originally 640, but the "pixels" in the original game was two pixels wide, so 320 should give the correct size)
resheight = Number of pixels height, default 200 (originally 400 (intro) or 384 (in-game), 200 should give the correct size)
bitdepth = Number of bits pr. pixel, default 32 (the game data files have a 16 colour palette, which means 4 bpp, but higher bpp will work as well)
ingamewidth = Numbers og pixels visible in-game, default 320.
ingameheight = Numbers og pixels visible in-game, default 192.
audiofile = Location of the audio file.
videomode = Video mode; 0 is window mode and 1 is fullscreen, default 0
levelcount = Number of levels, default 15 (15 in Titus the Fox and 16 in Moktar)
level 1-15 (1-16 for Moktar) = Location of the level files for all levels, default LEVELx.SQZ (the order is "0J123456789BCEFG" for Moktar, Titus is equal except it doesn't have level F)
sprites = Location of the sprite file, default SPREXP.SQZ (SPRITES.SQZ in Moktar)
game = The game opentitus will parse; 0 for Titus the Fox and 1 for Moktar. This is necessary because of different sprite sizes, different menus and different level titles.
logo = The company logo file, default TITUS.SQZ
intro = The game intro image file, default TITRE.SQZ
menu = The menu, default MENU.SQZ
finish = The image displayed when game is completed, default LEVELA.SQZ (this doesn't exist in Moktar)
*format = Image format. 0: 16-colour grayscale, 1: 16-colour (ordinary colours), 2: 256 colours + palette. Logo, intro and menu are 256 colour, *EGA files are 16-colour, and LEVELA.SQZ are grayscale
font = Font file, default FONTS.SQZ