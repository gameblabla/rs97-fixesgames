+------------------------------------------------------------------------+
|                                                                        |
|                      REminiscence port for GCW0:                       |
|                                                                        |
+------------------------------------------------------------------------+

About:
------

This port is my first trial at OpenSource development, SDL programing and generally
speaking Linux. While rather an impressive game all the credit should go to cyx
(Gregory Montoir) for his great work on REminiscence, because this "port" is merely
just a simple recompile with a few settings altered.

The SourceCode is avaible in "https://github.com/ElwingGit/GCW0_REminiscence.git".

Game data:
----------

Flashback is not a free game, to be able to play it with this new engine you need
the game data. This port should support all the file format supported by Cyx's
REminiscence.

* Required data:
The PC game data, both the DOS and the CD-ROM version are supported (in my opinion
vector graphics looks better than the video of the CD-ROM version...).

*optional datas:
The Amiga mod files can be added to get music during the cinematics.
The SegaCD speech file (voice.vce) for in game speech.

Installation:
------

REminiscence by default will create a folder for the data and the savefile
if it did not exist before. Thus you can either:
- copy the opk on the GCW0
- run the opk once.
- copy the datafile in "/usr/local/home/.REminiscence/data"
- start the opk again and enjoy the game.
or:
- copy the opk on the GCW0
- create the folder and copy the datafile in "/usr/local/home/.REminiscence/data"
- start the opk again and enjoy the game.

Key Mapping:
------------
This port offer two different key mappings, here the default key mapping (typeA):
    Arrow Keys      move Conrad
    A               use the current inventory object
    B               talk / use / run / shoot
    X               draw gun
    Y               display the inventory
    Start           display the options

and here is the second mapping (typeB):
    Arrow Keys      move Conrad
    A               draw gun
    B               use the current inventory object
    X               talk / use / run / shoot
    Y               display the inventory
    Start           display the options

you can change the mapping used from the game main menu, in the "info" submenu.

Credits:
--------
- Gregory Montoir of course and all the people who helped him with the original REminiscence.
- Hi-Ban who provided some great icon to replace the copyright infringing icon I had before.
- all GCW0 team for their nice toolchain, their testing done, the GCW0 itself and the nice
  repo.



+------------------------------------------------------------------------+
|                                                                        |
|                      Original REminiscence ReadMe:                     |
|                                                                        |
+------------------------------------------------------------------------+


REminiscence README
Release version: 0.2.1 (Mar 15 2011)
-------------------------------------------------------------------------------


About:
------

REminiscence is a re-implementation of the engine used in the game Flashback
made by Delphine Software and released in 1992. More informations about the
game can be found at [1], [2] and [3].


Compiling:
----------

Update the defines in the Makefile if needed. The SDL and zlib libraries are required.


Data Files:
-----------

You will need the original files of the PC (DOS or CD) or Amiga release.
If you have a version distributed by SSI, you'll have to rename the files
and drop the 'ssi' suffix (ie. logosssi.cmd -> logos.cmd).

To hear background music during polygonal cutscenes with the PC version,
you'll need to copy the .mod files of the Amiga version :

	mod.flashback-ascenseur
	mod.flashback-ceinturea
	mod.flashback-chute
	mod.flashback-desintegr
	mod.flashback-donneobjt
	mod.flashback-fin
	mod.flashback-fin2
	mod.flashback-game_over
	mod.flashback-holocube
	mod.flashback-introb
	mod.flashback-jungle
	mod.flashback-logo
	mod.flashback-memoire
	mod.flashback-missionca
	mod.flashback-options1
	mod.flashback-options2
	mod.flashback-reunion
	mod.flashback-taxi
	mod.flashback-teleport2
	mod.flashback-teleporta
	mod.flashback-voyage

To hear voice during in-game dialogues, you'll need to copy the 'VOICE.VCE'
file from the SegaCD version to the DATA directory.


Running:
--------

By default, the engine will try to load the game data files from the 'DATA'
directory (as the original game did). The savestates are saved in the current
directory. These paths can be changed using command line switches :

    Usage: rs [OPTIONS]...
    --datapath=PATH   Path to data files (default 'DATA')
    --savepath=PATH   Path to save files (default '.')

In-game hotkeys :

    Arrow Keys      move Conrad
    Enter           use the current inventory object
    Shift           talk / use / run / shoot
    Escape          display the options
    Backspace       display the inventory
    Alt Enter       toggle windowed/fullscreen mode
    Alt + and -     change video scaler
    Ctrl S          save game state
    Ctrl L          load game state
    Ctrl + and -    change game state slot
    Ctrl R          toggle input keys record
    Ctrl P          toggle input keys replay

Debug hotkeys :

    Ctrl F          toggle fast mode
    Ctrl I          Conrad 'infinite' life
    Ctrl B          toggle display of updated dirty blocks
    Ctrl M          mirror mode (right - left swapped)


Credits:
--------

Delphine Software, obviously, for making another great game.
Yaz0r, Pixel and gawd for sharing information they gathered on the game.
Nicolas Bondoux for sound fixes.


Contact:
--------

Gregory Montoir, cyx@users.sourceforge.net


URLs:
-----

[1] http://www.mobygames.com/game/flashback-the-quest-for-identity
[2] http://en.wikipedia.org/wiki/Flashback:_The_Quest_for_Identity
[3] http://ramal.free.fr/fb_en.htm
