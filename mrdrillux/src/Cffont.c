/*

text put structures for SDL


*/
#include "Cffont.h"
#include "SDL.h"
/*private funcs for init*/
void CffontSetupFromFireworks(Cffont *this,int size){

	int i;
	int x,y;
	SDL_Rect *w;
	int space_x=16,space_y=16,plus=96;

	if(size==8){space_x=31;space_y=31;plus=128;}
	if(size==16){space_x=15;space_y=15;plus=96;}
	if(size==32){space_x=7;space_y=7;plus=64;}


	//printf("%d,%d,%d,%d\n",this->width,this->height,this->font->w,this->font->h);
	x=0;
	y=0;
	w=this->fontarea;
	for(i=0;i<N_ASCII;++i,++w){
		w->w=this->width;
		w->h=this->height;

		if(i<33||i>33+plus){

			w->x=space_x*this->width;
			w->y=space_y*this->height;

		}else{

			w->x=x;
			w->y=y;
			y+=this->height;
			if(y>=this->font->h){
				y=0;
				x+=this->width;
			}
		}
	}

}

//8x8 bitmaped font loading func
Cffont* CffontInitDefault8(char *filename){
	return CffontInit(filename,8,8,0x0ff,0,0x0ff);
}
//16x16 bitmaped font loading func
Cffont* CffontInitDefault16(char *filename){
	return CffontInit(filename,16,16,0x0ff,0,0x0ff);
}
//32x32 bitmaped font loading func
Cffont* CffontInitDefault32(char *filename){
	return CffontInit(filename,32,32,0x0ff,0,0x0ff);
}

Cffont* CffontInit(char *filename,int width,int height,int r,int g,int b){

	Cffont *p;
	Uint32 color;

	p=(Cffont *)malloc(sizeof(Cffont));
	if(NULL==p){
		fprintf(stderr,"ERROR:no memory at CffontInit\n");
		return(NULL);
	}
	memset(p,0,sizeof(Cffont));
	p->font=SDL_LoadBMP(filename);
	if(NULL==p->font){
		fprintf(stderr,"ERROR:NO MEMORY or cant find %s at CffontInit\n",filename);
		free(p);
		return(NULL);
	}
	color=SDL_MapRGB(p->font->format,r,g,b);
	SDL_SetColorKey(p->font,SDL_SRCCOLORKEY,color);
	p->width=width;
	p->height=height;
//	SDL_DisplayFormat(p->font);
	CffontSetupFromFireworks(p,p->width);

	return(p);


}

void CffontFree(Cffont *this){
	if(NULL==this)return;
	if(NULL==this->font)return;
	SDL_FreeSurface(this->font);
	free(this);
}


void CffontBlitxy(Cffont *this,char *string,SDL_Surface *surf,int x,int y){
	SDL_Rect dest = { x, y, this->width, this->height };
	for (; *string; string++) {
		SDL_Rect *psrc = &this->fontarea[(unsigned int)*string];
		SDL_BlitSurface(this->font, psrc, surf, &dest);
		dest.x += dest.w;
	}
}
