#
# Atari800 port on PSP
#
# Copyright (C) 2009 Ludovic Jacomme (ludovic.jacomme@gmail.com)
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
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

ATARI_VERSION	= 1.1.0
TARGET			= dingux-atari
ZERODEV_PATH	= 
BINARIES_PATH	= 

SDL_CONFIG = /opt/rs97-toolchain/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/bin/sdl-config

OBJS = 	gp2x_psp.o \
		antic.o atari.o atari_sdl.o binload.o \
		cartridge.o cassette.o colours.o compfile.o cpu.o cycle_map.o \
		devices.o gtia.o input.o log.o memory.o monitor.o  \
		pbi.o pia.o pokey.o pokeysnd.o remez.o rtime.o screen.o sio.o \
		statesav.o ui_basic.o ui.o util.o \
		psp_main.o psp_kbd.o psp_danzeff.o psp_sdl.o psp_font.o \
		psp_fmgr.o psp_menu.o  psp_menu_kbd.o psp_menu_set.o \
		psp_menu_help.o \
		miniunz.o ioapi.o unzip.o \
		psp_joy.o \
		psp_menu_cheat.o \
		psp_menu_list.o \
		psp_menu_joy.o \
		psp_editor.o



CC		= /opt/rs97-toolchain/bin/mipsel-linux-gcc
CXX		= /opt/rs97-toolchain/bin/mipsel-linux-g++
STRIP	= /opt/rs97-toolchain/bin/mipsel-linux-strip

DEFAULT_CFLAGS = $(shell $(SDL_CONFIG) --cflags)

MORE_CFLAGS = -I. \
-DNOCRYPT \
-DGCW0_MODE -DATARI_VERSION=\"$(ATARI_VERSION)\"  \
-mips32 -Ofast -mno-abicalls -mplt -D_GNU_SOURCE=1 -D_REENTRANT -DIS_LITTLE_ENDIAN \
-fsigned-char -ffast-math -fomit-frame-pointer \
-fexpensive-optimizations -fno-strength-reduce  \
-funroll-loops  -finline-functions \
-DNO_STDIO_REDIRECT

CFLAGS = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
LDFLAGS = -s

LIBS += \
-lSDL \
-lSDL_image \
-lpng -lz -lm -lpthread -lstdc++ -ldl -flto


.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@

all: $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJS) $(LIBS) -o $(TARGET) && $(STRIP) $(TARGET)

install: $(TARGET)
	cp $< /media/dingux/local/emulators/dingux-atari/

clean:
	rm -f $(OBJS) $(TARGET)

