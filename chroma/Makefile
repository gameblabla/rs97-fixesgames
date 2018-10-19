CC = /opt/rs97-toolchain/bin/mipsel-linux-gcc
INSTALL = /usr/bin/install -c

prefix = retrogame
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
datadir = ${datarootdir}
datarootdir = ${prefix}/share

CFLAGS = -g -O2  -Winline -I/opt/rs97-toolchain/mipsel-buildroot-linux-musl/sysroot/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/opt/rs97-toolchain/mipsel-buildroot-linux-musl/sysroot/usr/include/freetype2  -DCHROMA_CURSES_HEADER=\<.h\> -DCHROMA_DATA_DIR=\"\"
INSTALL = /usr/bin/install -c
LDFLAGS =  -lSDL
LIBS = -lSDL_image -lfreetype

all:  chroma-sdl

OBJECTS_COMMON = main.o level.o engine.o menu.o colours.o util.o enigma.o xor.o editor.o xmlparser.o names.o
OBJECTS_CURSES = cursesdisplay.o cursesmenudisplay.o
OBJECTS_SDL = sdldisplay.o sdlshadowdisplay.o sdlmenudisplay.o sdlfont.o sdlscreen.o graphics.o

chroma-curses: $(OBJECTS_COMMON) $(OBJECTS_CURSES)
	$(CC) -o chroma-curses $(OBJECTS_COMMON) $(OBJECTS_CURSES) $(LDFLAGS) $(LIBS)

chroma-sdl: $(OBJECTS_COMMON) $(OBJECTS_SDL)
	$(CC) -o chroma $(OBJECTS_COMMON) $(OBJECTS_SDL) $(LDFLAGS) $(LIBS)

clean:  clean-sdl
	-rm $(OBJECTS_COMMON)

clean-curses:
	-rm $(OBJECTS_CURSES) chroma-curses

clean-sdl:
	-rm $(OBJECTS_SDL) chroma

install:  install-sdl
	for datafolder in colours help levels levels/* locale locale/* locale/*/*; do \
	if [ -d $$datafolder ]; then \
	${INSTALL} -d $(DESTDIR)$(datadir)/chroma/$$datafolder; \
	fi; \
	done
	for datafile in colours/* help/* levels/* levels/*/* locale/* locale/*/* locale/*/*/*; do \
	if [ -f $$datafile ]; then \
	${INSTALL} $$datafile $(DESTDIR)$(datadir)/chroma/$$datafile; \
	fi; \
	done

install-curses:
	${INSTALL} -d $(DESTDIR)$(bindir)
	${INSTALL} chroma-curses $(DESTDIR)$(bindir)/chroma-curses

install-sdl:
	${INSTALL} -d $(DESTDIR)$(bindir)
	${INSTALL} chroma $(DESTDIR)$(bindir)/chroma
	for datafile in graphics graphics/*; do \
        if [ -d $$datafile ]; then \
        ${INSTALL} -d $(DESTDIR)$(datadir)/chroma/$$datafile; \
        fi; \
	done
	for datafile in graphics/* graphics/*/*; do \
        if [ -f $$datafile ]; then \
        ${INSTALL} $$datafile $(DESTDIR)$(datadir)/chroma/$$datafile; \
        fi; \
        done
