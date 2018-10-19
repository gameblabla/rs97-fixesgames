# makefile for SDL

#NP21=1
VERSION=0.37
#PSPDBG=1
A320=1

ifdef A320
ARCH = a320
ARCH_PREF = /opt/rs97-toolchain/bin/mipsel-linux-
else
ARCH = host
ARCH_PREF =
endif


ifdef NP21
TARGET = NP21-$(ARCH)
else
TARGET = NP2-$(ARCH)
endif


CC = $(ARCH_PREF)gcc
STRIP = $(ARCH_PREF)strip
SDLCONF =$(SDL_PATH)sdl-config

RM		= rm -f
CP		= cp -p

OBJBASE		= ../obj
OBJ		= $(OBJBASE)/$(ARCH)

BIN		= ../bin

BASE		= ..
COMMON		= ../common
CODECNV		= ../codecnv
ifdef NP21
IX86C		= ../i386c
I386C		= ../i386c
else
IX86C		= ../i286c
I286C		= ../i286c
endif
MEM		= ../mem
IO		= ../io
CBUS		= ../cbus
BIOS		= ../bios
SOUND		= ../sound
VERMOUTH	= ../sound/vermouth
GETSND		= ../sound/getsnd
VRAM		= ../vram
FDD		= ../fdd
LIO		= ../lio
FONT		= ../font
GENERIC		= ../generic
EMBED		= ../embed
MENU		= ../embed/menu
MENUBASE	= ../embed/menubase
NP2TARGET	= .
NP2TARGET2	= ./psp

HEADERS	=	$(NP2TARGET2)/compiler.h $(BASE)/common.h $(COMMON)/milstr.h \
		$(COMMON)/_memory.h $(COMMON)/rect.h $(COMMON)/lstarray.h \
		$(NP2TARGET)/trace.h

CPUCORES=	$(IX86C)/cpucore.h $(IX86C)/memory.h

PCCORES=	$(BASE)/pccore.h $(BASE)/nevent.h $(BASE)/statsave.h

IOCORES=	$(IO)/iocore.h $(IO)/artic.h $(IO)/cgrom.h $(IO)/cpuio.h \
		$(IO)/crtc.h $(IO)/dipsw.h $(IO)/dmac.h $(IO)/egc.h \
		$(IO)/epsonio.h $(IO)/emsio.h $(IO)/fdc.h $(IO)/fdd320.h \
		$(IO)/gdc.h $(IO)/gdc_cmd.h $(IO)/gdc_sub.h $(IO)/mouseif.h \
		$(IO)/necio.h $(IO)/nmiio.h $(IO)/np2sysp.h $(IO)/pic.h \
		$(IO)/pit.h $(IO)/printif.h $(IO)/serial.h $(IO)/sysport.h \
		$(IO)/upd4990.h

FMBOARDS=	$(SOUND)/soundrom.h $(SOUND)/tms3631.h $(SOUND)/fmtimer.h \
		$(SOUND)/opngen.h $(SOUND)/psggen.h $(SOUND)/rhythm.h \
		$(SOUND)/adpcm.h $(SOUND)/pcm86.h $(SOUND)/cs4231.h

VERMOUTHS=	$(VERMOUTH)/midiout.h $(VERMOUTH)/midimod.h \
		$(VERMOUTH)/midinst.h $(VERMOUTH)/midvoice.h \
		$(VERMOUTH)/midtable.h

EMBEDS=		$(EMBED)/vramhdl.h $(EMBED)/vrammix.h \
		$(MENUBASE)/menudeco.inc $(MENUBASE)/menubase.h \
		$(MENUBASE)/menuvram.h $(MENUBASE)/menuicon.h \
		$(MENUBASE)/menusys.h $(MENUBASE)/menudlg.h \
		$(MENUBASE)/menumbox.h $(MENUBASE)/menures.h

ifdef NP21
CFLAGS=	 \
		-DCPUCORE_IA32 \
		-I$(BASE) -I$(COMMON) -I$(CODECNV) -I$(IX86C) -I$(MEM) \
		-I$(IO) -I$(CBUS) -I$(BIOS) -I$(LIO) -I$(VRAM) -I$(SOUND) \
		-I$(VERMOUTH) -I$(GETSND) -I$(FDD) -I$(FONT) -I$(GENERIC) \
		-I$(EMBED) -I$(MENU) -I$(MENUBASE) \
		-I$(NP2TARGET) -I$(NP2TARGET2) \
		-I$(IX86C)/ia32/ -I$(IX86C)/ia32/instructions \
		-I$(IX86C)/ia32/instructions/fpu
else
CFLAGS=	 \
		-I$(BASE) -I$(COMMON) -I$(CODECNV) -I$(IX86C) -I$(MEM) \
		-I$(IO) -I$(CBUS) -I$(BIOS) -I$(LIO) -I$(VRAM) -I$(SOUND) \
		-I$(VERMOUTH) -I$(GETSND) -I$(FDD) -I$(FONT) -I$(GENERIC) \
		-I$(EMBED) -I$(MENU) -I$(MENUBASE) \
		-I$(NP2TARGET) -I$(NP2TARGET2)
endif

CFLAGS+= `$(SDLCONF) --cflags`
CFLAGS+= -DSIZE_QVGA
CFLAGS+= -Ofast -mno-abicalls -mplt -mips32
# CFLAGS+= -g


# CFLAGS+= -G0 -Wall -O2 

ifdef PSPDBG
CFLAGS+=		-DPSPDBG
endif

INCDIR =
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS = `$(SDLCONF) --libs` -lm
#LIBS = -lpspgu -lpsppower -lpspaudiolib -lpspaudio -lm


ifdef NP21
IX86OBJ=	$(OBJ)/cpucore.o $(OBJ)/cpu.o $(OBJ)/cpu_io.o \
		$(OBJ)/cpu_mem.o $(OBJ)/ctrlxfer.o $(OBJ)/debug.o \
		$(OBJ)/disasm.o $(OBJ)/exception.o $(OBJ)/groups.o \
		$(OBJ)/ia32.o $(OBJ)/bin_arith.o $(OBJ)/bit_byte.o \
		$(OBJ)/ctrl_trans.o $(OBJ)/data_trans.o $(OBJ)/dec_arith.o \
		$(OBJ)/flag_ctrl.o $(OBJ)/fpdummy.o $(OBJ)/logic_arith.o \
		$(OBJ)/misc_inst.o $(OBJ)/seg_reg.o $(OBJ)/shift_rotate.o \
		$(OBJ)/string_inst.o $(OBJ)/system_inst.o $(OBJ)/inst_table.o \
		$(OBJ)/interface.o $(OBJ)/paging.o $(OBJ)/resolve.o \
		$(OBJ)/segments.o $(OBJ)/task.o $(OBJ)/memory.o \

else
IX86OBJ=	$(OBJ)/i286c.o $(OBJ)/i286c_mn.o $(OBJ)/i286c_ea.o \
		$(OBJ)/i286c_0f.o $(OBJ)/i286c_8x.o $(OBJ)/i286c_sf.o \
		$(OBJ)/i286c_f6.o $(OBJ)/i286c_fe.o $(OBJ)/i286c_rp.o \
		$(OBJ)/v30patch.o $(OBJ)/memory.o

endif

ifdef NP21
NP21OBJ=	$(OBJ)/memvga.o $(OBJ)/pcidev.o $(OBJ)/makegrex.o
else
NP21OBJ=
endif

OBJS=		$(OBJ)/strres.o $(OBJ)/milstr.o $(OBJ)/_memory.o \
		$(OBJ)/textfile.o $(OBJ)/profile.o $(OBJ)/rect.o \
		$(OBJ)/lstarray.o $(OBJ)/bmpdata.o $(OBJ)/parts.o \
		$(OBJ)/resize.o	\
		$(OBJ)/sjiseuc.o $(OBJ)/eucsjis.o $(OBJ)/eucucs2.o \
		$(IX86OBJ) $(NP21OBJ) \
		$(OBJ)/dmax86.o $(OBJ)/dmav30.o $(OBJ)/memtram.o \
		$(OBJ)/memvram.o $(OBJ)/memegc.o $(OBJ)/memems.o \
		$(OBJ)/memepp.o \
		$(OBJ)/iocore.o $(OBJ)/artic.o $(OBJ)/cgrom.o \
		$(OBJ)/cpuio.o $(OBJ)/crtc.o $(OBJ)/dipsw.o \
		$(OBJ)/dmac.o $(OBJ)/egc.o $(OBJ)/emsio.o \
		$(OBJ)/epsonio.o $(OBJ)/fdc.o $(OBJ)/fdd320.o \
		$(OBJ)/gdc.o $(OBJ)/gdc_sub.o $(OBJ)/gdc_pset.o \
		$(OBJ)/mouseif.o $(OBJ)/necio.o $(OBJ)/nmiio.o \
		$(OBJ)/np2sysp.o $(OBJ)/pic.o $(OBJ)/pit.o \
		$(OBJ)/printif.o $(OBJ)/serial.o $(OBJ)/sysport.o \
		$(OBJ)/upd4990.o \
		$(OBJ)/cbuscore.o $(OBJ)/sasiio.o $(OBJ)/scsiio.o \
		$(OBJ)/scsicmd.o $(OBJ)/pc9861k.o $(OBJ)/mpu98ii.o \
		$(OBJ)/board14.o $(OBJ)/board26k.o $(OBJ)/board86.o \
		$(OBJ)/boardx2.o $(OBJ)/board118.o $(OBJ)/boardspb.o \
		$(OBJ)/amd98.o $(OBJ)/pcm86io.o $(OBJ)/cs4231io.o \
		$(OBJ)/midiout.o $(OBJ)/midimod.o $(OBJ)/midinst.o \
		$(OBJ)/midvoice.o $(OBJ)/midtable.o \
		$(OBJ)/getsnd.o $(OBJ)/getwave.o $(OBJ)/getsmix.o \
		$(OBJ)/bios.o $(OBJ)/bios09.o $(OBJ)/bios0c.o \
		$(OBJ)/bios12.o $(OBJ)/bios13.o $(OBJ)/bios18.o \
		$(OBJ)/bios19.o $(OBJ)/bios1a.o $(OBJ)/bios1b.o \
		$(OBJ)/bios1c.o $(OBJ)/bios1f.o $(OBJ)/sxsibios.o \
		$(OBJ)/sound.o $(OBJ)/sndcsec.o $(OBJ)/fmboard.o \
		$(OBJ)/fmtimer.o $(OBJ)/beepc.o $(OBJ)/beepg.o \
		$(OBJ)/tms3631c.o $(OBJ)/tms3631g.o $(OBJ)/opngenc.o \
		$(OBJ)/opngeng.o $(OBJ)/psggenc.o $(OBJ)/psggeng.o \
		$(OBJ)/rhythmc.o $(OBJ)/adpcmc.o $(OBJ)/adpcmg.o \
		$(OBJ)/pcm86c.o $(OBJ)/pcm86g.o $(OBJ)/cs4231c.o \
		$(OBJ)/cs4231g.o $(OBJ)/soundrom.o $(OBJ)/s98.o \
		$(OBJ)/vram.o $(OBJ)/scrndraw.o $(OBJ)/sdraw.o \
		$(OBJ)/sdrawq16.o $(OBJ)/dispsync.o $(OBJ)/palettes.o \
		$(OBJ)/maketext.o $(OBJ)/maketgrp.o $(OBJ)/makegrph.o \
		$(OBJ)/scrnbmp.o \
		$(OBJ)/diskdrv.o $(OBJ)/newdisk.o $(OBJ)/fddfile.o \
		$(OBJ)/fdd_xdf.o $(OBJ)/fdd_d88.o $(OBJ)/fdd_mtr.o \
		$(OBJ)/sxsi.o $(OBJ)/sxsihdd.o $(OBJ)/sxsicd.o \
		$(OBJ)/lio.o $(OBJ)/gscreen.o $(OBJ)/gpset.o \
		$(OBJ)/gline.o $(OBJ)/gcircle.o $(OBJ)/gput1.o \
		$(OBJ)/font.o $(OBJ)/fontdata.o $(OBJ)/fontpc88.o \
		$(OBJ)/fontpc98.o $(OBJ)/fontv98.o $(OBJ)/fontfm7.o \
		$(OBJ)/fontx1.o $(OBJ)/fontx68k.o $(OBJ)/fontmake.o \
		$(OBJ)/cmndraw.o $(OBJ)/cmjasts.o $(OBJ)/cmver.o \
		$(OBJ)/softkbd.o $(OBJ)/vramhdl.o $(OBJ)/vrammix.o \
		$(OBJ)/menustr.o $(OBJ)/filesel.o $(OBJ)/dlgcfg.o \
		$(OBJ)/dlgscr.o $(OBJ)/dlgabout.o \
		$(OBJ)/menubase.o $(OBJ)/menuvram.o $(OBJ)/menuicon.o \
		$(OBJ)/menusys.o $(OBJ)/menudlg.o $(OBJ)/menumbox.o \
		$(OBJ)/menures.o \
		$(OBJ)/np2.o $(OBJ)/dosio.o $(OBJ)/trace.o \
		$(OBJ)/commng.o $(OBJ)/fontmng.o \
		$(OBJ)/joymng.o $(OBJ)/mousemng.o $(OBJ)/scrnmng.o \
		$(OBJ)/soundmng.o $(OBJ)/sysmng.o $(OBJ)/taskmng.o \
		$(OBJ)/timemng.o $(OBJ)/ini.o $(OBJ)/sysmenu.o \
		$(OBJ)/pccore.o $(OBJ)/nevent.o $(OBJ)/calendar.o \
		$(OBJ)/timing.o $(OBJ)/keystat.o $(OBJ)/statsave.o \
		$(OBJ)/debugsub.o $(OBJ)/psplib.o $(OBJ)/pg.o $(OBJ)/keyconf.o

OBJS += $(OBJ)/keypad.o

all : $(TARGET)


$(OBJ)/keypad.o : keypad.c
	$(CC) -c $(CFLAGS) -o $@ $<


include depends.psp


$(TARGET) : $(OBJ) $(OBJS)
	@echo "Linking .. $(TARGET)"
	@$(CC) -o $@ $(OBJS) $(LIBS)

strip :
	$(STRIP) $(TARGET)

$(OBJ) :
	mkdir -p $@

clean:
	$(RM) $(OBJS)
	$(RM) $(TARGET)

tags:
	find .. -name "*.[hc]" | etags -

clean_np2:
	$(RM) $(OBJS)
	$(RM) $(NP2BIN)
	$(RM) out outp Project.map TAGS


