#include <Arduino.h>
#include <SPI.h>
#include "driver/gpio.h"
// Need this for the lower level access to set them up.
#include "esp_bt_main.h"
#include "esp_bt.h"
#include "esp_wifi.h"
const int SWITCH_PIN = 2; // Xiao C3のGPIO2ピンを使用
RTC_DATA_ATTR int counter = 0;  //RTC coprocessor領域に変数を宣言することでスリープ復帰後も値が保持できる
const int  SLEEPTIME_SECONDS = 2000; //秒
void esp32c3_deepsleep(uint8_t sleep_time, uint8_t wakeup_gpio) {
  // スリープ前にwifiとBTを明示的に止めないとエラーになる
  esp_bluedroid_disable();
  esp_bt_controller_disable();
  esp_wifi_stop();
  esp_deep_sleep_enable_gpio_wakeup(BIT(wakeup_gpio), ESP_GPIO_WAKEUP_GPIO_HIGH);  // 設定したIOピンがHIGHになったら目覚める
  esp_deep_sleep(1000 * 1000 * sleep_time);
}


void setup() {
  pinMode(SWITCH_PIN, OUTPUT); // ピンを出力として設定
  digitalWrite(SWITCH_PIN, HIGH);
  Serial.begin(57600);
  Serial.println("Go to DeepSleep!!");
  digitalWrite(SWITCH_PIN, LOW); // センサ類をOFFにする
  esp32c3_deepsleep(SLEEPTIME_SECONDS, 2);  //スリープタイム スリープ中にGPIO2がHIGHになったら目覚める
}

void loop() {
  // put your main code here, to run repeatedly:
}



