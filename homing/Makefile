PLATFORM=gcw0

ifeq ($(PLATFORM), gcw0)
	CC		:= /opt/rs97-toolchain/bin/mipsel-linux-gcc
	STRIP		:= /opt/rs97-toolchain/bin/mipsel-linux-strip
	SYSROOT		:= $(shell $(CC) --print-sysroot)
	CFLAGS		:=  -I/opt/rs97-toolchain/mipsel-buildroot-linux-musl/sysroot/usr/include/SDL
	CFLAGS		+= -DNO_FRAMELIMIT -DSCREEN_SCALE=1 -DHOME_DIR
	LDFLAGS		:= -lSDL -lm
	RELEASEDIR	:= release
endif

ifeq ($(PLATFORM), a320)
	CC		:= /opt/opendingux-toolchain/usr/bin/mipsel-linux-gcc
	STRIP		:= /opt/opendingux-toolchain/usr/bin/mipsel-linux-strip
	SYSROOT		:= $(shell $(CC) --print-sysroot)
	CFLAGS		:= $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
	CFLAGS		+= -DNO_FRAMELIMIT -DSCREEN_SCALE=1 -DHOME_DIR
	LDFLAGS		:= $(shell $(SYSROOT)/usr/bin/sdl-config --libs) -lm
	TARGET		:= fever.dge
endif

ifeq ($(PLATFORM), mingw32)
	CC		:= i486-mingw32-gcc
	STRIP		:= i486-mingw32-strip
	SYSROOT		:= $(shell $(CC) --print-sysroot)
	CFLAGS		:= -I/usr/i486-mingw32/include -I/usr/i486-mingw32/include/SDL
	LDFLAGS		:= -lmingw32 -lSDLmain -lSDL -lm -mwindows
	TARGET		:= fever.exe
endif

CC		?= gcc
STRIP		?= strip
CFLAGS		?= $(shell sdl-config --cflags) -DHOME_DIR
LDFLAGS		?= $(shell sdl-config --libs) -lm
TARGET		?= fever.elf
SRCDIR		:= src
OBJDIR		:= obj
SRC		:= $(wildcard $(SRCDIR)/*.c)
OBJ		:= $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

ifdef DEBUG
	CFLAGS	+= -Wall -Wextra -Werror -ggdb -pedantic -std=gnu89 -DDEBUG
else
	CFLAGS	+= -O2
endif

.PHONY: all opk clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
ifndef DEBUG
	$(STRIP) $@
endif

$(OBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJDIR):
	mkdir -p $@

opk: $(TARGET)
ifeq ($(PLATFORM), gcw0)
	mkdir -p		$(RELEASEDIR)
	cp $(TARGET)		$(RELEASEDIR)
	cp -R data		$(RELEASEDIR)
	cp platform/gcw0/*	$(RELEASEDIR)
	cp LICENSE.txt		$(RELEASEDIR)
	cp README.md		$(RELEASEDIR)
	mksquashfs		$(RELEASEDIR) homingFever.opk -all-root -noappend -no-exports -no-xattrs
endif

clean:
	rm -Rf $(TARGET) $(OBJDIR) $(RELEASEDIR)

