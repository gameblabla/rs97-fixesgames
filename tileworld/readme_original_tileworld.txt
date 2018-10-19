
  Welcome to Tile World

Tile World is an emulation of the game "Chip's Challenge" for the Atari
Lynx, created by Chuck Sommerville, and later ported to MS Windows by
Microsoft (among other ports).

  Important Note

Tile World is an emulation of the "Chip's Challenge" game engines only. It
does not come with the chips.dat file that contains the original level set.
That file, which is copyrighted and cannot be freely distributed, was
originally distributed with the MS version of "Chip's Challenge". If you
have a copy of this version of the game, you can use that file to play the
original games in Tile World. If you do not have a copy of this file,
however, you can still play Tile World with the many freely available level
files created by fans of the original game.

  Installing Tile World under Windows

First of all, you'll want to store the files contained in this archive into
its own separate directory. If you're using the self-extracting executable,
you can create a new directory during the installation. Otherwise, you'll
need to create a new directory beforehand -- something like c:\tworld --
and extract the files in there.

If you have a copy of the chips.dat data file, copy it to the data
subdirectory. This will allow you to play the original levels under Tile
World (for the MS ruleset and the Lynx ruleset both).

If you have other data files that you would like to try out in Tile World,
copy those to the sets directory.

The shell commands to do the above would look something like:

  cd c:\wherever\my\copy\of\chips\challenge\is\at
  copy chips.dat c:\tworld\data
  cd c:\my\collection\of\dat\files
  copy *.dat c:\tworld\sets

That's all that needs to be done to set it up. Run the program as
c:\tworld\tworld, or create a shortcut for it.

  Installing Tile World under Linux

Before proceeding, ensure that you have SDL installed on your machine. (If
you don't have SDL, you can get it by visiting
http://www.libsdl.org/download.html. If you download a precompiled version
-- i.e., an .rpm or .deb file -- note that you will need the development
runtime, as opposed to the binary runtime.)

Installing Tile World involves the usual three-part incantation:

  ./configure

By default, the program is set up so that it will keep its shared files
under /usr/local/share/tworld. If you would prefer the tworld directory to
be somewhere besides /usr/local/share, use the --datadir option to change
it when you run ./configure. Alternately, you can use the
--with-sharedir=DIR option to explicitly specify a completely different
path. (This value can also be changed at runtime, either via the TWORLDDIR
environment variable or via the command line.)

  make

There shouldn't be any serious warnings from the compiler. Use "make
mklynxcc" if you want to also build a copy of mklynxcc (see below).

  make install

Running "make install" as root will do the following:

* Copy the tworld binary to /usr/local/games.
* Copy the tworld.6 manpage to /usr/local/man/man6.
* Create /usr/local/share/tworld if it does not exist. (Or whatever
  directory you specified to ./configure.)
* Copy the external resources (i.e., the bitmaps and wave files) to
  /usr/local/share/tworld/res.
* Create the directories /usr/local/share/tworld/data and
  /usr/local/share/tworld/sets.

The sets directory is where you will generally store the .dat files that
you want to use. However, if you want to make use of a configuration file
with a particular data file, then you will need to store the data file in
the data directory, and the configuration file goes into the sets directory
instead. See the documentation for more information.

  Level Sets

As mentioned above, the original "Chip's Challenge" level set does not come
with Tile World, for reasons of copyright. If you do not already have a
copy of Microsoft's Windows version of "Chip's Challenge", you might still
be able to find a copy. Search the links listed below, under "Resources on
the Internet", for helpful hints on finding the game online.

If and when you do, you can copy the chips.dat file from there into Tile
World's data directory. You will then be able to play the levels of the
original set (both in MS mode and in Lynx mode).

There are also many "user-created" level sets. These are sets of levels
which have been invented by fans of the game. These sets are freely
available for downloading. If you have a .dat file that contains a level
set and you wish to use it, just copy it to Tile World's sets directory.
The next time you start Tile World, the new .dat should appear in the list
of available level sets.

At http://groups.yahoo.com/group/chips_challenge/files/ is a repository of
the available level sets that have been created. The biggest fans of the
game try to provide a copy of every known user-created level set at this
place. I have not included any of these level sets in this distribution, as
the authors continue to add new levels to their sets over time.

Actually, this distribution does contain one small level set. This is
included so that even if you don't have the original level set, you can
still get a brief glimpse of how the game works, and what some of the most
basic challenges are. Also, the same set of levels can be played with both
the MS and Lynx ruleset, so you can see how they differ.

Finally, there is one level set that is special and deserves particular
mention. That is CCLP2.dat, or "Chip's Challenge Level Pack 2". This set
was assembled by the fans, who voted on all of the user-created levels that
existed at the time. The levels that were voted as being the most fun were
then all put together to become CCLP2. It is the closest thing we have to a
sequel for the original game. (But be careful: CCLP2 is much harder than
the original!) You can download a copy of Tile World with this set already
installed. If instead you download it separately, you will want to store it
in your data directory instead of your sets directory. (This is because
Tile World comes with a special configuration file for CCLP2.)

  The Complete Documentation

The full documentation for Tile World is included with the distribution, in
the file tworld.html. There you will find information on how to play the
game, adding new level sets, customizing Tile World, and more.

  Creating New Level Sets

The most widely used program for creating new level sets is ChipEdit. It
comes with excellent documentation, and you should have little trouble
learning how to use it. Some other editors have recently been made
available, such as CCEdit and Chip's Workshop.

Normally, ChipEdit creates levels for the MS ruleset. If you wish to make a
level set for the Lynx ruleset, you have a few options:

* A very simple command-line utility is included with Tile World, called
  mklynxcc. This program will change a normal .dat file to one that will
  use the Lynx ruleset instead of the MS ruleset. Running mklynxcc foo.dat
  will change foo.dat's ruleset from MS to Lynx.
* You can use a configuration file to override the builtin ruleset. This
  method requires creating an extra file, but does allows you to avoid
  making changes to the .dat file. See the complete documentation for
  information on how to set up a configuration file.
* Finally, ChipEdit has an obscure feature which allows you to control the
  signature of the data file. This is done by adding a SIGNATURE entry to
  the chipedit.ini file. The default signature value is 0x0002AAAC, which
  indicates a data file that uses the MS ruleset. If you set the SIGNATURE
  to be 0x0102AAAC, then ChipEdit will create data files marked to use the
  Lynx ruleset instead.

  Resources on the Internet

There is quite a bit of information about "Chip's Challenge" available on
the internet. Much of it is focused on maximizing your score on the
original level set for the MS game, but you will also find lots of general
help and useful information as well.

Jimmy Vermeer maintains a site that tracks people's scores on the original
level set and CCLP2, as well as information on other levels and links to
many other pages.

  http://www.geocities.com/purpletentacle1977ca/

Anders Kaseorg's site contains the Chip's Challenge FAQ, as well as the AVI
repository and a web interface to the newsgroup:

  http://chips.kaseorg.com/

The grand repository of user-created level sets can be found at the
chips_challenge Yahoo group:

  http://groups.yahoo.com/group/chips_challenge/files/

ChipEdit's home page is at:

  http://www.stage62.com/chipedit/chipedit.htm

Finally, Tile World's home page is at:

  http://www.muppetlabs.com/~breadbox/software/tworld/

  License

Tile World is copyright (C) 2001-2006 by Brian Raiter. This program is free
software; you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License,
included in this distribution in the file COPYING, for more details.

  Bugs

Bug reports are always appreciated, and can be sent to the author at
breadbox<at>muppetlabs.com<dot>com. The list of known bugs is at
http://www.muppetlabs.com/~breadbox/software/tworld/BUGS.html. Please check
here before sending a bug report, to to make sure the bug has not already
been documented.

  Credits

Tile World was written by Brian Raiter.

The sound effects included in this distribution were created by Brian
Raiter, with assistance from SoX. Brian Raiter has explictly placed these
files in the public domain.

The tile images included in this distribution were created by Anders
Kaseorg, with assistance from POV-Ray. Anders Kaseorg has explicitly placed
these files in the public domain.

The introductory set of levels included in this distribution were created
by Brian Raiter. Brian Raiter has explictly placed these levels in the
public domain.

"Chip's Challenge" was designed by Chuck Sommerville, who is also the
author of the original Lynx program.

"Chip's Challenge" is a registered trademark of Alpha Omega Publications.

Creating this program would have been flatly impossible without the help of
several fans of "Chip's Challenge". The author would particularly like to
acknowledge Anders Kaseorg for sharing the fruits of his investigations
into the game logic of the MS version and for being an effective bug
hunter, Chuck Sommerville for his pointers regarding the game logic of the
Lynx version and his unfailing support of this project, and "CCExplore" for
his in-depth investigations of esoteric game behavior.

Many other regulars of the annexcafe.chips.challenge newsgroup assisted
with bug reports, suggestions, and all-around encouragement. Their help is
gratefully acknowledged.

The anonymous author of the document describing the .dat file format, Don
Gregory, the "Charter Chipsters", and the contributors to the CC AVI
library all deserve mention as well -- this program would never have been
written without the information they made freely available.

Last but not least, a tip of the hat to John K. Elion for writing ChipEdit.

Brian Raiter
April 2006
