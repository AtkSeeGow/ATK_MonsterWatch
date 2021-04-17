#include <M5StickCPlus.h>

#include "esp_bt_main.h"
#include "esp_bt.h"
#include "esp_wifi.h"

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

  M5.IMU.Init();

  if (bootCount == 0)
    watch.SyncingTime();
    
  bootCount++;

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, LOW);
  esp_bluedroid_disable();
  esp_bt_controller_disable();
  esp_wifi_stop();
}

void loop()
{
  watch.Execution();
  monster.Execution();

  if (millis() > 30000)
    M5.Axp.DeepSleep(0);

  if (M5.BtnB.wasPressed()){
  }

  M5.update();
}
