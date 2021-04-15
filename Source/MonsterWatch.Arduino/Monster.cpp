//
//
//

#include <M5StickCPlus.h>

#include "Monster.h"
#include "MonsterAction.h"
#include "Idle.h"

Monster::Monster(TFT_eSprite *tftSprite)
{
  this->index = 0;
  this->action = Idle;
  this->tftSprite = tftSprite;
}

void Monster::Execution()
{
  unsigned long now = millis();
  if ((now - this->lastExecutionTime) > 250)
  {
    this->tftSprite->pushImage(0, 0, 48, 62, aaa[this->index]);
    this->tftSprite->pushSprite(175, 65);
    this->lastExecutionTime = now;
    this->index++;
    if (this->index == frames)
      this->index = 0;
  }
}

void Monster::SetAction(int action)
{

}
