-=TileWorld2X Beta 1=-
Copyright (c)2007 Dan Silsby
GNU GPL license, source may be found on
gp2x.de

******BETA RELEASE, BUGS MIGHT BE PRESENT******

Tileworld2X Ported by Dan Silsby 11/07/2007
(Senor Quack on gp32x.com and #gp2xdev on EFNET)
Music files written by chaozz of gp32x.com
and am-fm's music is used under Creative Commons
License.

-=WHAT IS IT?=-

TileWorld is an open source interpreter for
Chip's Challenge levels.  Chip's Challenge remains
a popular game today, despite having been 
introduced in 1989 on the Lynx handheld.  The
version that truly launched Chip's Challenge,
however, was the Microsoft Entertainment Pack
version released in the early nineties. 

Tileworld boasts improved tile graphics and
sound and fluid animation that the MS version
did not.

I have carefully ported TileWorld to the small
screen of the GP2X, and basically rewritten the
entire GUI, as well as a lot of the file handling
code and all of the sound code.  TileWorld2x 
boasts improvements over the desktop version of
TileWorld, including music support, improved
sound samples (OK, one), a simplified and prettier 
GUI, and automated handling of configuration files.

For more info on TileWorld and Chip's Challenge, 
visit the main TileWorld page or Wikipedia:

http://www.muppetlabs.com/~breadbox/software/tworld/
http://en.wikipedia.org/wiki/Chip's_Challenge

NOTICE: The original Chip's Challenge game is 
	NOT OK to redistribute.  If you wish to
	play the original levels from the game,
	(and why wouldn't you?) you will have 
	to copy the original CHIPS.DAT file
	into the data/ folder yourself.  It 
	is best to locate the Windows version.
	TileWorld2x includes over a dozen end-user 
	created	levels, including CCLP2, the 
	unofficial fan-made "sequel" to the 
	original game.  Thus, there is no need 
	to obtain the original game to have some 
	fun. ;)
	
-=OK, WHAT DO I NEED TO KNOW?=-

If you need to learn how to play Chip's Challenge,
there is an introductory level at the top of the
level selection list in TileWorld2X.

You can move Chip about using either the joystick
or the 4 buttons on the right side of the GP2X.

There are thousands of user-created levels
easily found on the Internet:
http://www.pillowpc2001.net/chips/levels.htm
http://www.ecst.csuchico.edu/~pieguy/chips/main.php
http://games.groups.yahoo.com/group/chips_challenge/
http://chips.kaseorg.com/

Some are crappy, some are quite fun.  I have included
some fun ones already.  If you want to
add more to the ones already included, it is very easy.
Simply copy the levelset data file into the data/
folder under tileworld2x.  That's it, just make sure
to unzip it first!  File extension, upper/lower case
do not matter at all, but it is important to
place it in data/, not sets/ and do not place it
in its own subfolder.

Tileworld2x is more intelligent than the original 
TileWorld and will automatically create two 
configuration  files for you, one named
LEVELSETNAME-MS.DAC, and the other 
LEVELSETNAME-LYNX.DAC.  You may now select these
from the main menu.

Why *two* configurations?  Because 
TileWorld is capable of emulating either system, 
Microsoft or Lynx.  Unfortunately, MS changed
some of the allowed behavior in their version 
and so frequently a user-created level 
cannot be played under Lynx mode.  This 
is unfortunate, because Lynx mode allows fluid 
animation and improved sounds.  Since many users
were MS users, you will sometimes have to use
MS mode and forego some fancier graphics in the
name of completing all the levels in a set.

-=CONTACT INFO=-
You can reach me via PM at GP32X.COM forums, or by
email at dansilsby <AT> gmail <DOT> com

Have fun and report bugs, remember this is a 
beta release, anything could happen!