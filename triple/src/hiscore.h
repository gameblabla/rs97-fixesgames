#ifndef _HISCORE_H_
#define _HISCORE_H_

extern int hiscore[5];
extern int topHiscore;
extern int toTitleTimer;
extern int scoreUncoverTimer;

void hiscoreUnload();
void hiscoreLoad();
void hiscoreUpdate(int curScore);
void hiscoreLogic();
void hiscoreDraw();

#endif /* _HISCORE_H_ */
