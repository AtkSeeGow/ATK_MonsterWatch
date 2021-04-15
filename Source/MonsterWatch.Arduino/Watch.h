// Watch.h

#include <M5StickCPlus.h>

#ifndef _WATCH_h
#define _WATCH_h

class Watch {
  public:
    Watch(TFT_eSprite *tftSprite);
    void Execution();
    void SyncingTime();
  private:
    TFT_eSprite *tftSprite;
    RTC_TimeTypeDef timeStruct;
    RTC_DateTypeDef dateStruct;
};

#endif
