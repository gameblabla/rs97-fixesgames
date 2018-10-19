PLATFORM=gcw0
DEBUG=0

ifeq ($(PLATFORM), gcw0)
  CC         := /opt/rs97-toolchain/bin/mipsel-linux-gcc
  STRIP      :=  /opt/rs97-toolchain/bin/mipsel-linux-strip
  SYSROOT    := $(shell $(CC) -print-sysroot)
  #CFLAGS     := --std=c89 --pedantic -Wall $(shell $(SYSROOT)/usr/bin/sdl-config --cflags) -DHOME_DIR -DNETWORKING
  CFLAGS     := --pedantic -Wall -I/opt/rs97-toolchain/mipsel-buildroot-linux-musl/sysroot/usr/include/SDL -DHOME_DIR -DNETWORKING
  LDFLAGS    := -lSDL -lm
endif


ifeq ($(DEBUG), 1)
  CC         = gcc
  STRIP      = strip
  SYSROOT    = $(shell $(CC) -print-sysroot)
  CFLAGS     = --pedantic -Wall $(shell $(SYSROOT)/usr/bin/sdl-config --cflags) -DHOME_DIR -DNETWORKING
  LDFLAGS    = $(shell $(SYSROOT)/usr/bin/sdl-config --libs) -lm
endif

ifeq ($(PLATFORM), macos)
  CFLAGS     := $(shell sdl-config --cflags)
endif
ifeq ($(PLATFORM), mingw)
  CC         := i686-w64-mingw32-gcc
  STRIP      := i686-w64-mingw32-strip
  SYSROOT    ?= /usr/i686-w64-mingw32
  CFLAGS     := --std=c89 --pedantic -Wall $(shell $(SYSROOT)/bin/sdl-config --cflags)
  LDFLAGS    := $(shell $(SYSROOT)/bin/sdl-config --libs) -lm
  TARGET     := shifty.exe
endif

SRCDIRS      = . backend

CC           ?= gcc
STRIP        ?= strip
TARGET       ?= shifty.elf
SYSROOT      ?= $(shell $(CC) -print-sysroot)
MACHINE      ?= $(shell $(CC) -dumpmachine)
DESTDIR      ?= $(SYSROOT)
CFLAGS       ?= --std=c89 --pedantic -Wall $(shell sdl-config --cflags) -DHOME_DIR -DNETWORKING
LDFLAGS      ?= $(shell sdl-config --libs) -lm
OUTDIR       ?= output/$(MACHINE)

ifdef DEBUG
#  CFLAGS += -DDEBUG -ggdb3 -Wall
  CFLAGS += -ggdb3 -Wextra
  OUTDIR := $(OUTDIR)-debug
  TARGET := $(TARGET)-debug
else
  CFLAGS += -O2
endif

BINDIR       := $(OUTDIR)/bin
SRCDIR       := src
OBJDIR       := $(OUTDIR)/obj
#IGNORED_FILES:= $(SRCDIR)/
SRC          := $(filter-out $(IGNORED_FILES),$(foreach dir,$(SRCDIRS),$(sort $(wildcard $(addprefix $(SRCDIR)/,$(dir))/*.c))))
OBJ          := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

.PHONY: all clean

all:	$(TARGET)

$(TARGET): $(OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $(BINDIR)/$@
ifndef DEBUG
	$(STRIP) $(BINDIR)/$@
endif

$(OBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $< -o $@ -I include

$(BINDIR) $(OBJDIR):
	mkdir -p $@

clean:
	rm -Rf $(OUTDIR)
