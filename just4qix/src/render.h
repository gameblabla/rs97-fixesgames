#define screen_width 	320
#define screen_height 	240


SDL_Surface* Load(const char* File);
bool Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y);
bool DrawTiled(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int W, int H, int OX, int OY);
bool InitVideo();
bool InitGfx();
void Render();
void CleanUpVideo();