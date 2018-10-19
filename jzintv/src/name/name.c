#include "config.h"
#include "name.h"

#ifdef GCWZERO
//#include <inttypes.h>
#include <string.h>
#include "jzintv.h"
//#include "bincfg/legacy.h"

	    char overlayname[1000];
	    char overlay[999], pngextension[10], pathtopng[999];
	    int *resetflag=0;
#endif
const char *find_cart_name(uint_32 crc32, int *year, 
                           int *default_ecs, int *default_ivc)
{
    int i;

    for (i = 0; name_list[i].name; i++)
    {
        if (name_list[i].crc32 == crc32)
        {
            if (year)           *year        = name_list[i].year;
            if (default_ecs)    *default_ecs = name_list[i].ecs;
            if (default_ivc)    *default_ivc = name_list[i].ivc;
#ifdef GCWZERO //determine overlay name and path
if (!resetflag) {
	    strcpy(overlay, name_list[i].name);
	    strcat(pngextension, ".png");
//	    strcat(pathtopng, "./overlays/");
	    strcat(pathtopng, "/mnt/int_sd/.jzintellivision/overlays/");
	    strcat(overlay, pngextension);
	    strcat(pathtopng, overlay);
	    strcpy(overlay, pathtopng);
	    memcpy(overlayname, overlay, strlen(overlay)+1);
	    jzp_printf("\nOverlay Name=\n%s\n", overlayname);
	    }
#endif

#ifdef GCWZERO 

//	jzp_printf("crc32=%" PRIu32 "\n",crc32);
	jzp_printf("\nSearching for config file for romname\n%s\n", name_list[i].name);

	const char *cfgarray[84]=
	{
		"Atlantis"		,		"7",
"UNKNOWN Baseball 2 (proto)"	,		"3",
		"Beauty and the Beast"	,		"7",
		"Body Slam Super Pro Wrestling",	"2",
		"Centipede"		,		"6",
		"USCF Chess"		,		"4",
		"Chip Shot Super Pro Golf (Caddie)",	"2",
		"Chip Shot Super Pro Golf (Caddy)",	"2",
		"Championship Tennis"	,		"1",
		"Choplifter (Unreleased)",		"5",
		"Choplifter (Unreleased)",		"1",
		"Congo Bongo"		,		"5",
		"Commando"		,		"2",
		"Super Pro Decathlon"	,		"2",
		"Deep Pockets Super Pro Pool & Billiards (Unreleased)","2",
		"Defender"		,		"5",
		"Mattell Demonstration Cartridge 1983",	"1",
		"Demon Attack"		,		"7",
		"Dig Dug"		,		"5",
		"Diner"			,		"2",
		"Hover Force 3-D (Unfinished)",		"2",
		"Hover Force"		,		"2",
		"King of the Mountain"	,		"1",
		"Land Battle"		,		"4",
		"Learning Fun II"	,		"2",
		"Microsurgeon"		,		"7",
		"MTE-201 Test Cartridge",		"8",
		"Stadium Mud Buggies"	,		"2",
		"Pac-Man (Atarisoft)"	,		"5",
		"Pac-Man (INTV)"	,		"5",
		"Pole Position"		,		"2",
		"Quest (Unfinished)"	,		"1",
		"Scarfinger"		,		"2",
		"Space Shuttle?"	,		"1",
		"Slam Dunk Super Pro Basketball",	"2",
"UNKNOWNC.M. Sound Test"	,		"3",
		"Super Pro Football"	,		"2",
		"Spiker! SP Volleyball"	,		"2",
		"Tower of Doom"		,		"3",
		"Triple Challenge"	,		"9",
		"World Series Major League Baseball (Incomplete)","1",
		"World Series Major League Baseball",	"1",
	};
	int j;
	char cfgnumber[10];
	for (j=0; j<83; j++)
	{
		if (strcmp(name_list[i].name,cfgarray[j])==0)
		{
/*NOW CREATE .cfg FILE THEN RESTART*/
			memcpy(cfgnumber, cfgarray[j+1], sizeof(cfgarray[j+1]));
			jzp_printf("\nConfig file %s NEEDED\n",cfgnumber);
// read rom path
			char configfilecontents[999];
			FILE *file;
			file=fopen("/name/game/.jzintellivision/.filename","r");
			if( fgets (configfilecontents, 999, file)!=NULL ) {
			}else jzp_printf("fgets didn't work, FILE EMPTY");
			fclose(file);
// replace extension with .cfg
//			char *ftowrite;
//			ftowrite = strrtok (configfilecontents,".");
//			strcat(ftowrite,".cfg");
//jzp_printf("ftowrite=",ftowrite);
// now create ./cfgnumber.cfg
			char number[2];
			number[0] = cfgnumber[0];
			number[1] = '\0';
			char ftoread[7]="./0.cfg";
			ftoread[2]=cfgnumber[0];
			jzp_printf("\nConfig filename to use is\n%s\n",ftoread);
//does target romname.cfg already exist?
			if( access( configfilecontents, F_OK ) != -1 ) //GCW TODO CHANGED HERE
			{
				jzp_printf("\nConfig file already present.\n");
			} else {
//file empty, lock and load...
				FILE *source, *target;
				char *ch;
				source = fopen(ftoread, "r");
				target = fopen(configfilecontents, "w");
				while( ( ch = fgetc(source) ) != EOF )	fputc(ch, target);
				jzp_printf("\nConfig file copied successfully.\n");
				fclose(source);
				fclose(target);
//				jzp_printf("Now we must reset emulator\n");
				resetflag=1;

			}

//			}
			   

			break;
		} 
	}

#endif

            return name_list[i].name;
        }
    }

    return NULL;
}

