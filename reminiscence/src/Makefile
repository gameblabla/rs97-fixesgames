DEBUG=0

DEFINES = -DBYPASS_PROTECTION
#DEFINES = -DBYPASS_PROTECTION -DENABLE_PASSWORD_MENU -DNDEBUG

ifeq ($(DEBUG),1)
  CC = g++
  CXX = g++
else
  CC = /opt/rs97-toolchain/bin/mipsel-linux-g++
  CXX = /opt/rs97-toolchain/bin/mipsel-linux-g++
endif

SYSROOT    := $(shell $(CC) --print-sysroot)
SDL_CFLAGS := -I/opt/rs97-toolchain//usr/mipsel-buildroot-linux-uclibc/sysroot/usr/include/SDL/
SDL_LIBS   := -lSDL

CXXFLAGS := -g -O -Wall -Wuninitialized -Wshadow -Wundef -Wreorder -Wnon-virtual-dtor -Wno-multichar
CXXFLAGS += -MMD $(SDL_CFLAGS) -DUSE_ZLIB $(DEFINES)

SRCS = collision.cpp cutscene.cpp file.cpp fs.cpp game.cpp graphics.cpp main.cpp menu.cpp \
	mixer.cpp mod_player.cpp piege.cpp resource.cpp scaler.cpp seq_player.cpp sfx_player.cpp \
	staticres.cpp systemstub_sdl.cpp unpack.cpp util.cpp video.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

LIBS = $(SDL_LIBS) -lz

-include Makefile.local

rs: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f *.o *.d

-include $(DEPS)
