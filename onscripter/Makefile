DEBUG=0

ifeq ($(DEBUG),0)
  LIBS = -lSDL_ttf -lSDL_image -lSDL_mixer -lSDL -lfreetype -ljpeg -lpng -lz -lbz2 -lvorbisidec
else
  LIBS = -lSDL_ttf -lSDL_image -lSDL_mixer -lSDL -lfreetype -ljpeg -lpng -lz -lbz2 -lvorbisidec
endif

DEFS = -DGP2X -DLINUX -DPDA_WIDTH=320 -DBPP16 -DUSE_OGG_VORBIS -DINTEGER_OGG_VORBIS -DRETROGAME
EXESUFFIX = 
OBJSUFFIX = .o

.SUFFIXES:
.SUFFIXES: $(OBJSUFFIX) .cpp .h

ifeq ($(DEBUG),1)
  CC = g++ 
  LD = g++ -o
else
  CC = /opt/rs97-toolchain/bin/mipsel-linux-g++ 
  LD =  /opt/rs97-toolchain/bin/mipsel-linux-g++ -o
endif

CFLAGS = -O2 -mno-abicalls -mips32 -mplt -s -Wall -fno-exceptions -fno-rtti -fno-check-new -fomit-frame-pointer -pipe -c -I /opt/rs97-toolchain/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/include/SDL/ $(DEFS)
RM = rm -f

TARGET = onscripter$(EXESUFFIX) nsaconv$(EXESUFFIX) sarconv$(EXESUFFIX) nsadec$(EXESUFFIX) sardec$(EXESUFFIX)
EXT_OBJS = 

include Makefile.onscripter
