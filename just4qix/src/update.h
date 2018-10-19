
typedef struct {
	float x;
	float y;
	float dx;
	float dy;
} Henchman;

void update();
Uint32 floodFillScanlineStack(int x, int y, Uint16 newColor, Uint16 oldColor);
static void deleteHenchman(int index);