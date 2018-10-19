
# List of possible defines :
# BERMUDA_WIN32  : enable windows directory browsing code
# BERMUDA_POSIX  : enable unix/posix directory browsing code
# BERMUDA_VORBIS : enable playback of digital soundtracks (22 khz mono .ogg files)
# BERMUDA_BLUR   : enable blur rendering effect (use 'b' key to toggle effect on/off)
# OPENDINGUX     : build for OpenDingux, with resolution changed to 320x240 and bigger dialog text
# CYRILICTEXT	 : use cyrillic alphabet

#Original options, use this for building on your own system
DEFINES = -DBERMUDA_POSIX -DBERMUDA_VORBIS -DBERMUDA_BLUR -DOPENDINGUX -DCYRILICTEXT
SDL_CFLAGS = `/opt/rs97-toolchain/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/bin/sdl-config --cflags`
SDL_LIBS = `/opt/rs97-toolchain/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/bin/sdl-config --libs`
PREFIX = /opt/rs97-toolchain/bin/mipsel-linux-
#PREFIX = 
CXXFLAGS = -g -O -Wall $(SDL_CFLAGS) $(DEFINES)

#Use this lines to build for OpenDingux
#DEFINES = -DBERMUDA_POSIX -DBERMUDA_VORBIS -DOPENDINGUX
#SDL_CFLAGS = `/opt/opendingux-toolchain/usr/bin/sdl-config --cflags`
#SDL_LIBS = `/opt/opendingux-toolchain/usr/bin/sdl-config --libs`
#PREFIX = mipsel-linux-
#CXXFLAGS = -O2 -mips32 -mtune=mips32r2 -fomit-frame-pointer -fno-exceptions -ffunction-sections -ffast-math -msoft-float -g -Wall $(SDL_CFLAGS) $(DEFINES)

VORBIS_LIBS = -logg -lvorbisidec -logg

CXX = $(PREFIX)g++

-include Makefile.local

OBJDIR = obj

SRCS = avi_player.cpp bag.cpp decoder.cpp dialogue.cpp file.cpp fs.cpp game.cpp \
	main.cpp mixer.cpp opcodes.cpp parser_dlg.cpp parser_scn.cpp random.cpp \
	resource.cpp saveload.cpp staticres.cpp str.cpp systemstub_sdl.cpp util.cpp \
	win16.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

all: $(OBJDIR) bs

bs: $(addprefix $(OBJDIR)/, $(OBJS))
	$(CXX) $(LDFLAGS) -o $@ $^ $(SDL_LIBS) $(VORBIS_LIBS)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d

-include $(addprefix $(OBJDIR)/, $(DEPS))
