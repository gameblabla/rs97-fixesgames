#PLATFORM?=native
PLATFORM?=dingux

# Compiler to use for each platform.
CC_native:=gcc
CC_dingux:=/opt/rs97-toolchain/bin/mipsel-linux-gcc

# Toolchain base directory for each platform.
TOOLCHAIN_native:=
TOOLCHAIN_dingux:=

# Executable file extension for each platform.
EXEEXT_native:=
EXEEXT_dingux:=.dge

# Tile size for each platform.
# Use size 24 for 320x240 and size 48 for 640x480.
RES_native:=48
RES_dingux:=24

# Pick the definitions for the active platform.
CC:=$(CC_$(PLATFORM))
TOOLCHAIN:=$(TOOLCHAIN_$(PLATFORM))
EXEEXT:=$(EXEEXT_$(PLATFORM))
RES:=$(RES_$(PLATFORM))

CFLAGS:=-O2 -g -Wall -Wextra -Wundef -Wunused-macros
LDFLAGS:=

SDL_CONFIG:=/opt/rs97-toolchain/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/bin/sdl-config
CFLAGS+=$(shell $(SDL_CONFIG) --cflags)
LDFLAGS+=$(shell $(SDL_CONFIG) --libs)
LDFLAGS+=-lSDL_mixer

CFLAGS+=-DTILE_SIZE=$(RES)

OUTDIR:=output/$(PLATFORM)

SOURCES:=$(wildcard src/*.c)
OBJS:=$(patsubst src/%.c,$(OUTDIR)/obj/%.o,$(SOURCES))

BINARY:=$(OUTDIR)/dist/drill$(EXEEXT)

.PHONY: all dist run clean

all: $(BINARY)

clean:
	rm -rf $(OUTDIR)

dist: $(BINARY)
	mkdir -p $(OUTDIR)/dist/susumi
	mkdir -p $(OUTDIR)/dist/system/bmp
	mkdir -p $(OUTDIR)/dist/system/wav
	cp dat/* $(OUTDIR)/dist
	cp gfx/icon.png $(OUTDIR)/dist
	cp gfx/blockbg.txt gfx/charabg.txt gfx/otherbg.txt $(OUTDIR)/dist
	cp gfx/player.txt $(OUTDIR)/dist/susumi
	cp gfx/*.bmp $(OUTDIR)/dist
	cp gfx/susumi-$(RES)/*.bmp $(OUTDIR)/dist/susumi
	cp gfx/system-$(RES)/*.bmp $(OUTDIR)/dist/system/bmp
	cp snd/*.txt $(OUTDIR)/dist
	cp snd/wav/*wav $(OUTDIR)/dist/system/wav

run: dist
	cd $(OUTDIR)/dist && ./drill$(EXEEXT)

$(OBJS): $(OUTDIR)/obj/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINARY): $(OBJS)
	mkdir -p $(@D)
	$(CC) -o $@ $^ $(LDFLAGS)
