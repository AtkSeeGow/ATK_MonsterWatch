// Monster.h

#include <M5StickCPlus.h>

#ifndef _MONSTER_h
#define _MONSTER_h

class Monster {
  public:
    Monster(TFT_eSprite *tftSprite);
    void Execution();
  private:
    int index;
    int type;
    int action;
    float accelX;
    float accelY;
    float accelZ;
    TFT_eSprite *tftSprite;
    unsigned long lastExecutionTime;
    unsigned long lastWalkTime;
};

#endif
