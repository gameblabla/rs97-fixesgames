# Temper makefile
# Gilead Kutnick - Exophase

# Global definitions

#PREFIX    = /opt/open2x/gcc-4.1.1-glibc-2.3.6
PREFIX    = /opt/arm-openwiz-linux-gnu
CC        = $(PREFIX)/bin/arm-openwiz-linux-gnu-gcc
STRIP     = $(PREFIX)/bin/arm-openwiz-linux-gnu-strip

OBJS      = main.o cpu.o memory.o irq.o io.o video.o timer.o psg.o cd.o       \
            adpcm.o arcade_card.o bin_cue.o disasm.o menu.o screen.o audio.o  \
            event.o debug.o cpu_arm_compatible.o cpu_arm_fast.o wiz.o         \
            wiz_mmuhack.o video_arm_asm.o asm_util.o
BIN       ?= temper.gpe 

# Platform specific definitions 

VPATH      += ..
VPATH      += ../arm
CFLAGS     += -Wall -DWIZ_BUILD -mcpu=arm926ej-s -mtune=arm926ej-s
CFLAGS     += -O3 -std=c99 -funsigned-char -msoft-float -fno-common           \
	      -fno-builtin

INCLUDES   = 
LIBS       =  -Bdynamic -lm -ldl -lpthread -Bstatic -lbz2 -ltremolo

# Compilation:

.SUFFIXES: .c .S

%.z: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c -o $@ $<

%.u: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c -o $@ $<

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c -o $@ $<


all:	$(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(BIN)  
#	$(STRIP) $(BIN)

clean:
	rm -f *.o *.u *.z $(BIN) 

