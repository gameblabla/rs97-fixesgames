Compiling instructions for Dingux
OpenTitus 0.8.0

OpenTitus is released under the Gnu GPL version 3 or (at your option) any later version of the license, and it is released "as is"; without any warranty.


Install the toolchain, found at http://code.google.com/p/dingoo-linux/downloads/list

Compile time dependencies:
LibSDL development headers
LibSDLMixer library
Chocolate-Doom (1.6.0)'s OPL library (opl/libopl_dingux.a), check out opl/opl_readme.txt for further details

To compile, use the following commands:

All:
make -f Makefile_dingux


Clean:
make -f Makefile_dingux clean


The Makefile will define the compile-time constant "_DINGUX". It is used in /opentitus.c (videomode), in /src/common.c (sleep handling) and in /include/globals.h (keys).

Copy original titus/moktar files to the titus/moktar subfolders. Copy the executable, titus.conf (or titus_moktar.conf, rename it to titus.conf to play Moktar) and titus/moktar folders to the same folder on the card (such as /local/), and launch it via Applications/Explorer.

Keys:
B: Pick up/throw (Space)
Y: Toggle audio (F3)
X: Jump, helpful for kneestand+jump (Up)
A: Health display (E), additional press displays the status page (F4)
Start: Pause (P)
Select: Exit (Esc)
Right shoulder button: Die (F1)
