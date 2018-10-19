/**************************
汎用サブルーチン群
**************************/

#include "CBmps.h"
#include "geometry.h"


extern SDL_Surface *ScreenSurface;
//画像用構造体初期化
//引数：最大画像数
CBmps* CBmpsInit(int nums){
	CBmps *p;

	if( nums < 0 )return(NULL);

	p=(CBmps *)malloc(sizeof(CBmps));
	if(NULL==p){
		fprintf(stderr,"ERROR:NO MEMORY at CBmpsInit()\n");
		return(NULL);
	}
	memset(p,0,sizeof(CBmps));
	p->nums=nums;

	p->bmp=(SDL_Surface **)malloc(sizeof(SDL_Surface *)*p->nums);
	if(p->bmp == NULL){
		fprintf(stderr,"ERROR:NO MEMORY at CBmpsInit()\n");
		free(p);
		return(NULL);
	}



	memset(p->bmp,0,sizeof(SDL_Surface *)*p->nums);
	p->index=0;
	return(p);
}
//画像用解放
int CBmpsFree(CBmps *this){

	int i;

	//メモリはロード時に確保される
	for(i=0;i<this->nums;++i)
		if(this->bmp[i]!=NULL){
			SDL_FreeSurface(this->bmp[i]);
			this->bmp[i]=NULL;
		}

	if(NULL!=this->bmp){
		free(this->bmp);
		this->bmp=NULL;
	}
	if(NULL!=this){
		free(this);

	}

	return(0);
}
//画像用データロード
int CBmpsLoad(CBmps *this, char *filename){
	return CBmpsLoadWzNum(this,filename,this->index);
}
int CBmpsLoadWzNum(CBmps* this,char *filename,int num){


	if(this==NULL)return(-1);
	this->index=num;
	if(this->index>=this->nums){
		fprintf(stderr,"ERROR:excess CBmps.nums %d\n",num);
		return(-1);
	}

	this->bmp[this->index]=SDL_LoadBMP(filename);

	if(NULL==this->bmp[this->index]){
		fprintf(stderr,"ERROR:NO MEMORY or cant find %s at CBmpsLoad()\n",filename);
		return(-1);
	}



	this->index++;
	return(0);
}

//プライベート関数、画像をスクリーンフォーマットへ変換
int CBmpsConvert(CBmps* this){

	int i,res=0;
	SDL_Surface *sf,*converted;

	for(i=0;i<this->nums;++i){
		sf=this->bmp[i];
		if(NULL!=sf){
			//flag is set to 0
//			converted=SDL_ConvertSurface(sf,screen->format,0);
			converted=SDL_DisplayFormat(sf);
			if(NULL==converted){
				fprintf(stderr,"ERROR:cant convert at CBmpsConvert\n");
				res=-1;
			}else{
				SDL_FreeSurface(this->bmp[i]);
				this->bmp[i]=converted;
			}
		}
	}

	return(res);
}
int CBmpsSetTransparent(CBmps* this,int r,int g,int b){
	int i;
	Uint32 trans;
	SDL_Surface *sf;
	for(i=0;i<this->nums;++i){
		sf=this->bmp[i];
		if(NULL!=sf){
			//set transparent color
			trans=SDL_MapRGB(sf->format,r,g,b);
			SDL_SetColorKey(sf,SDL_SRCCOLORKEY,trans);
		}
	}
	return(0);
}
/*
画面の初期化、スクリーンを返す
*/

SDL_Surface* CScreenInitDefault(void){

	int flags=SDL_SWSURFACE;
	return CScreenInit(SCREEN_WIDTH,SCREEN_HEIGHT,16,flags);
}
SDL_Surface* CScreenInitDefaultHW(void){


//	int flags=SDL_ASYNCBLIT|SDL_HWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF;
	int flags=SDL_HWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF;
//	int flags=SDL_HWSURFACE|SDL_FULLSCREEN;
//	int flags=SDL_SWSURFACE;
//	int flags=SDL_SWSURFACE|SDL_FULLSCREEN|SDL_HWPALETTE;
	return CScreenInit(SCREEN_WIDTH,SCREEN_HEIGHT,16,flags);
}

SDL_Surface* CScreenInit(int w,int h,int bpp,int flags){

	SDL_Surface *p;

	if(SDL_Init(SDL_INIT_VIDEO)){
		fprintf(stderr,"couldn't initialize SDL:%s\n",SDL_GetError());
		return(NULL);
	}
	atexit(SDL_Quit);

	ScreenSurface = SDL_SetVideoMode(320, 480, bpp, flags);
  p = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, bpp, 0, 0, 0, 0);
	if(p==NULL){
		fprintf(stderr,"couldn't set %dx%dx%d video mode: %s\n",w,h,bpp,SDL_GetError());
		return(NULL);
	}
	//erase mouse cursor
	SDL_ShowCursor(0);


	return(p);

}

//assuming 'this' is already allocated.
int CBmpsLoadFromFileWithDir(CBmps *this,char *filename,char *dir){

	FILE *fp;
	char buf[4096],filenamebuf[4096];
	char *ignore="#\r\n";
	int index;
	char * p;
	Uint32 trans;//for black as transparent color

	if(this==NULL){
		fprintf(stderr,"ERROR:must allocate CBmps before CBmpsLoadFromFile()\n");
		return(-1);
	}

	fp=fopen(filename,"rb");
	if(NULL==fp){
		fprintf(stderr,"ERROR:cant open %s\n",filename);
		return(-1);
	}
	do{
		if(NULL==(fgets(buf,4096,fp)))break;
		if(NULL!=strchr(ignore,*buf))continue;

		p=strtok(buf,"\r\t\n #");
		index=atoi(p);
		p=strtok(NULL,"\r\t\n #");
//		sprintf(filenamebuf,"%s%s",env->bitmapsdir,p);
		sprintf(filenamebuf,"%s%s",dir,p);
		if(CBmpsLoadWzNum(this,filenamebuf,index))continue;
		p=strtok(NULL,"\r\t\n #");
		if(NULL==p){//if more character exists,doesn't set transparent color
			trans=SDL_MapRGB(this->bmp[index]->format,0,0,0);
//			SDL_SetColorKey(this->bmp[index],SDL_SRCCOLORKEY|SDL_RLEACCEL,trans);
			SDL_SetColorKey(this->bmp[index],SDL_SRCCOLORKEY,trans);
		}

	}while(1);
	fclose(fp);
	return 0;

}


int CBmpsBlit(CBmps *this,SDL_Surface *dest,int num,int x,int y){

	SDL_Surface *p;
	SDL_Rect dr;

	p=this->bmp[num];
	if(p==NULL)return(-1);

	dr.w=p->w;
	dr.h=p->h;
	dr.x=x;
	dr.y=y;

	SDL_BlitSurface(p,NULL,dest,&dr);

	return(0);
}
