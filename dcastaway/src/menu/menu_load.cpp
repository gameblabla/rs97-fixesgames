
#ifndef NO_MENU

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "menu.h"
#include "db.h"

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "sound/sound.h"
#include "tvfilter/tvfilter.h"

#include <ctype.h>

#define MAX_FILENAME_LEN 80

typedef struct{
	char d_name[MAX_FILENAME_LEN];
	char d_type;
}fichero;

typedef struct{
	fichero *fich;
	char *map;
}fichmap_t;

typedef struct{
	char **gamelist;
	Uint16 numgames;
}mapeo;

extern int changed_fdc0;
extern int changed_fdc1;

#ifdef DREAMCAST
#define chdir(A) fs_chdir(A)
static char actual_dir[512];
#endif

#define MAX_FILES_PER_DIR (1024*4)
#define SHOW_MAX_FILES 12

extern char *dcastaway_image_file;
extern char *dcastaway_image_file2;
extern int use_gamesdb;

static char *text_str_load_separator="~~~~~~~~~~~~~~~~~~~~~~~~";
static char *text_str_load_dir="#DIR#";
static char *text_str_load_title="Filemanager";
static fichero *text_dir_files=NULL;
static fichmap_t *fichmap=NULL;
static mapeo *mapeos=NULL;
static int text_dir_num_files=0, text_dir_num_files_index=0;

static char *text_load=NULL;

static void clean_mapeos(void)
{
	unsigned i;
	for(i=0;i<MAX_FILES_PER_DIR;i++)
		if (mapeos[i].gamelist)
			db_free ( mapeos[i].numgames, mapeos[i].gamelist );
		else
			break;
	memset(mapeos,0,sizeof(mapeo)*MAX_FILES_PER_DIR);
}

static int min_in_dir=0, max_in_dir=SHOW_MAX_FILES;

static int compare_names(fichmap_t *a, fichmap_t *b)
{
	return strcmp(a->map,b->map);
}

static int is_unique(char *newname, unsigned me)
{
	unsigned i;
	for(i=0;i<text_dir_num_files;i++)
		if ((fichmap[i].map)&&(i!=me))
			if (!strcmp(fichmap[i].map,newname))
				return 0;
	return 1;
}

static void always_unique(void)
{
	unsigned i,l;
	for(i=0;i<text_dir_num_files;i++)
		if (fichmap[i].map)
		{
			char cual='B';
			l=strlen(fichmap[i].map);
			while(!is_unique(fichmap[i].map,i))
			{
				fichmap[i].map[l]='(';
				fichmap[i].map[l+1]=cual;
				fichmap[i].map[l+2]=')';
				cual++;
			}
		}
}

static int checkFiles(void)
{
	char *buf=(char *)calloc(1,2046);
	int i,max=text_dir_num_files;
	int ret=(text_dir_num_files<1);
	if (max>16)
		max=16;
	for(i=0;(i<max)&&(!ret);i++)
	{
		strcpy(buf,text_dir_files[i].d_name);
		if (!buf[0])
			ret=1;
		else
		if (!text_dir_files[i].d_type)
		{
			FILE *f=fopen(buf,"rb");
			if (!f)
				ret=1;
			else
				fclose(f);
		}
		else
		if (strcmp(buf,".."))
		{
			DIR *d=opendir(buf);
			if (!d)
				ret=1;
			else
				closedir(d);
		}
	}
	free(buf);
	return ret;
}

static void draw_bar(int per, int max)
{
	text_draw_window(80,64,172,48,"Process");
	text_draw_barra(84, 92, 150, 12, per+1, max);
	write_text(14,9,"Please wait");
	text_flip();
}

static  int getFiles(char *dir)
{
	int i,j,k;
	DIR *d;
	text_dir_num_files_index=0;
	min_in_dir=0;
	max_in_dir=SHOW_MAX_FILES;
	text_dir_num_files=0;

	if (text_dir_files!=NULL)
	{
		free(fichmap);
		free(text_dir_files);
	}

	draw_bar(-1,100);

	text_dir_files=(fichero *)calloc(sizeof(fichero),MAX_FILES_PER_DIR);
	fichmap=(fichmap_t *)calloc(sizeof(fichmap_t),MAX_FILES_PER_DIR);
	if (mapeos==NULL)
		mapeos=(mapeo *)calloc(sizeof(mapeo),MAX_FILES_PER_DIR);
#ifdef DREAMCAST
	if (!strcmp(dir,".."))
	{
		int ind;
		for(ind=strlen(actual_dir)-1;ind>0;ind--)
			if (actual_dir[ind]=='/')
			{
				actual_dir[ind]=0;
				break;
			}
		d=opendir(actual_dir);	
	}
	else
#endif
	d=opendir(dir);
	if (d==NULL)
		return -1;
	for(i=0;i<MAX_FILES_PER_DIR;i++)
	{
		struct dirent *actual=readdir(d);
		if (actual==NULL)
			break;
#ifdef DREAMCAST
		else
		{
			unsigned itl=0;
			while(actual->d_name[itl])
			{
				actual->d_name[itl]=tolower(actual->d_name[itl]);
				itl++;
			}
		}
#endif
		if ((!strcmp(actual->d_name,"."))||(!strcmp(actual->d_name,"rom"))||(!strcmp(actual->d_name,CACHEFILE))||(!strcmp(actual->d_name,"1st_read.bin"))||(!strcmp(actual->d_name,"ip.bin"))||(!strcmp(actual->d_name,"dcastaway.app"))||(!strcmp(actual->d_name,"dcastaway_safe.app"))||(!strcmp(actual->d_name,"dcastaway.exe"))||(!strcmp(actual->d_name,"dcastaway_safe.exe"))||(!strcmp(actual->d_name,".ds_store"))||(!strcmp(actual->d_name,"dcastaway.linux"))||(!strcmp(actual->d_name,"dcastaway_safe.linux")))
		{
			i--;
			continue;
		}
		if (strlen(actual->d_name)>3)
		{
			char *final=(char *)&actual->d_name[strlen(actual->d_name)-3];
			if ((!strcmp(final,"dsv"))||(!strcmp(final,"DSV"))||(!strcmp(final,"SST"))||(!strcmp(final,"sst")))
			{
				i--;
				continue;
			}
		}
		strncpy(text_dir_files[i].d_name,actual->d_name,MAX_FILENAME_LEN-1);
#ifdef DEBUG_FILEMANAGER
		printf("Fichero(%i): '%s'\n",i,text_dir_files[i].d_name);
#endif
		fichmap[i].map=(char *)&text_dir_files[i].d_name;
		fichmap[i].fich=&text_dir_files[i];
#ifndef DREAMCAST
		{
			struct stat sstat;
			char *tmp=(char *)calloc(1,256);
			strcpy(tmp,dir);
			strcat(tmp,FILE_SEPARATOR);
			strcat(tmp,text_dir_files[i].d_name);
			if (!stat(tmp, &sstat))
		        	if (S_ISDIR(sstat.st_mode))
					text_dir_files[i].d_type=4;
			free(tmp);
		}
#else
		text_dir_files[i].d_type=actual->d_type & 4;
#endif
	}
	closedir(d);
	text_dir_num_files=i;

#ifndef DREAMCAST
	chdir(dir);
#else
	if (strcmp(dir,MENU_DIR_DEFAULT))
	{
		if (strcmp(dir,".."))
		{
			strcat(actual_dir,"/");
			strcat(actual_dir,dir);
		}
	}
	chdir(actual_dir);
	if (strcmp(actual_dir,MENU_DIR_DEFAULT))
	{
		strcpy(text_dir_files[i].d_name,"..");
		fichmap[i].map=(char *)&text_dir_files[i].d_name;
		fichmap[i].fich=&text_dir_files[i];
		text_dir_files[i].d_type=4;
		if (text_dir_num_files>0)
		{
			char *pptmp=(char *)calloc(1,MAX_FILENAME_LEN);
			int tmptype=text_dir_files[0].d_type;
			strcpy(pptmp,text_dir_files[0].d_name);
			text_dir_files[0].d_type=text_dir_files[text_dir_num_files].d_type;
			text_dir_files[text_dir_num_files].d_type=tmptype;
			strcpy(text_dir_files[0].d_name,text_dir_files[text_dir_num_files].d_name);
			strcpy(text_dir_files[text_dir_num_files].d_name,pptmp);
			fichmap[0].map=(char *)&text_dir_files[0].d_name;
			fichmap[0].fich=(fichero *)&fichmap[0].map;
			fichmap[text_dir_num_files].map=(char *)&text_dir_files[text_dir_num_files].d_name;
			fichmap[text_dir_num_files].fich=(fichero *)&fichmap[text_dir_num_files];
			free(pptmp);
		}
		text_dir_num_files++;
	}
#endif


	for(i=0;i<text_dir_num_files;i++)
	{
		if (text_dir_files[i].d_type==0)
			for(j=i;j<text_dir_num_files;j++)
				if (text_dir_files[j].d_type==4)
				{
					char *ctmp=(char *)calloc(1,MAX_FILENAME_LEN);
					strcpy(ctmp,text_dir_files[j].d_name);
					strcpy(text_dir_files[j].d_name,text_dir_files[i].d_name);
					strcpy(text_dir_files[i].d_name,ctmp);
					text_dir_files[i].d_type=4;
					text_dir_files[j].d_type=0;
					free(ctmp);
					break;
				}
		fichmap[i].fich=&text_dir_files[i];
		fichmap[i].map=(char *)text_dir_files[i].d_name;
#ifdef DEBUG_FILEMANAGER
		printf("fichmap(%i) = '%s'\n",i,fichmap[i].map);
#endif

	}

	cache_retrieve();
	audio_play_wait();
	j=k=text_dir_num_files;
#ifdef DEBUG_FILEMANAGER
	printf("num_files=%i, max=%i\n",text_dir_num_files,MAX_FILES_PER_DIR);
#endif
	for(i=0;i<j && use_gamesdb;i++)
	{
#ifdef DEBUG_FILEMANAGER
		printf("Fichero: '%s' (%i)\n",text_dir_files[i].d_name,text_dir_files[i].d_type);
#endif
		draw_bar(i,j);
		if (text_dir_files[i].d_type==0 && k<MAX_FILES_PER_DIR)
		{
		    	cache_t *cache;
			Uint32 l=k-j;
			Uint32 crc=0;
			
			if((cache = cache_find(fichmap[i].map)))
				crc=cache->c_crc32;
#if defined(CALCULE_CRC_FILES) || defined(DEBUG_FILEMANAGER)
			else
				crc=db_calculate_id_on_file (fichmap[i].map);
#endif
#ifdef DEBUG_FILEMANAGER
			printf("-CRC=%x\n",crc);
#endif
			if (crc && db_find ( crc, &mapeos[l].numgames, &mapeos[l].gamelist ))
			{
				unsigned indice;
				fichmap[i].map=mapeos[l].gamelist[0];
#ifdef DEBUG_FILEMANAGER
				printf("-- '%s'\n",fichmap[i].map);
#endif
				for(indice=1;indice<mapeos[l].numgames && k<MAX_FILES_PER_DIR;indice++)
				{
					strcpy((char *)&text_dir_files[k].d_name,(char *)&text_dir_files[i].d_name);
					text_dir_files[k].d_type==0;
					fichmap[k].map=mapeos[l].gamelist[indice];
					fichmap[k].fich=&text_dir_files[k];
#ifdef DEBUG_FILEMANAGER
					printf("-- '%s'\n",fichmap[k].map);
#endif
					k++;
				}
			}
#ifdef DEBUG_FILEMANAGER
			else puts("NO ENCONTRADO!!!!!!!");
#endif
		}
#ifdef DEBUG_FILEMANAGER
		else puts("Directorio");
#endif
	}
	cache_free();
	text_dir_num_files=k;
	always_unique();
	for(i=0;i<k;i++)
		if (text_dir_files[i].d_type==0)
		{
#ifdef DEBUG_FILEMANAGER
			puts("QSORT"); fflush(stdout);
#endif
			qsort((void *)&fichmap[i],k-i,sizeof(fichmap_t),(int (*)(const void*, const void*))compare_names);
#ifdef DEBUG_FILEMANAGER
			puts("!QSORT"); fflush(stdout);
#endif
			break;
		}
	return 0;
}


static  void draw_loadMenu(int c)
{
	int i,j;
//	static int b=0;
//	int bb=(b%6)/3;
	SDL_Rect r;
	extern SDL_Surface *text_screen;
//	r.x=80-64; r.y=0; r.w=110+64+64; r.h=240;
	r.x=80-64; r.y=18; r.w=110+64+64-16; r.h=208;

	text_draw_background();
//	text_draw_window(80-64,12,160+64+64,220,text_str_load_title);
	text_draw_window(80-64,18,160+64+64-4,208-2,text_str_load_title);

	if (text_dir_num_files_index<min_in_dir)
	{
		min_in_dir=text_dir_num_files_index;
		max_in_dir=text_dir_num_files_index+SHOW_MAX_FILES;
	}
	else
		if (text_dir_num_files_index>=max_in_dir)
		{
			max_in_dir=text_dir_num_files_index+1;
			min_in_dir=max_in_dir-SHOW_MAX_FILES;
		}
	if (max_in_dir>text_dir_num_files)
		max_in_dir=text_dir_num_files-min_in_dir;


	for (i=min_in_dir,j=1;i<max_in_dir;i++,j+=2)
	{
		if (i!=min_in_dir)
			write_text(3,j+1,text_str_load_separator);

		SDL_SetClipRect(text_screen,&r);
		
		if ((text_dir_num_files_index==i)) //&&(bb))
			write_text_sel(3,j+2,276,fichmap[i].map);
		else
			write_text(3,j+2,(char *)fichmap[i].map);

		SDL_SetClipRect(text_screen,NULL);

		if (fichmap[i].fich->d_type==4)
			write_text(30,j+2,text_str_load_dir);
	}
//	write_text(3,j,text_str_load_separator);
	text_flip();
//	b++;
}


static  int key_loadMenu(int *c)
{
	static int lag=0;
	int end=0;
	static int left=0, right=0, up=0, down=0;
	int hit0=0, hit1=0, hit2=0;
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		if (event.type == SDL_QUIT)
			end=-1;
		else
		if (event.type == SDL_VIDEORESIZE)
			TV_ResizeWindow(event.resize.w, event.resize.h);
		else
		if (event.type == SDL_KEYUP)
		{
			switch(event.key.keysym.sym)
			{
				case SDLK_w:
				case SDLK_UP: up=0; break;
				case SDLK_s:
				case SDLK_DOWN: down=0; break;
				case SDLK_a:
				case SDLK_LEFT: left=0; break;
				case SDLK_d:
				case SDLK_RIGHT: right=0; break;
				case SDLK_F12: TV_ToggleFullScreen(screen); break;
			}
		}
		else
		if (event.type == SDL_KEYDOWN)
		{
			lag=0;
			audio_play_click();
			switch(event.key.keysym.sym)
			{
				case SDLK_d:
				case SDLK_RIGHT: right=1; break;
				case SDLK_a:
				case SDLK_LEFT: left=1; break;
				case SDLK_w:
				case SDLK_UP: up=1; break;
				case SDLK_s:
				case SDLK_DOWN: down=1; break;
				case SDLK_c:
				case SDLK_LSHIFT:
				case SDLK_x:
				case SDLK_SPACE: hit2=1; break;
				case SDLK_z:
				case SDLK_RETURN:
				case SDLK_e:
				case SDLK_LCTRL: hit0=1; break;
				case SDLK_1:
#ifdef DREAMCAST
				case SDLK_TAB:
#else
				case SDLK_BACKSPACE:
#endif
						 if (text_dir_num_files)
						 	text_dir_num_files_index=text_dir_num_files-1;
						 break;
				case SDLK_2:
#ifdef DREAMCAST
				case SDLK_BACKSPACE:
#else
				case SDLK_TAB:
#endif
						 text_dir_num_files_index=0;
						 break;
				case SDLK_q:
				case SDLK_LALT: hit1=1; break;
			}
			if ((hit0)||(hit2))
			{
				if ((fichmap[text_dir_num_files_index].fich->d_type==4)||(!strcmp(fichmap[text_dir_num_files_index].map,"."))||(!strcmp(fichmap[text_dir_num_files_index].map,"..")))
				{
					char *tmp=(char *)calloc(1,512);
					strcpy(tmp,(char *)&fichmap[text_dir_num_files_index].fich->d_name);
					clean_mapeos();
					if (getFiles(tmp))
						end=-1;
					free(tmp);
					break;
				}
				else
				{
#ifdef DEBUG_FILEMANAGER
					printf("A CARGAR '%s' con mapeo '%s'\n",(char *)&fichmap[text_dir_num_files_index].fich->d_name,fichmap[text_dir_num_files_index].map);
#endif
					if (hit0)
					{
						strcpy(dcastaway_image_file,(char *)&fichmap[text_dir_num_files_index].fich->d_name);
						changed_fdc0=1;
					}
					else
					{
						strcpy(dcastaway_image_file2,(char *)&fichmap[text_dir_num_files_index].fich->d_name);
						changed_fdc1=1;
					}
					end=1;
					break;
				}
			}
			else if (hit1)
			{
				end=-1;
				break;
			}
		}
	}
	if (!(lag&7))
	{
		if (up)
		{
			audio_play_click();
			if (text_dir_num_files_index>0)
				text_dir_num_files_index--;
		}
		else if (down)
		{
			audio_play_click();
			if (text_dir_num_files_index+1!=text_dir_num_files)
				text_dir_num_files_index++;
		}
		else if (left)
		{
			audio_play_click();
			text_dir_num_files_index-=SHOW_MAX_FILES;
			if (text_dir_num_files_index<0)
				text_dir_num_files_index=0;
		}
		else if (right)
		{
			audio_play_click();
			text_dir_num_files_index+=SHOW_MAX_FILES;
			if (text_dir_num_files_index+1>=text_dir_num_files)
				text_dir_num_files_index=text_dir_num_files-1;
		}
	}
	lag++;
	return end;
}

static  void raise_loadMenu()
{
	int i;

	audio_play_file();
	text_draw_background();
	text_flip();
	for(i=0;i<10;i++)
	{
		text_draw_background();
		text_draw_window(126-(10*i),(10-i)*24,188+(10*i),220,"");
		text_flip();
		SDL_Delay(15);
	}
}

static  void unraise_loadMenu()
{
	int i;

	for(i=9;i>=0;i--)
	{
		text_draw_background();
		text_draw_window(126-(10*i),(10-i)*24,188+(10*i),220,"");
		text_flip();
		SDL_Delay(15);
	}
	text_draw_background();
	text_flip();
}

static int getDefaultFiles(void)
{
#ifdef DREAMCAST
	strcpy(actual_dir,MENU_DIR_DEFAULT);
#endif
	return(getFiles(MENU_DIR_DEFAULT));
}


int run_menuLoad()
{
	int end=0,c=0;
#ifdef DREAMCAST
	extern void reinit_sdcard(void);
	reinit_sdcard();
#endif

	if (text_dir_files==NULL)
		end=getDefaultFiles();
	else
	if (!text_dir_num_files)
		end=getDefaultFiles();
	else
	if (checkFiles())
		end=getDefaultFiles();

	raise_loadMenu();
	while((!end)&&(text_dir_num_files))
	{
		draw_loadMenu(c);
		end=key_loadMenu(&c);
	}
	unraise_loadMenu();

	return end;
}

#endif
