#include <M5StickCPlus.h>

#include "Idle.h"

TFT_eSprite tft = TFT_eSprite(&M5.Lcd);

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  tft.createSprite(160, 80);
  tft.setRotation(3);
  M5.Axp.EnableCoulombcounter();
}

void loop() {
  tft.fillSprite(BLACK);
  for (int i = 0; i < frames; i++)
  {
    delay(250);
    tft.pushImage(50, 15, animation_width  , animation_height, Idle[i]);
    tft.pushSprite(40, 20);
  }
  M5.update();
}
