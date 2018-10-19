#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <unistd.h>

#ifdef USE_LZMA
#include "lzma/lzma.h"
#else
#include <zlib.h>
#endif

#ifdef DREAMCAST
#include <kos.h>
void reinit_sdcard(void);
#endif

#include "config.h"
#include "st.h"
#include "st/mem.h"

#include "m68k/m68k_intrf.h"

#include "savedisk.h"

#if defined(AUTOSAVESTATE) && defined(DREAMCAST)
#undef SAVE_PREFIX
#define SAVE_PREFIX ""
#endif

#define MAX_SAVESTATES 4
#define FILESAVE_NAME  "save_"
#define FILESAVE_EXTENSION ".sst"

#ifdef DREAMCAST
#define MAX_COMP_SIZE (1024*128)
#else
#define MAX_COMP_SIZE (1024*1024)
#endif

#ifndef NO_MENU
extern int mainMenu_savedisk;
#else
int mainMenu_savedisk=-1;
#endif

extern Uint8 compat;
extern int speedbar;
extern int checkedsample;
extern int checkedsound;
extern int nbframeskip;
extern int autoframeskip;
extern int maxframeskip;
extern int autofire;
extern char *dcastaway_image_file;

extern int8    *membase;
extern int     display_mode;
//68000 vars
extern unsigned long   reg[23];
extern unsigned long   dfc, sfc, vbr;
extern unsigned short SaveWordTrc;
extern unsigned char GetTrc;
extern int      intmask, intpri;
extern char     cpu_state;
extern unsigned cpu_type;
extern volatile unsigned cpu_count;

extern Uint8    memconf;
extern Uint8    mfp_gpip, mfp_aer, mfp_ddr, mfp_iera, mfp_ierb, mfp_ipra, mfp_iprb,
mfp_isra, mfp_isrb, mfp_imra, mfp_imrb, mfp_ivr, mfp_tacr,
mfp_tbcr, mfp_tcdcr, mfp_scr, mfp_ucr, mfp_rsr, mfp_tsr, mfp_udr;

//Mfp delay timer variables
extern int32 mfp_reg[12];

extern Uint8    acia1_cr, acia1_sr, acia1_dr, acia2_cr, acia2_sr,
acia2_dr;

//Video shifter
extern Uint8    vid_baseh, vid_basem;
extern Uint8    vid_syncmode, vid_shiftmode;
extern Sint16    vid_col[16];
extern int      vid_flag;


extern Uint16   dma_car, dma_scr, dma_sr, dma_mode;
extern Uint8    dma_adrh, dma_adrm, dma_adrl;

extern unsigned char sample[10000];

/* fdc.c */
extern unsigned char fdc_data, fdc_track, fdc_sector,
fdc_status, fdc_command, fdc_int;
extern char     fdcdir;
extern unsigned char	disk_ejected[2];
extern unsigned char	disk_changed[2];
extern int discpos[2];
extern struct Disk disk[2];


extern Uint16   blt_halftone[16];
extern Sint16   blt_src_x_inc, blt_src_y_inc;
extern Sint16   blt_end_1, blt_end_2, blt_end_3;
extern Sint16   blt_dst_x_inc, blt_dst_y_inc;
extern Uint16   blt_x_cnt, blt_y_cnt;
extern int8     blt_hop, blt_op, blt_status, blt_skew;



extern int ikbd_pulling;
extern int ikbd_direct;
extern unsigned char   inbuff[];
extern unsigned char   outbuff[];
extern int             inbuffi;
extern int             outbuffi;

extern char msabuff[MAX_DISC_SIZE];
extern unsigned int firsttick,lasttick;

#ifdef DEBUG_SAVESTATE
#include <unistd.h>
static char ___getcwd[1024];
#define currentdir() getcwd((char *)&___getcwd[0],1023)
#endif

static void eliminate_file(char *filename)
{
#ifdef DEBUG_SAVESTATE
	printf("save: eliminate_file(%s) [%s]\n",filename,currentdir());fflush(stdout);
#endif
	FILE *f=fopen(filename,"r");
	if (f)
	{
		fclose(f);
		unlink(filename);
	}
}

#if defined(DREAMCAST) && !defined(AUTOSAVESTATE)
#include"save_icon.h"
#define VMUFILE_PAD 128+512
static unsigned char    *paquete=NULL;
static int              paquete_size=0;

static void prepare_save(void)
{
	if (paquete)
		return;
	char *str="DCaSTaway";
	vmu_pkg_t pkg;
	memset(&pkg, 0, sizeof(pkg));
	strcpy(pkg.desc_short, str);
	strcpy(pkg.desc_long, str);
	strcpy(pkg.app_id, str);
	pkg.icon_cnt = 1;
	pkg.icon_anim_speed = 0;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	pkg.eyecatch_data = NULL;
	pkg.data_len = 4;
	pkg.data = (const uint8*)&pkg;
	memcpy((void *)&pkg.icon_pal[0],(void *)&vmu_savestate_icon_pal,32);
	pkg.icon_data = (const uint8*)&vmu_savestate_icon_data;
	vmu_pkg_build(&pkg, &paquete, &paquete_size);
	paquete_size-=4;
}

static void rebuild_paquete(char *name, unsigned size, unsigned char* data, FILE *f)
{
	unsigned short *crc=(unsigned short*) &paquete[0x46];
	unsigned *data_len=(unsigned *) &paquete[0x48];
	char *desc_long=(char *) &paquete[16];
	bzero(desc_long,32);
	strncpy(desc_long,name,31);
	*data_len=size;
	int i, c, n = 0;
	(*crc)=0;
	for (i = 0; i < paquete_size; i++)
	{
		if (i<VMUFILE_PAD)
			n ^= (paquete[i]<<8);
		else
			n ^= (data[i-VMUFILE_PAD]<<8);
		for (c = 0; c < 8; c++)
			if (n & 0x8000)
				n = (n << 1) ^ 4129;
			else
				n = (n << 1);
	}
	(*crc)=(n & 0xffff);
	fwrite((void *)&paquete[0],1,VMUFILE_PAD,f);
}

static void set_vmu_pad(FILE *f)
{
	fseek(f,VMUFILE_PAD,SEEK_SET);
}


#else
#define VMUFILE_PAD 0
#define prepare_save()
#define rebuild_paquete(A,B,C,D)
#define set_vmu_pad(A)
#endif

static unsigned getFreeBlocks(int is_sd)
{

#if defined(DREAMCAST) && !defined(AUTOSAVESTATE)
	Uint16 buf16[255];
	int free_blocks=0,i=0;
	maple_device_t *dev;
	Uint8 addr=0;
	int p,u;

	if (!is_sd) {
#ifdef DEBUG_SAVESTATE
	puts("save: getFreeBlocks");fflush(stdout);
#endif
	uint8 v=maple_first_vmu();
	if (!v)
		return 0;
	if (maple_compat_resolve(v,&dev,MAPLE_FUNC_MEMCARD)!=0)
		return 0;
#ifdef DEBUG_SAVESTATE
	puts("save: getFreeBlocks - mapped");fflush(stdout);
#endif
	free_blocks=vmufs_free_blocks(dev);
#ifdef DEBUG_SAVESTATE
	printf("save: getFreeBlocks - return %i\n",free_blocks);fflush(stdout);
#endif
	return free_blocks;
	}
#endif
	return 0x10000;
}

int loadstate(int numstate){
	Uint8 tosnum1,tosnum2;
	Uint16 tosnum=0;
	char strbuf[10];
	FILE *loadfile,*loadtos;
	char filename[256],tempname[256];
	struct Disk stdisk;
	int i;
	unsigned long  sizecompressed=0;
	unsigned long  sizeuncompressed=0;

	memset(filename,0,256);
#ifdef DREAMCAST
	sprintf ( filename, SAVE_MEM_PREFIX FILESAVE_NAME "%d" FILESAVE_EXTENSION, numstate );
#else
	sprintf ( filename, "%s%d" FILESAVE_EXTENSION, dcastaway_image_file, numstate );
#endif

#ifdef DEBUG_SAVESTATE
	printf ( "save: loadstate(%i) %s [%s]\n", numstate, filename, currentdir() );fflush(stdout);
#endif

	if (NULL == (loadfile = fopen(filename,"rb"))){
#ifdef DREAMCAST
		extern int sdcard_exists;
		reinit_sdcard();
		if  (sdcard_exists) {
			FILE *fsd;
			long len;
			memset(tempname,0,256);
			sprintf ( tempname, "%s%d" FILESAVE_EXTENSION, dcastaway_image_file, numstate );
			fsd=fopen(tempname,"rb");
			if (!fsd) {
				memset(tempname,0,256);
				sprintf ( tempname, "/sd/dcastaway/%s%d" FILESAVE_EXTENSION, dcastaway_image_file, numstate );
				fsd=fopen(tempname,"rb");
				if (!fsd)
					return -1;
			}
			fseek(fsd,0,SEEK_END);
			len=ftell(fsd);
			if (len<1) {
				fclose(fsd);
				return 1;
			}
			fseek(fsd,0,SEEK_SET);
			loadfile=fopen(filename,"wb");
			if (!loadfile) {
				fclose(fsd);
				return -1;
			}
			for(;len>0;len-=256)
				fwrite((void *)tempname,1,fread((void *)tempname,1,256,fsd),loadfile);
			fclose(fsd);
			fclose(loadfile);
			loadfile=fopen(filename,"rb");
			if (!loadfile)
				return -1;
		}
		else
			return -1;
#else
#ifdef DEBUG_SAVESTATE
		puts( "save: loadstate - failed!");fflush(stdout);
#endif
		return -1;
#endif
	}
	

#ifndef USE_FAME_CORE
	//Save 68000 vars
	fread(&reg[0],1,sizeof(unsigned long)*23,loadfile);
	fread(&dfc,1,sizeof(dfc),loadfile);
	fread(&sfc,1,sizeof(sfc),loadfile);
	fread(&vbr,1,sizeof(vbr),loadfile);
	fread(&SaveWordTrc,1,sizeof(SaveWordTrc),loadfile);
	fread(&intmask,1,sizeof(intmask),loadfile);
	fread(&intpri,1,sizeof(intpri),loadfile);
	fread(&cpu_state,1,sizeof(cpu_state),loadfile);
	fread(&cpu_type,1,sizeof(cpu_type),loadfile);
#else
	{
	 unsigned cvnz,stx,biginst,usp,ssp,sfc,vbr,im,cst,cty,sr,sr2;
	 int recalc_int=1;
	 fread(&M68KCONTEXT.dreg[0],1,sizeof(unsigned long)*8,loadfile);
	 fread(&M68KCONTEXT.areg[0],1,sizeof(unsigned long)*8,loadfile);
	 fread(&cvnz,1,sizeof(unsigned long),loadfile);
	 fread(&stx,1,sizeof(unsigned long),loadfile);
	 fread(&biginst,1,sizeof(unsigned long),loadfile);
	 fread(&M68KCONTEXT.pc,1,sizeof(unsigned long),loadfile);
	 fread(&usp,1,sizeof(unsigned long),loadfile);
	 fread(&ssp,1,sizeof(unsigned long),loadfile);
	 fread(&recalc_int,1,sizeof(unsigned long),loadfile);
	 fread(&M68KCONTEXT.interrupts[4],1,sizeof(unsigned long),loadfile);
	 fread(&sfc,1,sizeof(unsigned long),loadfile);
	 fread(&vbr,1,sizeof(unsigned long),loadfile);
	 fread(&M68KCONTEXT.execinfo,1,sizeof(unsigned short),loadfile);
	 fread(&im,1,sizeof(int),loadfile);
	 fread(&M68KCONTEXT.interrupts[0],1,sizeof(int),loadfile);
	 fread(&cst,1,sizeof(char),loadfile);
	 fread(&cty,1,sizeof(unsigned),loadfile);
	 sr=(cvnz>>24)+(cvnz>>15)+(cvnz<<2)+(cvnz>>5)+(stx<<4);
	 sr2=(stx<<7)+(stx>>3)+(im<<8);
	 M68KCONTEXT.sr=(sr&(unsigned short)0xff)+(sr2&(unsigned short)0xff00);
	 if (GetS())
		 M68KCONTEXT.asp=usp;
	 else
		 M68KCONTEXT.asp=ssp;
	}
#endif
	
	//Save FDC vars
	fread(&fdc_data,1,sizeof(fdc_data),loadfile);
	fread(&fdc_track,1,sizeof(fdc_track),loadfile);
	fread(&fdc_sector,1,sizeof(fdc_sector),loadfile);
	fread(&fdc_status,1,sizeof(fdc_status),loadfile);
	fread(&fdc_command,1,sizeof(fdc_command),loadfile);
	fread(&fdc_int,1,sizeof(fdc_int),loadfile);
	fread(&fdcdir,1,sizeof(fdcdir),loadfile);	
	fread(&disk_ejected[0],1,sizeof(unsigned char)*2,loadfile);
	fread(&disk_changed[0],1,sizeof(unsigned char)*2,loadfile);
	fread(&discpos[0],1,sizeof(int)*2,loadfile);
	//IO vars
	fread(&memconf,1,sizeof(memconf),loadfile);
	fread(&mfp_gpip,1,sizeof(mfp_gpip),loadfile);
	fread(&mfp_aer,1,sizeof(mfp_aer),loadfile);
	fread(&mfp_ddr,1,sizeof(mfp_ddr),loadfile);
	fread(&mfp_iera,1,sizeof(mfp_iera),loadfile);
	fread(&mfp_ierb,1,sizeof(mfp_ierb),loadfile);
	fread(&mfp_ipra,1,sizeof(mfp_ipra),loadfile);
	fread(&mfp_iprb,1,sizeof(mfp_iprb),loadfile);
	fread(&mfp_isra,1,sizeof(mfp_isra),loadfile);
	fread(&mfp_isrb,1,sizeof(mfp_isrb),loadfile);
	fread(&mfp_imra,1,sizeof(mfp_imra),loadfile);
	fread(&mfp_imrb,1,sizeof(mfp_imrb),loadfile);
	fread(&mfp_ivr,1,sizeof(mfp_ivr),loadfile);
	fread(&mfp_tacr,1,sizeof(mfp_tacr),loadfile);
	fread(&mfp_tbcr,1,sizeof(mfp_tbcr),loadfile);
	fread(&mfp_tcdcr,1,sizeof(mfp_tcdcr),loadfile);
	fread(&mfp_scr,1,sizeof(mfp_scr),loadfile);
	fread(&mfp_ucr,1,sizeof(mfp_ucr),loadfile);
	fread(&mfp_rsr,1,sizeof(mfp_rsr),loadfile);
	fread(&mfp_tsr,1,sizeof(mfp_tsr),loadfile);
	fread(&mfp_udr,1,sizeof(mfp_udr),loadfile);
	fread(&mfp_reg[0],1,sizeof(int32)*32,loadfile);
	
	//ACIA vars
	fread(&acia1_cr,1,sizeof(acia1_cr),loadfile);
	fread(&acia1_sr,1,sizeof(acia1_sr),loadfile);
	fread(&acia1_dr,1,sizeof(acia1_dr),loadfile);
	fread(&acia2_cr,1,sizeof(acia2_cr),loadfile);
	fread(&acia2_sr,1,sizeof(acia2_sr),loadfile);
	fread(&acia2_dr,1,sizeof(acia2_dr),loadfile);
	
	//Video vars
	fread(&vid_adr,1,sizeof(vid_adr),loadfile);
	fread(&vid_baseh,1,sizeof(vid_baseh),loadfile);
	fread(&vid_basem,1,sizeof(vid_basem),loadfile);
	fread(&vid_syncmode,1,sizeof(vid_syncmode),loadfile);
	fread(&vid_shiftmode,1,sizeof(vid_shiftmode),loadfile);
	fread(&vid_col[0],1,sizeof(int16)*16,loadfile);
	fread(&vid_flag,1,sizeof(vid_flag),loadfile);
	vid_flag=1;
	//DMA vars
	fread(&dma_car,1,sizeof(dma_car),loadfile);
	fread(&dma_scr,1,sizeof(dma_scr),loadfile);
	fread(&dma_sr,1,sizeof(dma_sr),loadfile);
	fread(&dma_mode,1,sizeof(dma_mode),loadfile);
	fread(&dma_adrh,1,sizeof(dma_adrh),loadfile);
	fread(&dma_adrm,1,sizeof(dma_adrm),loadfile);
	fread(&dma_adrl,1,sizeof(dma_adrl),loadfile);
	fread(&mfpcycletab[0],1,sizeof(int32)*16,loadfile);
	
	//Sound vars
	fread(&psg[0],1,sizeof(Uint32)*26,loadfile);
	fread(&sample[0],1,sizeof(char)*10000,loadfile);
	
	fread(&display_mode,1,sizeof(display_mode),loadfile);
	
	//Blitter
	fread(&blt_halftone,1,sizeof(Uint16)*16,loadfile);
	fread(&blt_src_x_inc,1,sizeof(blt_src_x_inc),loadfile);
	fread(&blt_src_y_inc,1,sizeof(blt_src_y_inc),loadfile);
	fread(&blt_src_addr,1,sizeof(blt_src_addr),loadfile);
	fread(&blt_end_1,1,sizeof(blt_end_1),loadfile);
	fread(&blt_end_2,1,sizeof(blt_end_2),loadfile);
	fread(&blt_end_3,1,sizeof(blt_end_3),loadfile);
	fread(&blt_dst_x_inc,1,sizeof(blt_dst_x_inc),loadfile);
	fread(&blt_dst_y_inc,1,sizeof(blt_dst_y_inc),loadfile);
	fread(&blt_dst_addr,1,sizeof(blt_dst_addr),loadfile);
	fread(&blt_x_cnt,1,sizeof(blt_x_cnt),loadfile);
	fread(&blt_y_cnt,1,sizeof(blt_y_cnt),loadfile);
	fread(&blt_hop,1,sizeof(blt_hop),loadfile);
	fread(&blt_op,1,sizeof(blt_op),loadfile);
	fread(&blt_status,1,sizeof(blt_status),loadfile);
	fread(&blt_skew,1,sizeof(blt_skew),loadfile);
	
	
	//Ikbd
	fread(&ikbd_pulling,1,sizeof(ikbd_pulling),loadfile);
	fread(&ikbd_direct,1,sizeof(ikbd_direct),loadfile);
	fread(&inbuff[0],1,sizeof(unsigned char)*10,loadfile);
	fread(&outbuff[0],1,sizeof(unsigned char)*20,loadfile);
	inbuffi%=10;
	fread(&inbuffi,1,sizeof(inbuffi),loadfile);
	outbuffi%=20;
	fread(&outbuffi,1,sizeof(outbuffi),loadfile);
	fread(&mouse,1,sizeof(mouse),loadfile);
	fread(&joystick,1,sizeof(joystick),loadfile);
	
	//fread(&firsttick,1,sizeof(firsttick),loadfile);
	//fread(&lasttick,1,sizeof(lasttick),loadfile);
	
	fread(&sizecompressed,1,sizeof(sizecompressed),loadfile);
	fread(&msabuff[0],1,sizecompressed,loadfile);
	sizeuncompressed=MEMSIZE+4;
#ifdef USE_LZMA
	Lzma_Decode((Byte *)&membase[0],(size_t *)&sizeuncompressed,(Byte *)&msabuff[0],(size_t *)&sizecompressed);
#else
	uncompress((Bytef *)&membase[0],&sizeuncompressed,(const Bytef *)&msabuff[0],sizecompressed);
#endif
	
	fclose(loadfile);
#ifdef DEBUG_SAVESTATE
	puts( "save: loadstate - Ok!");fflush(stdout);
#endif
	return 0;
}


int savestate(int numstate){
	unsigned long  sizecompressed=0;
	FILE* savefile;
	char filename[256],tempname[256];
	struct Disk stdisk;
	int i;
	
	memset(filename,0,256);
#ifdef DREAMCAST
	sprintf ( filename, SAVE_MEM_PREFIX FILESAVE_NAME "%d" FILESAVE_EXTENSION, numstate );
#else
	sprintf ( filename, "%s%d" FILESAVE_EXTENSION, dcastaway_image_file, numstate );
#endif

#ifdef DEBUG_SAVESTATE
	printf ( "save: savestate(%i) %s [%s]\n", numstate,filename,currentdir() );fflush(stdout);
#endif

	eliminate_file(filename);
	savefile = fopen(filename,"wb");
	
	//Save 68000 vars
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - 68000" ); fflush(stdout);
#endif

#ifndef USE_FAME_CORE
	fwrite(&reg[0],1,sizeof(unsigned long)*23,savefile);
	fwrite(&dfc,1,sizeof(dfc),savefile);
	fwrite(&sfc,1,sizeof(sfc),savefile);
	fwrite(&vbr,1,sizeof(vbr),savefile);
	fwrite(&SaveWordTrc,1,sizeof(SaveWordTrc),savefile);
	fwrite(&intmask,1,sizeof(intmask),savefile);
	fwrite(&intpri,1,sizeof(intpri),savefile);
	fwrite(&cpu_state,1,sizeof(cpu_state),savefile);
	fwrite(&cpu_type,1,sizeof(cpu_type),savefile);
#else
	{
	 unsigned cvnz,stx,biginst,usp,ssp,sfc,vbr,im,cst,cty,sr;
	 int recalc_int=1;
	 fwrite(&M68KCONTEXT.dreg[0],1,sizeof(unsigned long)*8,savefile);
	 fwrite(&M68KCONTEXT.areg[0],1,sizeof(unsigned long)*8,savefile);
	 sr=M68KCONTEXT.sr;
	 cvnz=((sr&1)<<24)+((sr&2)<<15)+((sr&4)>>2)+((sr&8)<<5);
	 stx=((sr&0x10)>>4)|((sr&0x8000)>>7)|((sr&0x2000)<<3);
	 fwrite(&cvnz,1,sizeof(unsigned long),savefile);
	 fwrite(&stx,1,sizeof(unsigned long),savefile);
	 biginst=m68k_fetch(M68KCONTEXT.pc,0)|(m68k_fetch(M68KCONTEXT.pc+2,0)>>16);
	 fwrite(&biginst,1,sizeof(unsigned long),savefile);
	 fwrite(&M68KCONTEXT.pc,1,sizeof(unsigned long),savefile);
	 if (GetS())
	 {
		 ssp=M68KCONTEXT.areg[7];
		 usp=M68KCONTEXT.asp;
	 }
	 else
	 {
		 usp=M68KCONTEXT.areg[7];
		 ssp=M68KCONTEXT.asp;
	 }
	 fwrite(&usp,1,sizeof(unsigned long),savefile);
	 fwrite(&ssp,1,sizeof(unsigned long),savefile);
	 fwrite(&recalc_int,1,sizeof(unsigned long),savefile);
	 sfc=vbr=0;
	 fwrite(&M68KCONTEXT.interrupts[4],1,sizeof(unsigned long),savefile);
	 fwrite(&sfc,1,sizeof(unsigned long),savefile);
	 fwrite(&vbr,1,sizeof(unsigned long),savefile);
	 fwrite(&M68KCONTEXT.execinfo,1,sizeof(unsigned short),savefile);
	 im=GetI();
	 fwrite(&im,1,sizeof(int),savefile);
	 fwrite(&M68KCONTEXT.interrupts[0],1,sizeof(int),savefile);
	 cst=(unsigned)-1;
	 fwrite(&cst,1,sizeof(char),savefile);
	 cty=68000;
	 fwrite(&cty,1,sizeof(unsigned),savefile);
	}
#endif
	
	//Save FDC vars
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - FDC" ); fflush(stdout);
#endif
	fwrite(&fdc_data,1,sizeof(fdc_data),savefile);
	fwrite(&fdc_track,1,sizeof(fdc_track),savefile);
	fwrite(&fdc_sector,1,sizeof(fdc_sector),savefile);
	fwrite(&fdc_status,1,sizeof(fdc_status),savefile);
	fwrite(&fdc_command,1,sizeof(fdc_command),savefile);
	fwrite(&fdc_int,1,sizeof(fdc_int),savefile);
	fwrite(&fdcdir,1,sizeof(fdcdir),savefile);
	fwrite(&disk_ejected[0],1,sizeof(unsigned char)*2,savefile);
	fwrite(&disk_changed[0],1,sizeof(unsigned char)*2,savefile);
	fwrite(&discpos[0],1,sizeof(unsigned int)*2,savefile);
	
	//IO vars
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - IO" ); fflush(stdout);
#endif
	fwrite(&memconf,1,sizeof(memconf),savefile);
	fwrite(&mfp_gpip,1,sizeof(mfp_gpip),savefile);
	fwrite(&mfp_aer,1,sizeof(mfp_aer),savefile);
	fwrite(&mfp_ddr,1,sizeof(mfp_ddr),savefile);
	fwrite(&mfp_iera,1,sizeof(mfp_iera),savefile);
	fwrite(&mfp_ierb,1,sizeof(mfp_ierb),savefile);
	fwrite(&mfp_ipra,1,sizeof(mfp_ipra),savefile);
	fwrite(&mfp_iprb,1,sizeof(mfp_iprb),savefile);
	fwrite(&mfp_isra,1,sizeof(mfp_isra),savefile);
	fwrite(&mfp_isrb,1,sizeof(mfp_isrb),savefile);
	fwrite(&mfp_imra,1,sizeof(mfp_imra),savefile);
	fwrite(&mfp_imrb,1,sizeof(mfp_imrb),savefile);
	fwrite(&mfp_ivr,1,sizeof(mfp_ivr),savefile);
	fwrite(&mfp_tacr,1,sizeof(mfp_tacr),savefile);
	fwrite(&mfp_tbcr,1,sizeof(mfp_tbcr),savefile);
	fwrite(&mfp_tcdcr,1,sizeof(mfp_tcdcr),savefile);
	fwrite(&mfp_scr,1,sizeof(mfp_scr),savefile);
	fwrite(&mfp_ucr,1,sizeof(mfp_ucr),savefile);
	fwrite(&mfp_rsr,1,sizeof(mfp_rsr),savefile);
	fwrite(&mfp_tsr,1,sizeof(mfp_tsr),savefile);
	fwrite(&mfp_udr,1,sizeof(mfp_udr),savefile);
	fwrite(&mfp_reg[0],1,sizeof(int32)*32,savefile);
	
	//ACIA vars
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - ACIA" ); fflush(stdout);
#endif
	fwrite(&acia1_cr,1,sizeof(acia1_cr),savefile);
	fwrite(&acia1_sr,1,sizeof(acia1_sr),savefile);
	fwrite(&acia1_dr,1,sizeof(acia1_dr),savefile);
	fwrite(&acia2_cr,1,sizeof(acia2_cr),savefile);
	fwrite(&acia2_sr,1,sizeof(acia2_sr),savefile);
	fwrite(&acia2_dr,1,sizeof(acia2_dr),savefile);
	
	//Video vars
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - Video" ); fflush(stdout);
#endif
	fwrite(&vid_adr,1,sizeof(vid_adr),savefile);
	fwrite(&vid_baseh,1,sizeof(vid_baseh),savefile);
	fwrite(&vid_basem,1,sizeof(vid_basem),savefile);
	fwrite(&vid_syncmode,1,sizeof(vid_syncmode),savefile);
	fwrite(&vid_shiftmode,1,sizeof(vid_shiftmode),savefile);
	fwrite(&vid_col[0],1,sizeof(Sint16)*16,savefile);
	fwrite(&vid_flag,1,sizeof(vid_flag),savefile);
	
	//DMA vars
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - DMA" ); fflush(stdout);
#endif
	fwrite(&dma_car,1,sizeof(dma_car),savefile);
	fwrite(&dma_scr,1,sizeof(dma_scr),savefile);
	fwrite(&dma_sr,1,sizeof(dma_sr),savefile);
	fwrite(&dma_mode,1,sizeof(dma_mode),savefile);
	fwrite(&dma_adrh,1,sizeof(dma_adrh),savefile);
	fwrite(&dma_adrm,1,sizeof(dma_adrm),savefile);
	fwrite(&dma_adrl,1,sizeof(dma_adrl),savefile);
	fwrite(&mfpcycletab[0],1,sizeof(int32)*16,savefile);
	
	//Sound vars
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - Sound" ); fflush(stdout);
#endif
	fwrite(&psg[0],1,sizeof(Uint32)*26,savefile);
	fwrite(&sample[0],1,sizeof(char)*10000,savefile);
	
	// disp mode
	fwrite(&display_mode,1,sizeof(display_mode),savefile);
	
	//Blitter
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - Blitter" ); fflush(stdout);
#endif
	fwrite(&blt_halftone,1,sizeof(Uint16)*16,savefile);
	fwrite(&blt_src_x_inc,1,sizeof(blt_src_x_inc),savefile);
	fwrite(&blt_src_y_inc,1,sizeof(blt_src_y_inc),savefile);
	fwrite(&blt_src_addr,1,sizeof(blt_src_addr),savefile);
	fwrite(&blt_end_1,1,sizeof(blt_end_1),savefile);
	fwrite(&blt_end_2,1,sizeof(blt_end_2),savefile);
	fwrite(&blt_end_3,1,sizeof(blt_end_3),savefile);
	fwrite(&blt_dst_x_inc,1,sizeof(blt_dst_x_inc),savefile);
	fwrite(&blt_dst_y_inc,1,sizeof(blt_dst_y_inc),savefile);
	fwrite(&blt_dst_addr,1,sizeof(blt_dst_addr),savefile);
	fwrite(&blt_x_cnt,1,sizeof(blt_x_cnt),savefile);
	fwrite(&blt_y_cnt,1,sizeof(blt_y_cnt),savefile);
	fwrite(&blt_hop,1,sizeof(blt_hop),savefile);
	fwrite(&blt_op,1,sizeof(blt_op),savefile);
	fwrite(&blt_status,1,sizeof(blt_status),savefile);
	fwrite(&blt_skew,1,sizeof(blt_skew),savefile);
	
	//Ikbd
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - Ikbd" ); fflush(stdout);
#endif
	fwrite(&ikbd_pulling,1,sizeof(ikbd_pulling),savefile);
	fwrite(&ikbd_direct,1,sizeof(ikbd_direct),savefile);
	fwrite(&inbuff[0],1,sizeof(unsigned char)*10,savefile);
	fwrite(&outbuff[0],1,sizeof(unsigned char)*20,savefile);
	fwrite(&inbuffi,1,sizeof(inbuffi),savefile);
	fwrite(&outbuffi,1,sizeof(outbuffi),savefile);
	fwrite(&mouse,1,sizeof(mouse),savefile);
	fwrite(&joystick,1,sizeof(joystick),savefile);
	
	// compress to save space
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - Compressing.." );fflush(stdout);
#endif
	//fwrite(&firsttick,1,sizeof(firsttick),savefile);
	//fwrite(&lasttick,1,sizeof(lasttick),savefile);
	sizecompressed=MAX_DISC_SIZE;
#ifdef USE_LZMA
	i=Lzma_Encode((Byte *)&msabuff[0],(size_t *)&sizecompressed,(Byte *)&membase[0],MEMSIZE+4,9,65536*4);
#else
	i=compress2((Bytef *)&msabuff[0],&sizecompressed,(const Bytef *)&membase[0],MEMSIZE+4,Z_BEST_COMPRESSION);
#endif
	
#ifdef DEBUG_SAVESTATE
	printf ( "save: savestate - Compressed %iKBytes.\n",1+(sizecompressed/1024) );fflush(stdout);
#endif
	fwrite(&sizecompressed,1,sizeof(sizecompressed),savefile);

	//Memory
#ifdef DEBUG_SAVESTATE
	puts ( "save: Storing to RAM..." );fflush(stdout);
#endif
	fwrite(&msabuff[0],1,sizecompressed,savefile);
	
	fclose(savefile);

	// done!
#ifdef DEBUG_SAVESTATE
	puts ( "save: savestate - Done!" );fflush(stdout);
#endif
#ifdef  DREAMCAST
	extern int sdcard_exists;
	reinit_sdcard();
	if  (sdcard_exists) {
		FILE *fsd;
		long  len;
		memset(tempname,0,256);
		sprintf ( tempname, "%s%d" FILESAVE_EXTENSION, dcastaway_image_file, numstate );
		savefile = fopen(filename,"rb");
		if (!savefile)
			return 1;
		fseek(savefile,0,SEEK_END);
		len=ftell(savefile);
		if (len<1) {
			fclose(savefile);
			return 1;
		}
		fseek(savefile,0,SEEK_SET);
		fsd=fopen(tempname,"wb");
		if (!fsd) {
			memset(tempname,0,256);
			sprintf ( tempname, "/sd/dcastaway/%s%d" FILESAVE_EXTENSION, dcastaway_image_file, numstate );

			fsd=fopen(tempname,"wb");
			if (!fsd) {
				fclose(savefile);
				return 1;
			}
		}
		for(;len>0;len-=256)
			fwrite((void *)tempname,1,fread((void *)tempname,1,256,savefile),fsd);
		fclose(fsd);
		fclose(savefile);
	}
#endif
	return 0;
}

#if 0
static int save_to_vmu(void *buf, unsigned size)
{
	FILE *f=fopen(SAVE_PREFIX "dcastaway.sav","wb");
	int ret=(f!=NULL);
#ifdef DEBUG_SAVESTATE
	printf("save: save_to_vmu '%s' [%s]\n",SAVE_PREFIX "dcastaway.sav",currentdir());fflush(stdout);
#endif
	if (f)
	{
		unsigned s=fwrite(buf,1,size,f);
		ret=(s!=size);
		fclose(f);
	}
#ifdef DEBUG_SAVESTATE
	printf("save: save_to_vmu - return %i\n",ret);fflush(stdout);
#endif
	return ret;
}

static int load_to_vmu(void *buf, unsigned *size)
{
	FILE *f=fopen(SAVE_PREFIX "dcastaway.sav","rb");
	int ret=(f!=NULL);
#ifdef DEBUG_SAVESTATE
	printf("save: load_to_vmu '%s' [%s]\n",SAVE_PREFIX "dcastaway.sav",currentdir());fflush(stdout);
#endif
	if (f)
	{
		unsigned s;
		fseek(f,0,SEEK_END);
		size[0]=ftell(f);
		fseek(f,0,SEEK_SET);
		s=fread(buf,1,size[0],f);
		ret=(s!=size[0]);
		fclose(f);
	}
#ifdef DEBUG_SAVESTATE
	printf("save: load_to_vmu - return %i\n",ret);fflush(stdout);
#endif
	return ret;
}
#endif

#ifdef DREAMCAST
#include <unistd.h>
int loadstate_vmu(int numstate)
{
	unsigned size;
	char buf[512];
	FILE *fi, *fo;
	char filename[256];
	int ret=0;
	fi=fopen(SAVE_PREFIX "dcastaway.sav","rb");
#ifdef DEBUG_SAVESTATE
	printf("save: loadstate_vmu (%i) '%s' [%s]\n",numstate,SAVE_PREFIX "dcastaway.sav",currentdir());fflush(stdout);
#endif
	if (fi)
	{
		memset(filename,0,256);
		sprintf ( filename, SAVE_MEM_PREFIX FILESAVE_NAME "%d" FILESAVE_EXTENSION, numstate );
		eliminate_file(filename);
		fo=fopen(filename,"wb");
		if (fo)
		{
			unsigned long size,l;
			fseek(fi,0,SEEK_END);
			size=ftell(fi);
			fseek(fi,0,SEEK_SET);
			if (size%512)
				size+=512;
			size/=512;
			for(l=0;l<size;l++)
			{
				fread(buf,1,512,fi);
				if (fwrite(buf,1,512,fo)!=512)
					break;
			}
			fclose(fo);
			if (l<size)
			{
				eliminate_file(filename);
				ret=-3;
			}
			else
				ret=loadstate(numstate);
		}
		else
			ret=-2;
		fclose(fi);
	}
	else
		ret=-1;
#ifdef DEBUG_SAVESTATE
	printf("save: loadstate_vmu - return %i\n",ret);fflush(stdout);
#endif
	return ret;
}

static unsigned long file_blocks(char *filename)
{
	unsigned long size=0;
	FILE *f=fopen(filename,"rb");
#ifdef DEBUG_SAVESTATE
	printf("save: file_blocks('%s')\n",filename);fflush(stdout);
#endif
	if (f)
	{
		fseek(f,0,SEEK_END);
		size=ftell(f);
		fseek(f,0,SEEK_SET);
		fclose(f);
	}
	if (size%512)
		size+=512;
#ifdef DEBUG_SAVESTATE
	printf("save: file_blocks - return %i\n",size/512);fflush(stdout);
#endif
	return size/512;
}

int savestate_vmu(int numstate)
{
	int ret=savestate(numstate);
	char filename[256];
	char buf[512];
	unsigned long size=0;
	char *filevmu=SAVE_PREFIX "dcastaway.sav";

#ifdef DEBUG_SAVESTATE
	printf("save: savestate_vmu (%i) '%s' [%s]\n",numstate,SAVE_PREFIX "dcastaway.sav",currentdir());fflush(stdout);
#endif
	if (!ret)
	{
		FILE *fi, *fo;
		memset(filename,0,256);
		sprintf ( filename, SAVE_MEM_PREFIX FILESAVE_NAME "%d" FILESAVE_EXTENSION, numstate );
		size=file_blocks(filename);
#ifdef DEBUG_SAVESTATE
		printf("save: savestate_vmu - Blocks Input = %i\n",size);fflush(stdout);
#endif
		fi=fopen(filename,"rb");
		if (fi)
		{
			if ((getFreeBlocks(0)+file_blocks(filevmu))>=size)
			{
				eliminate_file(filevmu);
				FILE *fo=fopen(filevmu,"wb");
				if (fo)
				{
					unsigned long l=0;
					for(l=0;l<size;l++)
					{
						fread(buf,1,512,fi);
						if (fwrite(buf,1,512,fo)!=512)
							break;
					}
					fclose(fo);
					if (l<size)
					{
						eliminate_file(filevmu);
						ret=-4;
					}
				}
				else
					ret=-3;
			}
			else
				ret=-2;
			fclose(fi);
		}
		else
			ret=-1;
		
	}
#ifdef DEBUG_SAVESTATE
	printf("save: savestate_vmu - return %i\n",ret);fflush(stdout);
#endif
	return ret;
}
#endif


void savestate_init(void)
{
#ifdef DREAMCAST
	unsigned i;
	char filename[256];
#ifdef DEBUG_SAVESTATE
	puts("save: savestate_init");fflush(stdout);
#endif
	for(i=0;i<MAX_SAVESTATES;i++)
	{
		memset(filename,0,256);
		sprintf ( filename, SAVE_MEM_PREFIX FILESAVE_NAME "%d" FILESAVE_EXTENSION, i);
		eliminate_file(filename);
	}
	prepare_save();
#endif
}

extern unsigned dcastaway_disc_crc[2];
extern unsigned dcastaway_disc_actual_crc[2];
extern unsigned char disc[2][MAX_DISC_SIZE];
extern void *dcastaway_disc_orig[2];

static char __disc_write_namefile[64];

static char *get_namefile(unsigned num)
{
	unsigned crc=dcastaway_disc_crc[num];
	sprintf((char *)&__disc_write_namefile[0],SAVE_PREFIX "%.8X.dsv",crc);
	return (char *)&__disc_write_namefile[0];
}

#ifdef DREAMCAST
static char *get_namefile_sd(unsigned num)
{
	unsigned crc=dcastaway_disc_crc[num];
	sprintf((char *)&__disc_write_namefile[0], "%.8X.dsv",crc);
	return (char *)&__disc_write_namefile[0];
}
static char *get_namefile_sd2(unsigned num)
{
	unsigned crc=dcastaway_disc_crc[num];
	sprintf((char *)&__disc_write_namefile[0], "/sd/dcastaway/%.8X.dsv",crc);
	return (char *)&__disc_write_namefile[0];
}
#endif



void dcastaway_disc_real_write(int num)
{
	unsigned new_crc=savedisk_get_checksum((void *)&disc[num][0],MAX_DISC_SIZE);
	if (new_crc!=dcastaway_disc_actual_crc[num])
	{
		void *buff=(void *)&disc[num][0];
		void *buff_patch=(void *)&msabuff[0];
		memset(buff_patch,0,MAX_DISC_SIZE);
		unsigned changed=savedisk_get_changes(buff,MAX_DISC_SIZE,buff_patch,dcastaway_disc_orig[num]);

		if ((changed)&&(changed<MAX_DISC_SIZE))
		{
			char *namefile=get_namefile(num);
			void *bc=calloc(1,MAX_COMP_SIZE);
			unsigned long sizecompressed=MAX_COMP_SIZE;
#ifdef USE_LZMA
			int retc=Lzma_Encode((Byte *)bc,(size_t *)&sizecompressed,(Byte *)&msabuff[0],changed,9,(1<<15));
			if (retc== SZ_OK)
#else
			int retc=compress2((Bytef *)bc,&sizecompressed,(const Bytef *)&msabuff[0],changed,Z_BEST_COMPRESSION);
			if (retc>=0)
#endif
			{
				unsigned usado=0;
				int is_sd=1;
				{
					FILE *f=fopen(namefile,"rb");
					if (f)
					{
						fseek(f,0,SEEK_END);
						usado=ftell(f);
						fclose(f);
						usado/=512;
						is_sd=0;
					}
				}
				if ( ((getFreeBlocks(is_sd)+usado)*512) >=(sizecompressed+VMUFILE_PAD))
				{
					FILE *f;
					if (!is_sd) {
						eliminate_file(namefile);
						f=fopen(namefile,"wb");
					} else  {
#ifdef DREAMCAST
						extern int sdcard_exists;
						if (sdcard_exists) {
							f=fopen(get_namefile_sd(num),"wb");
							if (!f)
								f=fopen(get_namefile_sd2(num),"wb");
						}
#endif
					}
					if (f)
					{
						if (!is_sd)
							rebuild_paquete(disk[num].name, sizecompressed, (unsigned char*) bc, f);
						fwrite((void *)&sizecompressed,1,4,f);
						fwrite(bc,1,sizecompressed,f);
						fclose(f);
					}
				}
			}
			free(bc);
			dcastaway_disc_actual_crc[num]=new_crc;
		}
	}
}


void dcastaway_disc_initsave(unsigned num)
{
#if !defined(DREAMCAST) || defined(AUTOSAVESTATE)
	if (!dcastaway_disc_orig[num])
		dcastaway_disc_orig[num]=malloc(MAX_DISC_SIZE);
#else
	dcastaway_disc_orig[num]=(void *)(0x04200000+(num*MAX_DISC_SIZE));
#endif
	memcpy(dcastaway_disc_orig[num],(void *)&disc[num][0],MAX_DISC_SIZE);
	dcastaway_disc_crc[num]=savedisk_get_checksum(dcastaway_disc_orig[num],MAX_DISC_SIZE);

	if (mainMenu_savedisk)
	{
		FILE *f=fopen(get_namefile(num),"rb");
		int is_sd=0;
#ifdef DREAMCAST
		if (!f) {
			extern int sdcard_exists;
			if (sdcard_exists) {
				is_sd=1;
				f=fopen(get_namefile_sd(num),"rb");
				if  (!f) 
					f=fopen(get_namefile_sd2(num),"rb");
			}
		} 
#endif
		if (f)
		{
			void *bc=calloc(1,MAX_COMP_SIZE);
			unsigned long n;
			if (!is_sd)
				set_vmu_pad(f);
			fread((void *)&n,1,4,f);
			if (fread(bc,1,n,f)>=n)
			{
				unsigned long sizeuncompressed=MAX_DISC_SIZE;
#ifdef USE_LZMA
				int nn=n;
				int retc=Lzma_Decode((Byte *)&msabuff[0],(size_t *)&sizeuncompressed,(Byte *)bc,(size_t *)&nn);
				if (retc== SZ_OK)
#else
				int retc=uncompress((Bytef *)&msabuff[0],&sizeuncompressed,(const Bytef *)bc,n);
				if (retc>=0)
#endif
				{
					savedisk_apply_changes((void *)&disc[num][0],(void *)&msabuff[0],sizeuncompressed);
				}
				else
				{
					fclose(f);
					f=NULL;
					if (!is_sd)
						eliminate_file(get_namefile(num));
				}
			}
			free(bc);
			if (f)
				fclose(f);
		}
		dcastaway_disc_actual_crc[num]=savedisk_get_checksum((void *)&disc[num][0],MAX_DISC_SIZE);

	}
	else
		dcastaway_disc_actual_crc[num]=dcastaway_disc_crc[num];
}

