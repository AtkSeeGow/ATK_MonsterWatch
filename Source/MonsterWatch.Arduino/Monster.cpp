//
//
//

#include <M5StickCPlus.h>

#include "Type.h"
#include "Monster.h"

Monster::Monster(TFT_eSprite *tftSprite) {
  this->index = 0;
  this->type = Botamon;
  this->action = Idle;
  this->tftSprite = tftSprite;
}

void Monster::Execution() {
  unsigned long now = millis();

  M5.IMU.getAccelData(&this->accelX, &this->accelY, &this->accelZ);
  if (this->accelX > 1.5 || this->accelY > 1.5 )
    this->lastWalkTime = now;

  if ((now - this->lastWalkTime) < 5000)
    this->action = Walk;
  else
    this->action = Idle;

  if ((now - this->lastExecutionTime) > 250) {
    this->tftSprite->pushImage(0, 0, 48, 62, MonsterTypes[this->type][this->action][this->index]);
    this->tftSprite->pushSprite(175, 65);
    this->lastExecutionTime = now;
    this->index++;
    if (this->index == 4)
      this->index = 0;
  }
}
