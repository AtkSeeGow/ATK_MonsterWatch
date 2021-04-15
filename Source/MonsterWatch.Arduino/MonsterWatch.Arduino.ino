#include <M5StickCPlus.h>

#include "Monster.h"
#include "Watch.h"

RTC_DATA_ATTR int bootCount = 0;

TFT_eSprite tftSprite = TFT_eSprite(&M5.Lcd);

Monster monster = Monster(&tftSprite);
Watch watch = Watch(&tftSprite);

void setup()
{
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextSize(2);
  
  tftSprite.createSprite(160, 80);
  tftSprite.setRotation(3);
  tftSprite.fillSprite(BLACK);

  M5.Axp.ScreenBreath(8);
  M5.Axp.EnableCoulombcounter();

  if (bootCount == 0)
    watch.SyncingTime();
    
  bootCount++;
}

void loop()
{
  watch.Execution();
  monster.Execution();

  if (millis() > 15000)
  {
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, LOW);
    M5.Axp.DeepSleep(0);
  }

  if (M5.BtnB.wasPressed())
  {
  }

  M5.update();
}
