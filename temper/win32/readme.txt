Temper: Turbografx EMulator PEr Request

by Exophase (exophase@gmail.com)

-- Changelog --
Legend: @ - optimization, + - new feature, # - bug fix, - - removed feature

v0.75:
#  Fixed transfer operation bug that caused Shadow of the Beast and Steam
   Hearts to not work.
#  Fixed branch bug that caused Sidearms Special to not work, but at a slight
   performance expense.
#  Added back timer IRQ acknowledgement on IRQ enable read, seems to fix
   Afterburner II.
@  Expanded idle loop detection to handle some more games, meaning more speed
   in a few with the option on (Sapphire, Blazing Lasers, etc)
@  Wrote sprite rendering in ARM ASM, improvement of up to 1ms or more in
   frame rendering.
@  Wrote line fill in ARM ASM, small improvement sometimes (0.2ms in Blazing
   Lasers title screen)
@  Accidentally had something in the video rendering that shouldn't have been
   there, frees up 1.3ms per frame.
+  Added the ability to width scale the screen from 256 to 320. See options
   section for more details.
+  Added the ability to not limit the number of sprites per scanline. See options
   section for more details.

v0.7: Major release

+  CD-ROM support has been added. This includes CD, Super CD^2, and
   Arcade Card support. See the CD-ROM section for further details.
+  Support for BRAM. This isn't just pertinent to CD-ROM games since a number
   of card games can use it too. Unlike a real PC-Engine these are 8KB
   (instead of 2KB), which will sometimes give you more space (and hopefully
   won't cause problems).
+  Support for OGG compressed audio tracks, see CD-ROM section.
+  Support for Street Fighter 2 mapper (game should work fine)
+  Support for 6 button pad. Enable it in options.
+  Support for Populous's RAM, if for some reason you wanted to play that.
+  Savestates can now be BZ2 compressed, optionally (it's on by default).
   See options section for more details.
#  Fixed IRQ raising related bug, which fixes the scroll glitches in Air
   Zonk.
#  Changed the VDC timing to try to make it closer to a real PC-Engine
   after doing various tests with the hardware. Still isn't totally on the
   mark (some games still have graphical glitches, usually of the single
   scanline nature) but the number should be down.
#  Correctly implemented the csl/csh instructions, which together with
   the VDC fixes should fix scanline bugs in Violent Soldier.
#  Implemented the transfer instructions more thoroughly. Might not affect
   any card games, but at least one CD game depends on it.
#  Screen is now centered correctly.
#  Fixed a bug causing junk to appear on the edges of the screen when it's
   not large enough to fill the whole thing, sometimes.
#  Fixed a bug causing rapidfire to not work properly when other buttons
   were pressed at the same time.
#  Fixed various menu bugs.
#  Hopefully minimized some crash bugs.
@  Was accidentally clearing tile cache every frame. Oops (speed boost from
   this)
-  Took out audio frequency options because I didn't feel like dealing with
   it for CD stuff (so it's fixed at 44.1KHz)
-+ Removed "Force USA Encode" option since only one game needed it
   (Legend of Hero Tonma). Added further hueristic for this instead.
   However, it'd make the author of Mednafen a little happier if you stopped
   using these bitswapped ROMs altogether.

v0.56: Small bugfix release

#  Fixed bug where idle loop eliminated branches would crash if preceded
   by a cli (fixes Super Star Soldier)
#  Fixed a bug in tsb/trb emulation (fixes score in Parasol Stars, some
   other glitches in other games)
#  Fixed previous ROM directory not being loaded on startup.
#  Hopefully fixed GP2X freeze when loading a game after saving global config
+# Added options to save local/global config for pad as well.

v0.55: Thanks to Notaz for some help he gave on how to fix some things.

@  Changed something in video rendering that affected memory performance,
   games with low priority sprites will be faster now.
@  Optimized video rendering a bit, small speed improvement.
@  Added some more idle loop patterns, matches 14 more games. As many as
   95 games are still unmatched, but the actual number in real idle loops
   is probably much lower. Probably over 70% of games are matched
   currently.
+# Changed the way config file saving works. Will no longer save the global
   config automatically every time the options menu is exited, but instead
   there's an option to exit + save global (in addition to exit + save
   local and exit w/o saving). Current ROM directory was moved to a separate
   config file, temper.cf2.
#  Fixed bug where current directory mmuhack.o was not used. This will fix
   speed problems for people that couldn't get it loaded (thanks notaz!)
#  Fixed bug where Temper didn't reclaim audio after it quit. This will
   prevent Temper from ruining sound after exiting and hanging on next
   startup (thanks notaz!)
#  Fixed bug that could cause Temper to crash on exit, possibly bringing
   down the entire GP2X (thanks notaz!)
#  Disallowed the joystick read from returning conflicting buttons for
   F200 users, although you can still simulate this by programming different
   GP2X buttons to return them. At least one game was getting confused by
   this.
+  Added support for loading bz2 compressed ROMs.

v0.5: First release


-- About --

Temper is a PC-Engine/Turbografx 16 emulator written for GP2X. I wrote it
because a lot of GP2X users were complaining that the other emulators
weren't fast enough or full featured enough. So, the primary aim of Temper
is to be fast enough while not sacrificing features or compatibility. It
was written completely from scratch, although some source and individuals
were consulted to fill in some technical gaps thanks to the lack of much
strong publicly available documentation of the platform.


-- Features --

Temper currently only emulates the core PC-Engine platform. The CD-ROM
add-ons and SuperGrafx platform are not emulated, but might be emulated
at a later date (maybe more if you can make me happy with developing on
GP2X). 6 button pad, arcade card, or any of those other addons aren't
supported either. This is just a basic PC-Engine platform emulator with
no frills.

It does have a lot of basic features and options you'd come to expect
from an emulator on GP2X.


-- Compatibility --

The PC version (which has been heavily tested) has very good compatibility,
with HuCards, with no games outright not working. However, some games have
graphical glitches. Turning off "patch idle loops" might help sometimes,
otherwise leave it on because it'll make a lot of games much faster. Of
course there could be other things I've missed, who knows.

CD-ROM compatibility (again, on the PC version) is slightly worse, and not
all games have been tested (of a little over 200 tested, about 95% exhibit
no problems). Only one game (Popful Mail) is currently known to have severe
problems, although it's still perfectly playable.

I haven't tested the GP2X version that extensively, so if you find a problem
post about it on http://www.gp32x.com/board/ (look for the appropriate topic
first, there should be one made for Temper compatibility somewhere) and I'll
look into fixing it. ALMOST anything that works on the PC version shouldn't
be that hard to get working the same way on the GP2X version.


-- Performance --

It is written with a lot of ARM assembly which makes it a little faster
than the others. It could still be optimized a lot more, but at this point
it's basically "fast enough" in my opinion. I'll probably continue
optimizing it anyway, because that's what I do.

200MHz should hopefully be enough for all games, especially with fast RAM
timings on, but let me know if it isn't (on gp32x). You can try slower
speeds like 150MHz or lower, and you might get good results depending on
the game. Turning off audio won't change very much.

Currently there's no frameskip. I might add auto frameskip if there's a
demand for it (for playing at very low clockspeeds).

Using OGG compressed data tracks for CD games currently comes at a decent
performance price, although most games will still run fine most of the
time at 200MHz (turning on fast RAM timings will help too). If all else
fails the CPU can be overclocked past 200MHz.


-- CD-ROM --

As of version 0.7 Temper can play CD-ROM games. This includes Super CD^2,
Arcade Card, and Games Express Card games, but you must have the appropriate
syscard image in the syscards directory for any of this to work (and the one
you want to use must be selected in the menu). See the menu option for what
to name the files.

The CD-ROM images themselves must be derived from bin/cue. This means that
there should be:

- A cue sheet of the standard format.
- Either a single file (.bin or .iso) for the entire image or one per each
  track. The extension doesn't matter, but Temper supports either 2048 or
  2352 byte sectors for the images. For data tracks using 2048 bytes is
  advantageous since the others aren't used by games.
- Audio tracks may be .bin/.iso as above, .wav, or .ogg. WAVs may be anywhere
  from 11025Hz mono 8bit (8bit is assumed to be unsigned) to 44100Hz stereo
  16bit. OGGs must decode to stereo 44100Hz.

Basically, if you use a standard program to rip a CD as bin/cue then it will
work. Then, you can use the included bin_to_iso_ogg program to convert this
to iso/ogg/cue if you desire. Right now this program will only convert
bin/cue files, and if you use it on a case sensitive file system (like Linux)
you must make sure that the files in the CUE match the case of the BIN file
used.

NOTE: This program requires oggenc to be installed to your path (on Windows,
put it in c:\windows, on Linux in /usr/bin) to work. You can download oggenc
from http://www.rarewares.org/ogg.php.

Run the program as:

bin_cue_to_iso_ogg <name of cue file to convert> <base for output>

For instance if you did:

bin_cue_to_iso_ogg game.cue game

It would create the new cuesheet and track files in the directory game,
with names game.cue, and game_tN.* for the track files.


OGG playback is not without a price - at 200MHz some games might have some
slowdown. I'd like to have decoding happening on the second CPU, but for now
the best you can do is to stick with bin/cue or use fast RAM timings and/or
overclock.

Currently the CD-ROM access and load speeds are being emulated to be roughly
like they would on a real console. Some games require this to operate
properly. I'll probably add an option to disable this at some point.


If you try to load a cue sheet that Temper can't figure out (with invalid
file names for instance) it might drop back to the menu. If it can't load
any of the track files then this will happen. Try editing the CUE file
(in any standard text editor like notepad) and make sure that the names
used in the FILE lines match the files used, and that the type at the end
matches (BINARY for bin/iso, WAVE for wav, OGG for ogg).


-- Controls --

Menu:
  Up/Down/Left/Right: Navigate cursor
  B: select
  X: cancel
  A: move up directory for file browser
  L: page up for file browser
  R: page down for file browser

Default ingame:
  Up/Down/Left/Right: dpad
  X: II
  A: I
  Y/Vol middle: menu
  Start: Run
  Select: Select
  L: Load state
  R: Save state
  Vol up/down: change volume

You can change the default controls in the menu.


-- Menu options --

The main menu options should be self explanatory, with the exception of
"Swap CD." Although there are no multi-CD games there is one that lets you
use external CDs as data, so you can use this option for that.

The configure pad menu lets you change the pad mappings - on the right are
GP2X buttons and on the left are what they correspond to. These are what you
can map them to:

- PC-Engine controls (include six button controls)
- Save/load state
- Volume up/down
- Enter menu
- Toggle fastforward
- button rapidfire (like the fastest setting on a TurboGrafx controller)

If you don't set a menu button then vol middle will be forced to it.

The options menu has the following options available:

- Show fps (how many frames are emulated per second, w/o fastforward should be
  at roughly 60, use this to determine if the game isn't fullspeed)
- Enable sound (turning off sound won't improve performance)
- Fast forward (run the game as fast as possible w/o throttling)
- Clock speed (underclock for better battery life but worse performance, or
  overclock for worse battery life and better performance - warning, your GP2X
  could freeze if you set it too high. It only goes up to 300, but most won't
  be able to get that high...)
- Fast RAM timings - will improve performance a little, but some GP2X's might
  not be able to handle them, especially at higher clock speeds. Might go well
  in improving performance at lower clock speeds though.
- Gamma percent - from 0 to 300. Makes the display appear brighter or darker.
- Patch idle loops - this will detect when the ROM is just waiting around for
  something to happen and will speed it up, note that currently only very, very
  simple idle loops are detected so only some percentage of games get a benefit
  from this (don't know how many but looks to be over half). This can possibly
  cause glitches or problems in games so if it does turn it off, otherwise
  leave it on.
- Snapshot in saves - this option will save a snapshot in savestates, which you
  can see if you're over the load state option in the main menu (and if there
  is one saved for that slot with a snapshot). This is off by default because
  it currently about triples the size of the savestates.
- BZ2 compressed saves - use BZ2 compression to decrease the size of
  savestates. This also comes with an increased time to save/load them as
  well, so you can turn this off if that bothers you. But for arcade card
  games it's really best to leave it on if you don't want 2.5MB savestates.
- Use six button pad - allows the games to detect six button pad presses
  (note: they still have to be mapped in the config screen for it, they
  don't default to anything). Keeping this on by default is a bad idea since
  some older games don't work correctly this way.
- CD-ROM system - what kind of CD-ROM syscard is being used. This determines
  what syscard to be loaded from the syscards directory, and determines how
  much additional RAM is available:

  v1/v2: Uses syscard1.bin and syscard2.bin respectively, no extra RAM.
  v3: Uses syscard3.bin (for Super CD^2 support), 256KB extra RAM.
  acd: Also uses syscard3.bin, but has 256KB + 2048KB of extra RAM.
  gecd: Uses games_express.bin, has 256KB of extra RAM (it should probably be
   lower but I don't really know)
- Per-game BRAM saves - if this option is on then the current game gets its
  own .sav file BRAM in the bram directory. If it's off then bram.sav is
  used. Keep it off to save space or to transfer saves between games.


You can save a game specific config file here too. This will get loaded for
that game - if no game specific config file is found then a global config file
will be loaded. When you modify config settings you always modify the global
config file automatically.


-- Known Issues --

Some games have minor graphical glitches. PC-Engine video timing is really
finicky and that's probably what's causing them. Might be fixed in the future.

Games that have a resolution wider than 320 will be cut off, but centered.
Only a handful of games actually do this, most are 256 wide.

It seems that sometimes the emulator will hang when loading a new game that
has the CPU set to a clock speed that's different from what's currently
being used (causing the GP2X to need to be reset). It is not known why this
happens but hopefully it'll be worked around in the future. If anyone can
give me any further insight about this please contact me.


-- Contact --

If you think you can be polite and thorough enough e-mail me at
exophase@gmail.com. I probably won't answer things addressed in this readme,
so you should read through it first. Don't ask for ROMs, don't ask for
release dates, and basically just use common sense.

If you don't have my AIM/MSN/contacts etc then don't bother trying to get
them. Sorry, but I'm kind of fed up with people giving me a hard time over
IM.


-- Source code --

Temper is currently not open source, yeah, I know I know. No, it's not
using any GPLed or otherwise licensed source inside it. If you really want
the source send me an e-mail at exophase@gmail.com and I might consider
giving it to you if you have a pressing reason. Basically, this emulator
is representing me and my programming work, and I don't want any releases
of it to stray from that.

These are the basic guidelines for how to get the source:

- If you just want it to see what I'm doing and offer advice then I have
  no problem giving it to anyone. This means not modifying it and
  releasing your own versions.

- If you want to port it I might give it to you, depending on how much
  I want to port it for that platform. If I've already ported it to that
  platform then you can consider me the official maintainer, and if I
  decide to work on that platform later I become the official maintainer,
  meaning that unless I want you to be you stop being so.

- Any changes you make to the core in ports should be reviewed with me
  first so I make sure you don't do anything that I don't think is good.

- Can't spread the source around without my permission, for obvious
  reasons.

- Naturally it'd be pretty difficult to make any of this legally binding
  so you'll have to convince me that you're on the level and willing to
  play by my guidlines.

- If you can't speak English at all then don't bother.


-- Credits & Thanks --

FluBBa & Loopy - For PCEAdvance, got a few ideas from it.
Mednafen, Ootake, Hu-Go - Source code cleared up some things
sarencele (Mednafen author), Charles MacDonald - thanks for additional info
Lordus - ideas and support
Terryn - TONS and TONS of testing on PC version, this guy is amazing
notaz - some GP2X info
DaveC, zodttd, senquack, THB_, geise69, Duddyroar, jbrodack, sgstair -
 beta testing. Thanks a ton.


