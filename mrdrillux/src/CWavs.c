#include "CWavs.h"
#include "string.h"

//サウンド用構造体初期化
//引数：最大画像数
static int music_flag=0;
CWavs* CWavsInit(int nums){

	CWavs *p;

	if( nums < 0 )return(NULL);

	p=(CWavs *)malloc(sizeof(CWavs));
	if(NULL==p){
		fprintf(stderr,"NO MEMORY at CWavsInit()\n");
		return(NULL);
	}
	memset(p,0,sizeof(CWavs));

	p->nums=nums;

	p->wav=(Mix_Chunk **)malloc(sizeof(Mix_Chunk *)*p->nums);
	if(p->wav == NULL){
		fprintf(stderr,"NO MEMORY at CWavsInit()\n");
		free(p);
		return(NULL);
	}
	memset(p->wav,0,sizeof(Mix_Chunk *)*p->nums);
	p->musicchannel=-1;

	return(p);
}
//画像用解放
//引数：最大画像数
int CWavsFree(CWavs *this){

	int i;

	//メモリはロード時に確保されるはず
	for(i=0;i<this->nums;++i)
		if(this->wav[i]!=NULL){
			Mix_FreeChunk(this->wav[i]);
			this->wav[i]=NULL;
		}

	if(NULL!=this->wav){
		free(this->wav);
		this->wav=NULL;
	}

	if(NULL!=this){
		free(this);
	}

	return(0);
}
//音楽用データロード
int CWavsLoad(CWavs* this,char *filename){
	return CWavsLoadWzNum(this,filename,this->index);
}
int CWavsLoadWzNum(CWavs* this,char *filename,int num){

	if(this==NULL)return(-1);
	this->index=num;
	if(this->index==this->nums){
		fprintf(stderr,"ERROR:excess CWavs.nums : %d\n",num);
		return(-1);
	}

	this->wav[this->index]=Mix_LoadWAV(filename);
	if(NULL==this->wav[this->index]){
		fprintf(stderr,"ERROR:NO MEMORY or cant find %s at CWavsLoad()\n",filename);
		return(-1);
	}
	this->index++;
	return(0);
}
int CAudioInitDefault(void){

	return CAudioInit(22050,AUDIO_S16,2,512);

}

/*
サウンドを開く
開けたら１を返す。そうでない場合0を返す
*/
int CAudioInit(int rate,int format,int channels,int buffers){

	if (Mix_OpenAudio(rate, format, channels, buffers) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());

		return(0);
	}
	music_flag=1;
	//こんなことしていい？
//	Mix_AllocateChannels(24);
//	Mix_ReserveChannels(24);
	return(1);

}
int CAudioClose(void){

	if(music_flag==1){
		Mix_CloseAudio();
	music_flag=0;
		return(0);
	}
	return(-1);
}

//assuming 'this' is already allocated.
int CWavsLoadFromFileWithDir(CWavs *this,char *filename,char *dir){

	FILE *fp;
	char buf[4096],filenamebuf[4096];
	char *ignore="#\r\n";
	int index;
	char * p;

	if(this==NULL){
		fprintf(stderr,"ERROR:must allocate CWavs before CWavsLoadFromFileWithDir()\n");
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
		sprintf(filenamebuf,"%s%s",dir,p);
		CWavsLoadWzNum(this,filenamebuf,index);
		p=strtok(NULL,"\r\t\n #");

	}while(1);
	fclose(fp);
	return 0;

}


void CWavsPlay(CWavs *this,int index){

	Mix_PlayChannel(-1, this->wav[index], 0);


}

void CWavsPlayMusicStyle(CWavs *this,int index){

	if(this->musicchannel!=-1)
//		Mix_FadeOutChannel(this->musicchannel, 1000);
		Mix_HaltChannel(this->musicchannel);
	this->musicchannel=Mix_PlayChannel(-1, this->wav[index], -1);
//	this->musicchannel=Mix_FadeInChannel(-1,this->wav[index], -1,1000);

}

