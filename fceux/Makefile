
SRC = src/

CORE_OBJS = \
	$(SRC)cart.o $(SRC)cheat.o $(SRC)config.o $(SRC)movie.o $(SRC)oldmovie.o \
	$(SRC)drawing.o $(SRC)fceu.o $(SRC)fds.o $(SRC)file.o $(SRC)conddebug.o \
	$(SRC)filter.o $(SRC)ines.o $(SRC)input.o $(SRC)debug.o $(SRC)wave.o \
	$(SRC)nsf.o $(SRC)palette.o $(SRC)ppu.o $(SRC)sound.o $(SRC)state.o $(SRC)unif.o \
 	$(SRC)video.o $(SRC)vsuni.o $(SRC)x6502.o $(SRC)netplay.o $(SRC)emufile.o
    
BOARDS_OBJS = \
	$(SRC)boards/01-222.o \
	$(SRC)boards/3d-block.o \
	$(SRC)boards/09-034a.o \
	$(SRC)boards/12in1.o \
	$(SRC)boards/15.o \
	$(SRC)boards/158B.o \
	$(SRC)boards/18.o \
	$(SRC)boards/28.o \
	$(SRC)boards/32.o \
	$(SRC)boards/33.o \
	$(SRC)boards/34.o \
	$(SRC)boards/36.o \
	$(SRC)boards/40.o \
	$(SRC)boards/41.o \
	$(SRC)boards/42.o \
	$(SRC)boards/43.o \
	$(SRC)boards/46.o \
	$(SRC)boards/50.o \
	$(SRC)boards/51.o \
	$(SRC)boards/57.o \
	$(SRC)boards/62.o \
	$(SRC)boards/65.o \
	$(SRC)boards/67.o \
	$(SRC)boards/68.o \
	$(SRC)boards/69.o \
	$(SRC)boards/71.o \
	$(SRC)boards/72.o \
	$(SRC)boards/77.o \
	$(SRC)boards/79.o \
	$(SRC)boards/80.o \
	$(SRC)boards/82.o \
	$(SRC)boards/88.o \
	$(SRC)boards/90.o \
	$(SRC)boards/91.o \
	$(SRC)boards/96.o \
	$(SRC)boards/99.o \
	$(SRC)boards/103.o \
	$(SRC)boards/106.o \
	$(SRC)boards/108.o \
	$(SRC)boards/112.o \
	$(SRC)boards/116.o \
	$(SRC)boards/117.o \
	$(SRC)boards/120.o \
	$(SRC)boards/121.o \
	$(SRC)boards/151.o \
	$(SRC)boards/156.o \
	$(SRC)boards/164.o \
	$(SRC)boards/168.o \
	$(SRC)boards/170.o \
	$(SRC)boards/175.o \
	$(SRC)boards/176.o \
	$(SRC)boards/177.o \
	$(SRC)boards/178.o \
	$(SRC)boards/183.o \
	$(SRC)boards/185.o \
	$(SRC)boards/186.o \
	$(SRC)boards/187.o \
	$(SRC)boards/189.o \
	$(SRC)boards/193.o \
	$(SRC)boards/199.o \
	$(SRC)boards/206.o \
	$(SRC)boards/208.o \
	$(SRC)boards/222.o \
	$(SRC)boards/225.o \
	$(SRC)boards/228.o \
	$(SRC)boards/230.o \
	$(SRC)boards/232.o \
	$(SRC)boards/234.o \
	$(SRC)boards/235.o \
	$(SRC)boards/244.o \
	$(SRC)boards/246.o \
	$(SRC)boards/252.o \
	$(SRC)boards/253.o \
	$(SRC)boards/411120-c.o \
	$(SRC)boards/603-5052.o \
	$(SRC)boards/8157.o \
	$(SRC)boards/8237.o \
	$(SRC)boards/830118C.o \
	$(SRC)boards/8in1.o \
	$(SRC)boards/__dummy_mapper.o \
	$(SRC)boards/a9746.o \
	$(SRC)boards/ac-08.o \
	$(SRC)boards/addrlatch.o \
	$(SRC)boards/ax5705.o \
	$(SRC)boards/bandai.o \
	$(SRC)boards/bb.o \
	$(SRC)boards/bmc13in1jy110.o \
	$(SRC)boards/bmc42in1r.o \
	$(SRC)boards/bmc64in1nr.o \
	$(SRC)boards/bmc70in1.o \
	$(SRC)boards/BMW8544.o \
	$(SRC)boards/bonza.o \
	$(SRC)boards/bs-5.o \
	$(SRC)boards/cityfighter.o \
	$(SRC)boards/coolboy.o \
	$(SRC)boards/dance2000.o \
	$(SRC)boards/datalatch.o \
	$(SRC)boards/dream.o \
	$(SRC)boards/edu2000.o \
	$(SRC)boards/eh8813a.o \
	$(SRC)boards/emu2413.o \
	$(SRC)boards/et-100.o \
	$(SRC)boards/et-4320.o \
	$(SRC)boards/F-15.o \
	$(SRC)boards/famicombox.o \
	$(SRC)boards/ffe.o \
	$(SRC)boards/fk23c.o \
	$(SRC)boards/ghostbusters63in1.o \
	$(SRC)boards/gs-2004.o \
	$(SRC)boards/gs-2013.o \
	$(SRC)boards/h2288.o \
	$(SRC)boards/hp898f.o \
	$(SRC)boards/inlnsf.o \
	$(SRC)boards/karaoke.o \
	$(SRC)boards/kof97.o \
	$(SRC)boards/ks7010.o \
	$(SRC)boards/ks7012.o \
	$(SRC)boards/ks7013.o \
	$(SRC)boards/ks7016.o \
	$(SRC)boards/ks7017.o \
	$(SRC)boards/ks7030.o \
	$(SRC)boards/ks7031.o \
	$(SRC)boards/ks7032.o \
	$(SRC)boards/ks7037.o \
	$(SRC)boards/ks7057.o \
	$(SRC)boards/le05.o \
	$(SRC)boards/lh32.o \
	$(SRC)boards/lh53.o \
	$(SRC)boards/malee.o \
	$(SRC)boards/mihunche.o \
	$(SRC)boards/mmc1.o \
	$(SRC)boards/mmc2and4.o \
	$(SRC)boards/mmc3.o \
	$(SRC)boards/mmc5.o \
	$(SRC)boards/n106.o \
	$(SRC)boards/n625092.o \
	$(SRC)boards/novel.o \
	$(SRC)boards/onebus.o \
	$(SRC)boards/pec-586.o \
	$(SRC)boards/rt-01.o \
	$(SRC)boards/sa-9602b.o \
	$(SRC)boards/sachen.o \
	$(SRC)boards/sb-2000.o \
	$(SRC)boards/sc-127.o \
	$(SRC)boards/sheroes.o \
	$(SRC)boards/sl1632.o \
	$(SRC)boards/subor.o \
	$(SRC)boards/super24.o \
	$(SRC)boards/supervision.o \
	$(SRC)boards/t-227-1.o \
	$(SRC)boards/t-262.o \
	$(SRC)boards/tengen.o \
	$(SRC)boards/tf-1201.o \
	$(SRC)boards/transformer.o \
	$(SRC)boards/unrom512.o \
	$(SRC)boards/vrc1.o \
	$(SRC)boards/vrc2and4.o \
	$(SRC)boards/vrc3.o \
	$(SRC)boards/vrc5.o \
	$(SRC)boards/vrc6.o \
	$(SRC)boards/vrc7.o \
	$(SRC)boards/vrc7p.o \
	$(SRC)boards/yoko.o

INPUT_OBJS = $(SRC)input/arkanoid.o $(SRC)input/bworld.o $(SRC)input/cursor.o \
	$(SRC)input/fkb.o $(SRC)input/ftrainer.o $(SRC)input/hypershot.o $(SRC)input/mahjong.o \
	$(SRC)input/mouse.o $(SRC)input/oekakids.o $(SRC)input/pec586kb.o \
	$(SRC)input/powerpad.o $(SRC)input/quiz.o $(SRC)input/shadow.o $(SRC)input/snesmouse.o \
	$(SRC)input/suborkb.o $(SRC)input/toprider.o $(SRC)input/zapper.o

MAPPERS_OBJS = 

UTILS_OBJS = $(SRC)utils/crc32.o $(SRC)utils/endian.o $(SRC)utils/general.o \
	$(SRC)utils/guid.o $(SRC)utils/md5.o $(SRC)utils/memory.o $(SRC)utils/unzip.o \
	$(SRC)utils/xstring.o $(SRC)utils/ioapi.o $(SRC)utils/ConvertUTF.o

COMMON_DRIVER_OBJS = $(SRC)drivers/common/args.o $(SRC)drivers/common/cheat.o \
	$(SRC)drivers/common/config.o $(SRC)drivers/common/configSys.o  $(SRC)drivers/common/nes_ntsc.o

GUI_OBJS = \
	$(SRC)drivers/dingux-sdl/gui/gui.o \
	$(SRC)drivers/dingux-sdl/gui/file_list.o \
	$(SRC)drivers/dingux-sdl/gui/font.o

DRIVER_OBJS = $(SRC)drivers/dingux-sdl/config.o $(SRC)drivers/dingux-sdl/input.o \
	$(SRC)drivers/dingux-sdl/dingoo.o $(SRC)drivers/dingux-sdl/dingoo-joystick.o \
	$(SRC)drivers/dingux-sdl/dingoo-throttle.o $(SRC)drivers/dingux-sdl/dingoo-sound.o \
	$(SRC)drivers/dingux-sdl/dingoo-video.o $(SRC)drivers/dingux-sdl/dummy-netplay.o \
	$(SRC)drivers/dingux-sdl/scaler.o $(MINIMAL_OBJS) $(GUI_OBJS)

OBJS = $(CORE_OBJS) $(BOARDS_OBJS) $(INPUT_OBJS) $(MAPPERS_OBJS) $(UTILS_OBJS) \
	$(COMMON_DRIVER_OBJS) $(DRIVER_OBJS)

TOOLCHAIN=
BINDIR=
CC = /opt/rs97-toolchain/bin/mipsel-linux-gcc
CXX = /opt/rs97-toolchain/bin/mipsel-linux-g++
LD = /opt/rs97-toolchain/bin/mipsel-linux-g++
AS = /opt/rs97-toolchain/bin/mipsel-linux-as

INCLUDEDIR=$(TOOLCHAIN)/include
CFLAGS = -I$(INCLUDEDIR) -I$(SRC)
CXXFLAGS = -I$(INCLUDEDIR)

LDFLAGS = -s

W_OPTS	= -Wno-write-strings -Wno-sign-compare

F_OPTS = -fomit-frame-pointer -fno-builtin -fno-common

CC_OPTS	= -O2 -ffast-math -msym32 -mips32 -mno-shared -mno-abicalls -mlong-calls -mno-mips16 -fdata-sections -ffunction-sections $(F_OPTS) $(W_OPTS) -I/home/steward/Downloads/buildroot-2017.02.9/output/host/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/include/SDL

CFLAGS += $(CC_OPTS)
CFLAGS += -DDINGUX \
	  -DLSB_FIRST \
	  -DPSS_STYLE=1 \
	  -DHAVE_ASPRINTF \
	  -DFRAMESKIP \
	  -D_REENTRANT \
	  -I$(INCLUDEDIR)/SDL -D_GNU_SOURCE=1 -D_REENTRANT
CXXFLAGS += $(CFLAGS) -fno-rtti
LDFLAGS  += $(CC_OPTS)
ifdef STATIC
LDFLAGS  += -static-libgcc -static-libstdc++
endif
LIBS = -L$(LIBDIR) `sdl-config --libs` -lz -lm -s -Wl,--as-needed -Wl,--gc-sections -flto -s

TARGET = fceux

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p bin/
	@cp src/drivers/dingux-sdl/gui/*.bmp bin/
	@echo Linking $@...
	@echo $(LD) $(LDFLAGS) $(OBJS) -o bin/$@
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o bin/$@

%.o: %.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

%.o: %.o \
	@echo Compiling $<...
	$(CXX) $(CDEFS) $(CXXFLAGS) -c $< -o $@

%.o: %.s
	@echo Assembling $<...
	$(CXX) $(CDEFS) $(CXXFLAGS) -c $< -o $@

%.o: %.S
	@echo Assembling $<...
	$(CXX) $(CDEFS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) bin/$(TARGET)
