// Monster.h

#include <M5StickCPlus.h>

#ifndef _MONSTER_h
#define _MONSTER_h

class Monster {
  public:
    Monster(TFT_eSprite *tftSprite);
    void SetAction(int action);
    void Execution();
  private:
    int index;
    int action;
    TFT_eSprite *tftSprite;
    unsigned long lastExecutionTime;
};

#endif
