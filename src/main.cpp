#include <Arduino.h>
#include <SPI.h>
#include "driver/gpio.h"
// Need this for the lower level access to set them up.
#include "esp_bt_main.h"
#include "esp_bt.h"
#include "esp_wifi.h"
const int ledPin = 2; // Xiao C3のGPIO4ピンを使用
RTC_DATA_ATTR int counter = 0;  //RTC coprocessor領域に変数を宣言することでスリープ復帰後も値が保持できる

void esp32c3_deepsleep(uint8_t sleep_time, uint8_t wakeup_gpio) {
  // スリープ前にwifiとBTを明示的に止めないとエラーになる
  esp_bluedroid_disable();
  esp_bt_controller_disable();
  esp_wifi_stop();
  esp_deep_sleep_enable_gpio_wakeup(BIT(wakeup_gpio), ESP_GPIO_WAKEUP_GPIO_HIGH);  // 設定したIOピンがHIGHになったら目覚める
  esp_deep_sleep(1000 * 1000 * sleep_time);
}

void wakeup_cause_print() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case 0: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_UNDEFINED"); break;
    case 1: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_ALL"); break;
    case 2: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_EXT0"); break;
    case 3: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_EXT1"); break;
    case 4: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_TIMER"); break;
    case 5: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_TOUCHPAD"); break;
    case 6: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_ULP"); break;
    case 7: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_GPIO"); break;
    case 8: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_UART"); break;
    case 9: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_WIFI"); break;
    case 10: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_COCPU"); break;
    case 11: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG"); break;
    case 12: Serial.println("Wakeup caused by ESP_SLEEP_WAKEUP_BT"); break;
    default: Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}

void setup() {
  pinMode(ledPin, OUTPUT); // ピンを出力として設定
  digitalWrite(ledPin, HIGH);
  Serial.begin(57600);
  Serial.printf("Cause:%d\r\n", esp_sleep_get_wakeup_cause());
  wakeup_cause_print();
  //Serial.printf("Counter:%d\r\n", counter++);
  Serial.println("Start!!");
  for (int i = 0; i < 15; i++) {
    Serial.printf("%d analog :%d\n", i, analogRead(0));
    Serial.printf("Counter:%d\r\n", counter++);
    delay(1000);
  }
  Serial.println("Go to DeepSleep!!");
  digitalWrite(ledPin, LOW); // LEDをOFFにする
  esp32c3_deepsleep(10, 2);  //10秒間スリープ スリープ中にGPIO2がHIGHになったら目覚める
}

void loop() {
  // put your main code here, to run repeatedly:
}



