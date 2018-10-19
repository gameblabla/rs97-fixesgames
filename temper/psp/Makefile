# PCE emulator makefile
# Gilead Kutnick - Exophase

# Global definitions

PSPSDK    = $(shell psp-config --pspsdk-path)
PREFIX    = $(shell psp-config --psp-prefix)
PSPDEV    = $(shell psp-config --pspdev-path)

PSP_FW_VERSION  = 401 

OBJS            = main.o cpu.o memory.o irq.o io.o video.o timer.o psg.o       \
                  disasm.o screen.o event.o audio.o menu.o debug.o cd.o        \
                  adpcm.o arcade_card.o bin_cue.o

TARGET          = temper

# Platform specific definitions 

VPATH           += ..
CFLAGS          += -DPSP_BUILD -Os -G0 -funsigned-char
ASFLAGS         = $(CFLAGS)
PSP_EBOOT_TITLE = temper
EXTRA_TARGETS   = EBOOT.PBP

LIBS            += -lc -lm -lpspgu -lpsppower -lpsprtc -lpspaudio      

include $(PSPSDK)/lib/build.mak
