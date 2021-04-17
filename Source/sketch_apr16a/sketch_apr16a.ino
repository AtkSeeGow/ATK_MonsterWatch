#include <Arduino.h>
#include <M5StickCPlus.h>
#include <utility/MPU6886.h>
#include <driver/rtc_io.h>

void mpu6886_wake_on_motion_isr(void); // declaration of ISR
void mpu6886_wake_on_motion_setup(void); // declaration of setup

void MPU6886_I2C_Read_NBytes(uint8_t start_Addr, uint8_t number_Bytes, uint8_t *read_Buffer) {
  Wire1.beginTransmission(MPU6886_ADDRESS);
  Wire1.write(start_Addr);
  Wire1.endTransmission(false);
  uint8_t i = 0;
  Wire1.requestFrom(MPU6886_ADDRESS, (int)number_Bytes);

  while (Wire1.available()) {
    read_Buffer[i++] = Wire1.read();
  }
}

void MPU6886_I2C_Write_NBytes(uint8_t start_Addr, uint8_t number_Bytes, uint8_t *write_Buffer) {
  Wire1.beginTransmission(MPU6886_ADDRESS);
  Wire1.write(start_Addr);
  Wire1.write(*write_Buffer);
  Wire1.endTransmission();
}

#ifndef MPU6886_INT_STATUS
#define MPU6886_INT_STATUS 0x3A
#endif


#define STEP1_PWR_MGMT_1_CYCLE_SLEEP_GYRO_STANDBY_000(r) (r & 0x8F) // zero bits 6,5,4 of 7:0
#define STEP1_PWR_MGMT_2_STBY_XYZ_A_000_G_111 0x07 // zero bits 5,4,3 and one bits 2,1,0 of 5:0

#define STEP2_ACCEL_CONFIG2_FCHOICE_B_0_DLPF_CFG_001(r) (0x21) // average 32 samples, use 218 Hz DLPF

#define STEP2P5_INT_PIN_CFG_ACTIVE_LOW_NO_LATCH(r) ((r | 0x88) & 0xDF)

#define STEP3_INT_ENABLE_WOM_INT_EN_111 0xE0 // one bits 7,6,5, zero bits 4,3,2,1,0 of 7:0

#define STEP5_ACCEL_INTEL_CTRL_INTEL_EN_1_MODE_1_WOM_TH_MODE_0 0xC2 // one bits 7,6,1 of 7:0

#define STEP8_PWR_MGMT_1_CYCLE_1(r) (r | 0x20)

void mpu6886_wake_on_motion_setup(uint8_t num_lsb_at_16G_FSR) {
  uint8_t regdata;
  regdata = (M5.Imu.Ascale::AFS_16G << 3);
  MPU6886_I2C_Write_NBytes(MPU6886_ACCEL_CONFIG, 1, &regdata);
  delay(10);
  MPU6886_I2C_Read_NBytes(MPU6886_PWR_MGMT_1, 1, &regdata);
  ESP_LOGD("WOM", "1A: MPU6886_PWR_MGMT_1 = 0x%02X", regdata);
  regdata = STEP1_PWR_MGMT_1_CYCLE_SLEEP_GYRO_STANDBY_000(regdata);
  ESP_LOGD("WOM", "1B: MPU6886_PWR_MGMT_1 = 0x%02X", regdata);
  MPU6886_I2C_Write_NBytes(MPU6886_PWR_MGMT_1, 1, &regdata);

  // DEBUG READ
  MPU6886_I2C_Read_NBytes(MPU6886_PWR_MGMT_1, 1, &regdata);
  ESP_LOGD("WOM", "1C: MPU6886_PWR_MGMT_1 = 0x%02X", regdata);

  regdata = STEP1_PWR_MGMT_2_STBY_XYZ_A_000_G_111;
  ESP_LOGD("WOM", "1D: MPU6886_PWR_MGMT_2 = 0x%02X", regdata);
  MPU6886_I2C_Write_NBytes(MPU6886_PWR_MGMT_2, 1, &regdata);

  // DEBUG READ
  MPU6886_I2C_Read_NBytes(MPU6886_PWR_MGMT_2, 1, &regdata);
  ESP_LOGD("WOM", "1E: MPU6886_PWR_MGMT_2 = 0x%02X\n", regdata);


  /* Step 2: Set Accelerometer LPF bandwidth to 218.1 Hz
      • In ACCEL_CONFIG2 register (0x1D) set ACCEL_FCHOICE_B = 0 and A_DLPF_CFG[2:0] = 1 (b001)
  */
  MPU6886_I2C_Read_NBytes(MPU6886_ACCEL_CONFIG2, 1, &regdata);
  ESP_LOGD("WOM", "2A: MPU6886_ACCEL_CONFIG2 = 0x%02X", regdata);
  regdata = STEP2_ACCEL_CONFIG2_FCHOICE_B_0_DLPF_CFG_001(regdata);
  ESP_LOGD("WOM", "2B: MPU6886_ACCEL_CONFIG2 = 0x%02X", regdata);
  MPU6886_I2C_Write_NBytes(MPU6886_ACCEL_CONFIG2, 1, &regdata);

  // DEBUG READ
  MPU6886_I2C_Read_NBytes(MPU6886_ACCEL_CONFIG2, 1, &regdata);
  ESP_LOGD("WOM", "2C: MPU6886_ACCEL_CONFIG2 = 0x%02X\n", regdata);


  /* Step 2.5 - active low? */
  MPU6886_I2C_Read_NBytes(MPU6886_INT_PIN_CFG, 1, &regdata);
  ESP_LOGD("WOM", "2.5A: MPU6886_INT_PIN_CFG = 0x%02X", regdata);
  regdata = STEP2P5_INT_PIN_CFG_ACTIVE_LOW_NO_LATCH(regdata);
  ESP_LOGD("WOM", "2.5B: MPU6886_INT_PIN_CFG = 0x%02X", regdata);
  MPU6886_I2C_Write_NBytes(MPU6886_INT_PIN_CFG, 1, &regdata);

  // DEBUG READ
  MPU6886_I2C_Read_NBytes(MPU6886_INT_PIN_CFG, 1, &regdata);
  ESP_LOGD("WOM", "2.5C: MPU6886_INT_PIN_CFG = 0x%02X\n", regdata);

  /* Step 3: Enable Motion Interrupt
      • In INT_ENABLE register (0x38) set WOM_INT_EN = 111 to enable motion interrupt
  */
  regdata = STEP3_INT_ENABLE_WOM_INT_EN_111;
  ESP_LOGD("WOM", "3A: MPU6886_INT_ENABLE = 0x%02X", regdata);
  MPU6886_I2C_Write_NBytes(MPU6886_INT_ENABLE, 1, &regdata);

  // DEBUG READ
  MPU6886_I2C_Read_NBytes(MPU6886_INT_ENABLE, 1, &regdata);
  ESP_LOGD("WOM", "3B: MPU6886_INT_ENABLE = 0x%02X\n", regdata);

  /* Step 4: Set Motion Threshold
      • Set the motion threshold in ACCEL_WOM_THR register (0x1F)
  */
  regdata = num_lsb_at_16G_FSR;
  ESP_LOGD("WOM", "4A: num_lsb_at_16G_FSR = 0x%02X", regdata);

  MPU6886_I2C_Write_NBytes(0x20, 1, &regdata);
  MPU6886_I2C_Read_NBytes(0x20, 1, &regdata);
  ESP_LOGD("WOM", "4B: 0x20(XTHR) = 0x%02X", regdata);

  MPU6886_I2C_Write_NBytes(0x21, 1, &regdata);
  MPU6886_I2C_Read_NBytes(0x21, 1, &regdata);
  ESP_LOGD("WOM", "4C: 0x21(YTHR) = 0x%02X", regdata);

  MPU6886_I2C_Write_NBytes(0x22, 1, &regdata);
  MPU6886_I2C_Read_NBytes(0x22, 1, &regdata);
  ESP_LOGD("WOM", "4D: 0x22(ZTHR) = 0x%02X\n", regdata);

  /* Step 5: Enable Accelerometer Hardware Intelligence
      • In ACCEL_INTEL_CTRL register (0x69) set ACCEL_INTEL_EN = ACCEL_INTEL_MODE = 1;
        Ensure that bit 0 is set to 0
  */
  regdata = STEP5_ACCEL_INTEL_CTRL_INTEL_EN_1_MODE_1_WOM_TH_MODE_0;
  ESP_LOGD("WOM", "5A: MPU6886_ACCEL_INTEL_CTRL = 0x%02X", regdata);
  MPU6886_I2C_Write_NBytes(MPU6886_ACCEL_INTEL_CTRL, 1, &regdata);
  MPU6886_I2C_Read_NBytes(MPU6886_ACCEL_INTEL_CTRL, 1, &regdata);
  ESP_LOGD("WOM", "5B: MPU6886_ACCEL_INTEL_CTRL = 0x%02X\n", regdata);

  /* Step 7: Set Frequency of Wake-Up
      • In SMPLRT_DIV register (0x19) set SMPLRT_DIV[7:0] = 3.9 Hz – 500 Hz
  */
  // sample_rate = 1e3 / (1 + regdata)
  //   4.0 Hz = 1e3 / (1 + 249)
  //  10.0 Hz = 1e3 / (1 +  99)
  //  20.0 Hz = 1e3 / (1 +  49)
  //  25.0 Hz = 1e3 / (1 +  39)
  //  50.0 Hz = 1e3 / (1 +  19)
  // 500.0 Hz = 1e3 / (1 +   1)
  regdata = 19; //
  ESP_LOGD("WOM", "7A: MPU6886_SMPLRT_DIV = 0x%02X", regdata);
  MPU6886_I2C_Write_NBytes(MPU6886_SMPLRT_DIV, 1, &regdata);
  MPU6886_I2C_Read_NBytes(MPU6886_SMPLRT_DIV, 1, &regdata);
  ESP_LOGD("WOM", "7B: MPU6886_SMPLRT_DIV = 0x%02X\n", regdata);
  ESP_LOGD("WOM", "7C: effective sample rate = 0x%d\n", 1000 / (1 + regdata));

  /* Step 8: Enable Cycle Mode (Accelerometer Low-Power Mode)
      • In PWR_MGMT_1 register (0x6B) set CYCLE = 1
  */
  MPU6886_I2C_Read_NBytes(MPU6886_PWR_MGMT_1, 1, &regdata);
  ESP_LOGD("WOM", "8A: MPU6886_PWR_MGMT_1 = 0x%02X", regdata);
  regdata = STEP8_PWR_MGMT_1_CYCLE_1(regdata);
  ESP_LOGD("WOM", "8B: MPU6886_PWR_MGMT_1 = 0x%02X", regdata);
  MPU6886_I2C_Write_NBytes(MPU6886_PWR_MGMT_1, 1, &regdata);
  MPU6886_I2C_Read_NBytes(MPU6886_PWR_MGMT_1, 1, &regdata);
  ESP_LOGD("WOM", "8C: MPU6886_PWR_MGMT_1 = 0x%02X\n", regdata);
}

#define WOM_ATTACH_ISR
volatile uint32_t g_wom_count = 0;
volatile uint32_t g_wom_last_millis = 0;
void IRAM_ATTR mpu6886_wake_on_motion_isr(void) {
  g_wom_count++;
  g_wom_last_millis = millis();
  // ESP_LOGE("ISR", "UNSAFE DEBUG!");
}


#define GPIO_35_PIN_BITMASK 0x800000000 // 2^35 in hex
RTC_DATA_ATTR int bootCount = 0;

/*
  Method to print the reason by which ESP32
  has been awaken from sleep
*/
void get_wakeup_reason_string(char *cbuf, int cbuf_len) {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0     : snprintf(cbuf, cbuf_len, "Wakeup caused by external signal using RTC_IO");         break;
    case ESP_SLEEP_WAKEUP_EXT1     : snprintf(cbuf, cbuf_len, "Wakeup caused by external signal using RTC_CNTL");       break;
    case ESP_SLEEP_WAKEUP_TIMER    : snprintf(cbuf, cbuf_len, "Wakeup caused by timer");                                break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : snprintf(cbuf, cbuf_len, "Wakeup caused by touchpad");                             break;
    case ESP_SLEEP_WAKEUP_ULP      : snprintf(cbuf, cbuf_len, "Wakeup caused by ULP program");                          break;
    default                        : snprintf(cbuf, cbuf_len, "Wakeup was not caused by deep sleep: %d", wakeup_reason); break;
  }
}

#define WAKE_REASON_BUF_LEN 100
void setup() {
  // esp_err_t ret;
  rtc_gpio_deinit(GPIO_NUM_35);
  char wake_reason_buf[WAKE_REASON_BUF_LEN];

  // put your setup code here, to run once:
  M5.begin();
  M5.Axp.ScreenBreath(8);
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(20,  0); M5.Lcd.println("WAKE-ON-MOTION + ");
  M5.Lcd.setCursor(20, 10); M5.Lcd.println("DEEP-SLEEP TEST:");
  M5.Lcd.setCursor(20, 30); M5.Lcd.printf("BOOT COUNT = %d", bootCount);
  get_wakeup_reason_string(wake_reason_buf, WAKE_REASON_BUF_LEN);
  M5.Lcd.setCursor( 0, 50);
  M5.Lcd.printf(wake_reason_buf);

#ifdef WOM_GPIO_DEBUG_TEST
  pinMode(GPIO_NUM_26, OUTPUT);
  pinMode(GPIO_NUM_36, INPUT);
#endif // #ifdef WOM_GPIO_DEBUG_TEST


#ifdef WOM_ATTACH_ISR
  // set up ISR to trigger on GPIO35
  rtc_gpio_deinit(GPIO_NUM_35);
  delay(100);
  pinMode(GPIO_NUM_35, INPUT);
  delay(100);
  attachInterrupt(GPIO_NUM_35, mpu6886_wake_on_motion_isr, FALLING);
#endif // #ifdef WOM_ATTACH_ISR


  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  // set up mpu6886 for low-power operation
  M5.IMU.Init(); // basic init
  mpu6886_wake_on_motion_setup(10);


  // wait until IMU ISR hasn't triggered for X milliseconds
  while (1) {
    noInterrupts();
    uint32_t since_last_wom_millis = millis() - g_wom_last_millis;
    if (since_last_wom_millis > 5000) {
      break;
    }
    interrupts();
    ESP_LOGI("", "waiting : %d", since_last_wom_millis);
    delay(1000);
  }

  // disable all wakeup sources
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  // enable waking up on pin 35 (from IMU)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, LOW); //1 = High, 0 = Low

  //Go to sleep now
  ESP_LOGI("", "Going to sleep now");
  M5.Axp.SetSleep(); // conveniently turn off screen, etc.
  delay(100);
  rtc_gpio_init(GPIO_NUM_35); // initialize rtc_gpio pin for waking up
  esp_deep_sleep_start();
  ESP_LOGI("", "This will never be printed");


}

void loop() {
  uint8_t regdata;
  static uint8_t latch = 0;
  static uint32_t wom_count = 0;
  // static uint32_t last_dwrite_millis = 0;
  static uint32_t last_wom_millis = 0;
  uint32_t now_millis = millis();

  M5.Lcd.setCursor(20, 20);
  M5.Lcd.printf("%d", now_millis / 1000);

  if (wom_count != g_wom_count ) {
    // clear interrupt status register by reading it
    MPU6886_I2C_Read_NBytes(MPU6886_INT_STATUS, 1, &regdata);

    // update wom_count and last_wom_millis
    wom_count = g_wom_count;
    last_wom_millis = now_millis;
    latch = 1;

    // display wom_count
    M5.Lcd.setCursor(20, 40);
    M5.Lcd.printf("%d", wom_count);

    M5.Lcd.fillRect(70, 20, 50, 50, RED);
  }
  else if (now_millis - last_wom_millis > 2000) {
    if (latch) {
      M5.Lcd.fillRect(70, 20, 50, 50, BLUE);
      M5.Lcd.fillRect(75, 25, 40, 40, BLACK);
      latch = 0;
    }
  }

  delay(50);
}
