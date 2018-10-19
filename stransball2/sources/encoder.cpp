#include "stdio.h"

void encode(char *in,char *out)
{
	int seed[16]={1,7,3,9,34,1,24,1, 
				  8,25,14,65,2,7,4,1};
	int i=0,val;

	FILE *fpin;
	FILE *fpout;

	fpin=fopen(in,"rb");
	fpout=fopen(out,"wb");
	if (fpin==0 || fpout==0) {
		if (fpout) fclose(fpout);
		if (fpin) fclose(fpin);
		return;
	}

	do{
		val=fgetc(fpin);
		if (!feof(fpin)) {
			fputc(int('A'+(((val+seed[i%16])>>4)%16)),fpout);
			fputc(int('A'+((val+seed[i%16])%16)),fpout);
			i++;
		} /* if */ 
	}while(!feof(fpin));

	fclose(fpout);
	fclose(fpin);

} /* encode */ 



void decode(char *in,char *out)
{
//	int seed[32]={1,1,7,7,3,3,9,9,34,34,1,1,24,24,1,1,
//				  8,8,25,25,14,14,65,65,2,2,7,7,4,4,1,1};
	int seed[16]={1,7,3,9,34,1,24,1, 
				  8,25,14,65,2,7,4,1};
	int i=0,val1,val2;

	FILE *fpin;
	FILE *fpout;

	fpin=fopen(in,"rb");
	fpout=fopen(out,"wb");
	if (fpin==0 || fpout==0) {
		if (fpout) fclose(fpout);
		if (fpin) fclose(fpin);
		return;
	}

	do{
		val1=fgetc(fpin);
		val2=fgetc(fpin);
		if (!feof(fpin)) {
			fputc(int( (int(val1-'A')<<4) + int(val2-'A') - seed[(i%16)] ),fpout);
			i++;
		} /* if */ 
	}while(!feof(fpin));

	fclose(fpout);
	fclose(fpin);
} /* decode */ 

