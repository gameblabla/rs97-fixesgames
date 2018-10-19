DEBUG	= 0

ifeq ($(DEBUG),0)
  CROSS_COMPILE ?= /opt/rs97-toolchain/bin/mipsel-linux-
endif

CC          := $(CROSS_COMPILE)gcc
STRIP       := $(CROSS_COMPILE)strip

SYSROOT     := $(shell $(CC) --print-sysroot)
SDL_CFLAGS  := -I/opt/rs97-toolchain/mipsel-buildroot-linux-musl/sysroot/usr/include/SDL
SDL_LIBS    := -lSDL

#CFLAGS  = -c -g -flto -Wall -D_REENTRANT -DPLATFORM_GCW $(SDL_CFLAGS)
CFLAGS  = -c -g -flto -Wall -D_REENTRANT $(SDL_CFLAGS)
LDFLAGS = -flto -lSDL_image -lSDL_gfx -ljpeg -lpng -lz -lSDL_mixer -lpthread -lgcc -lm -lc $(SDL_LIBS) $(MYFLAGS)
SOURCES = cmdline.c encoding.c err.c fileio.c help.c lxlogic.c mslogic.c play.c random.c res.c \
		score.c series.c solution.c tworld.c unslist.c \
		oshw-sdl/SFont.c oshw-sdl/ccicon.c oshw-sdl/sdlerr.c oshw-sdl/sdlin.c \
		oshw-sdl/sdloshw.c oshw-sdl/sdlout.c oshw-sdl/sdlsfx.c oshw-sdl/sdltext.c \
		oshw-sdl/sdltile.c oshw-sdl/sdltimer.c oshw-sdl/port_cfg.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLEPC=tworld
EXECUTABLEGP=tworld

all: 
	make gpversion

pcversion: $(SOURCES) $(EXECUTABLEPC)

gpversion: $(SOURCES) $(EXECUTABLEGP)

$(EXECUTABLEPC): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(EXECUTABLEGP): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

cleanobjs:
	rm -f $(OBJECTS)

clean:
	-rm -f tworld *~ *.o *.bak oshw-sdl/*.o oshw-sdl/*.bak oshw-sdl/*~

