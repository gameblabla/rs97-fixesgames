# Noiz2sa makefile(MinGW 2.0.0)
# $Id: Makefile,v 1.4 2003/08/10 09:00:05 kenta Exp $

NAME   = noiz2sa
O      = o
RM     = rm -f
CC     = /opt/rs97-toolchain/bin/mipsel-linux-gcc
CXX    = /opt/rs97-toolchain/bin/mipsel-linux-g++

PROG        = $(NAME).dge

DEFAULT_CFLAGS = `/opt/rs97-toolchain/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/bin/sdl-config --cflags` -I../bulletml/src/ 
LDFLAGS        = `/opt/rs97-toolchain/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/bin/sdl-config --libs` -L. -lSDL_mixer -lstdc++ libbulletml.a

MORE_CFLAGS = -O3

CFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
CPPFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -I./bulletml/ -I/usr/include -std=gnu++03

OBJS =	$(NAME).$(O) ship.$(O) shot.$(O) frag.$(O) bonus.$(O) \
	foe.$(O) foecommand.$(O) barragemanager.$(O) attractmanager.$(O) \
	background.$(O) letterrender.$(O) \
	screen.$(O) clrtbl.$(O) vector.$(O) degutil.$(O) rand.$(O) soundmanager.$(O)
#	$(NAME)_res.$(O)

$(PROG): $(OBJS) 
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)

#$(NAME)_res.o: $(NAME).rc
#	windres -F elf32-little -i $(NAME).rc -o $(NAME)_res.o
clean:
	$(RM) $(PROG) *.$(O)
