#include <Arduino.h>
#include <SPI.h>
#include "driver/gpio.h"
// Need this for the lower level access to set them up.
#include "esp_bt_main.h"
#include "esp_bt.h"
#include "esp_wifi.h"

// Define two Serial devices mapped to the two internal UARTs
HardwareSerial MySerial0(0);
HardwareSerial MySerial1(1);

const int SWITCH_PIN = 2; // Xiao C3のGPIO2ピンを使用
RTC_DATA_ATTR int counter = 0;  //RTC coprocessor領域に変数を宣言することでスリープ復帰後も値が保持できる
const int  SLEEPTIME_SECONDS = 30; //秒(3600→1時間)
int PORTLATE = 57600;
int BIGTIMEOUT = 10000;
int POSTTIMEOUT = 60000;
int NORMALTIMEOUT = 5000;
int SMALLTIMEOUT = 1000;

unsigned char data[4] = {};

int count;
float distance = -1;

void esp32c3_deepsleep(uint8_t sleep_time, uint8_t wakeup_gpio) {
  // スリープ前にwifiとBTを明示的に止めないとエラーになる
  esp_bluedroid_disable();
  esp_bt_controller_disable();
  esp_wifi_stop();
  esp_deep_sleep_enable_gpio_wakeup(BIT(wakeup_gpio), ESP_GPIO_WAKEUP_GPIO_HIGH);  // 設定したIOピンがHIGHになったら目覚める
  esp_deep_sleep(1000 * 1000 * sleep_time);
}

bool sendATCommand(const char *command, const int timeout)
{
    MySerial0.write(command);
    MySerial0.flush();
    delay(5000); // 応答を待つための適切な遅延を設定

    while (MySerial0.available())
    {
        String response = MySerial0.readStringUntil('\n');
        Serial.println(response);

        // エラーチェック
        if (response.indexOf("ERROR") != -1)
        {
            return false; // エラーが検出された場合
        }
    }

    return true;
}

bool sendBody(const char *command)
{
    MySerial0.write("AT+SHBOD=1024,10000\r\n");
    MySerial0.flush();
    delay(1000);
    MySerial0.write(command);
    MySerial0.flush();
    String response = MySerial0.readStringUntil('\n');
    Serial.println(response);
    response += MySerial0.readStringUntil('\n');
    String temp;
    do
    {
        temp = MySerial0.readStringUntil('\n');
        delay(1000);
        response += temp;

    } while (temp == "OK" || temp == "ERROR" || temp == "");
    delay(3000);

    // エラーチェックとテキスト形式のレスポンスの出力
    if (response.indexOf("ERROR") != -1)
    {
        Serial.println("Error in response");
        return false;
    }
    else
    {
        Serial.println(response);
        return true;
    }
}

void serial_send(float distance)
{

    if (!sendATCommand("AT+CGDCONT=1,\"IP\",\"soracom.io\"\r\n", NORMALTIMEOUT))
    {
        Serial.println("Error: AT+CGDCONT=1");
        return;
    }
    delay(1000);
    if (!sendATCommand("AT+CNACT=0,1\r\n", NORMALTIMEOUT))
    {
        Serial.println("Error: AT+CNACT");
        return;
    }
    delay(5000);

    if (!sendATCommand("AT+SHCONF=\"URL\",\"http://harvest.soracom.io\"\r\n", NORMALTIMEOUT))
    {
        Serial.println("Error: AT+SHCONF URL");
        return;
    }
    delay(1000);

    if (!sendATCommand("AT+SHCONF=\"BODYLEN\",1024\r\n", NORMALTIMEOUT))
    {
        Serial.println("Error: AT+SHCONF BODYLEN");
        return;
    }
    delay(1000);

    if (!sendATCommand("AT+SHCONF=\"HEADERLEN\",350\r\n", NORMALTIMEOUT))
    {
        Serial.println("Error: AT+SHCONF HEADERLEN");
        return;
    }
    delay(1000);

    if (!sendATCommand("AT+SHCONN\r\n", NORMALTIMEOUT))
    {
        Serial.println("Error: AT+SHCONN");
        return;
    }
    delay(1000);

    if (!sendATCommand("AT+SHAHEAD=\"Content-Type\",\"application/json\"\r\n", NORMALTIMEOUT))
    {
        Serial.println("Error: AT+SHAHEAD");
        return;
    }
    delay(1000);

    String distance_json = "\"distance\":" + String(distance);
    String All_data = "{" + distance_json + "}\r\n";

    if (!sendBody(All_data.c_str()))
    {
        Serial.println("Error: JSON Data");
        return;
    }
    delay(2000);

    if (!sendATCommand("AT+SHREQ=\"http://harvest.soracom.io\",3\r\n", POSTTIMEOUT))
    {
        Serial.println("Error: AT+SHREQ");
        return;
    }
    delay(2000);

    if (!sendATCommand("AT+SHDISC\r\n", NORMALTIMEOUT))
    {
        Serial.println("Error: AT+SHDISC");
        return;
    }

    Serial.println("done");
}


void setup() {
  Serial.begin(PORTLATE);
  // Configure MySerial0 on pins TX=6 and RX=7 (-1, -1 means use the default)
  MySerial0.begin(PORTLATE, SERIAL_8N1, -1, -1);
  MySerial1.begin(9600, SERIAL_8N1, 9, 10);
  pinMode(SWITCH_PIN, OUTPUT); // ピンを出力として設定
  digitalWrite(SWITCH_PIN, HIGH);
  distance = -1;
  count = 0;
   if (!sendATCommand("AT+CFUN=6\r\n", NORMALTIMEOUT))
    {
        Serial.println("Error: AT+CFUN=6");
        return;
    }
    delay(3000);
}

void loop() {
  do
    {
        for (int i = 0; i < 4; i++)
        {
            data[i] = MySerial1.read();
        }
    } while (MySerial1.read() == 0xff);

    MySerial1.flush();

    if (data[0] == 0xff)
    {
        int sum;
        sum = (data[0] + data[1] + data[2]) & 0x00FF;
        if (sum == data[3])
        {
            distance = (data[1] << 8) + data[2];
            if (distance > 30)
            {
                Serial.print("distance=");
                Serial.print(distance / 10);
                Serial.println("cm");
            }
            else
            {
                Serial.println("Below the lower limit");
            }
        }
        else
            Serial.println("ERROR");
    }
    delay(100);
    count += 1;

    if (count > 100 || distance != -1)//デバッグで＆から変更
    {
    delay(5000);//シリアルコンソール確認用のdelay(本番では不要)
    Serial.println("start");
    serial_send(distance/10);
    digitalWrite(SWITCH_PIN, LOW); // センサ類をOFFにする
    esp32c3_deepsleep(SLEEPTIME_SECONDS, 2);  //スリープタイム スリープ中にGPIO2がHIGHになったら目覚める
    }
}



