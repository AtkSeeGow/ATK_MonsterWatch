//
//
//

#include <M5StickCPlus.h>
#include <WiFi.h>

#include "Watch.h"

Watch::Watch(TFT_eSprite *tftSprite)
{
  this->tftSprite = tftSprite;
}

void Watch::Execution()
{
  M5.Rtc.GetTime(&this->timeStruct);
  M5.Rtc.GetData(&this->dateStruct);
  M5.Lcd.setCursor(20,15);
  M5.Lcd.printf("DATE:%04d/%02d/%02d", this->dateStruct.Year, this->dateStruct.Month, this->dateStruct.Date);
  M5.Lcd.setCursor(20,45);
  M5.Lcd.printf("TIME:%02d:%02d:%02d", this->timeStruct.Hours, this->timeStruct.Minutes, this->timeStruct.Seconds);
  M5.Lcd.setCursor(20,75);
  M5.Lcd.printf("V:%.2fv", M5.Axp.GetBatVoltage());
  M5.Lcd.setCursor(20,105);
  M5.Lcd.printf("I:%.2fma", M5.Axp.GetBatCurrent());
}

void Watch::SyncingTime()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(20, 15);
  M5.Lcd.println("Connecting WiFi");

  WiFi.begin("TP-Link A608", "0986230207");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(20, 15);
  M5.Lcd.println("Connected");

  configTime(28800, 0, "time.google.com");

  struct tm timeInfo;
  if (getLocalTime(&timeInfo)) {
    this->timeStruct.Hours   = timeInfo.tm_hour;
    this->timeStruct.Minutes = timeInfo.tm_min;
    this->timeStruct.Seconds = timeInfo.tm_sec;
    M5.Rtc.SetTime(&this->timeStruct);

    this->dateStruct.WeekDay = timeInfo.tm_wday;
    this->dateStruct.Month = timeInfo.tm_mon + 1;
    this->dateStruct.Date = timeInfo.tm_mday;
    this->dateStruct.Year = timeInfo.tm_year + 1900;
    M5.Rtc.SetData(&this->dateStruct);
  }

  M5.Lcd.fillScreen(BLACK);
}
