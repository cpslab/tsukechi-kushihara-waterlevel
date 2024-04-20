#include <Arduino.h>
#include <SPI.h>
// Need this for the lower level access to set them up.
#include <HardwareSerial.h>

//Define two Serial devices mapped to the two internal UARTs
HardwareSerial MySerial0(0);

int PORTLATE = 57600;
int BIGTIMEOUT = 10000;
int POSTTIMEOUT = 60000;
int NORMALTIMEOUT = 5000;
int SMALLTIMEOUT = 1000;

int count = 0;
float distance;

bool sendATCommand(const char* command , const int timeout) {
  MySerial0.write(command);
  MySerial0.flush();
  delay(5000);  // 応答を待つための適切な遅延を設定

  while (Serial1.available()) {
    char response = Serial1.read();
    Serial.print(response);

    // エラーチェック
    if (strstr(&response, "ERROR") != NULL) {
      return false;  // エラーが検出された場合
    }
  }

  return true;  

}

bool sendBody(const char* command ) {
  MySerial0.write("AT+SHBOD=1024,10000\r\n"); 
  MySerial0.flush();
  delay(3000);
  MySerial0.write(command);
  MySerial0.flush();
  delay(3000);
  unsigned long startTime = millis();
 String response = "";
 String hexString = ""; // 16進数形式のデータを格納するための文字列

 while (millis() - startTime < 5000) {
  if (MySerial0.available()) {
    char c = MySerial0.read();
    char hex[4]; // 16進数変換用の一時バッファ
    //sprintf(hex, "%02X ", (uint8_t)c); // 16進数形式に変換
    hexString += String(hex); // 変換した16進数を追加
    response += c;
  }
}

// エラーチェックとテキスト形式のレスポンスの出力
if (response.indexOf("ERROR") != -1) {
  Serial.println("Error in response");
  return false;
} else {
  Serial.print("Received Text: ");
  Serial.println(response);
  Serial.flush();
  return true;
}

}


void serial_send(float distance) {


  if (!sendATCommand("AT+CFUN=6\r\n",NORMALTIMEOUT)) {
    Serial.println("Error: AT+CFUN=6");
    return;
  }
  delay(BIGTIMEOUT);

  if (!sendATCommand("AT+CGDCONT=1,\"IP\",\"soracom.io\"\r\n",NORMALTIMEOUT)) {
    Serial.println("Error: AT+CGDCONT=1");
    return;
  }
  delay(SMALLTIMEOUT);
   //for (int i = 0; i <= 5; i++) {
    if (!sendATCommand("AT+CNACT=0,1\r\n",BIGTIMEOUT)) {
      Serial.println("Error: AT+CNACT");
      delay(1000);
      
      return;
  }
  delay(BIGTIMEOUT);

  if (!sendATCommand("AT+SHCONF=\"URL\",\"http://harvest.soracom.io\"\r\n",NORMALTIMEOUT)) {
    Serial.println("Error: AT+SHCONF URL");
    return;
  }
  delay(SMALLTIMEOUT);
  if (!sendATCommand("AT+CPSI?",NORMALTIMEOUT)) {
    Serial.println("Error: AT+CPSI?");
    return;
  }
  delay(SMALLTIMEOUT);



  if (!sendATCommand("AT+SHCONF=\"BODYLEN\",1024\r\n",NORMALTIMEOUT)) {
    Serial.println("Error: AT+SHCONF BODYLEN");
    return;
  }
  delay(SMALLTIMEOUT);

  if (!sendATCommand("AT+SHCONF=\"HEADERLEN\",350\r\n",NORMALTIMEOUT)) {
    Serial.println("Error: AT+SHCONF HEADERLEN");
    return;
  }
  delay(SMALLTIMEOUT);

  if (!sendATCommand("AT+SHCONN\r\n",NORMALTIMEOUT)) {
    Serial.println("Error: AT+SHCONN");
    return;
  }
  delay(SMALLTIMEOUT);

  if (!sendATCommand("AT+SHAHEAD=\"Content-Type\",\"application/json\"\r\n",NORMALTIMEOUT)) {
    Serial.println("Error: AT+SHAHEAD");
    return;
  }
  delay(SMALLTIMEOUT);

  

  String distance_json = "\"distance\":" + String(distance);
  String All_data = "{" + distance_json +"}\r\n";

// if (!sendATCommand("AT+SHBOD=1024,10000\r\n")) {
//     Serial.println("Error: AT+SHBOD");
//     return;
//   }
//   delay(15000);
//   if (!sendATCommand(All_data.c_str())) {
//     Serial.println("Error: JSON Data");
//     return;
//   }

  if (!sendBody(All_data.c_str())) {
    Serial.println("Error: JSON Data");
    return;
  }
  delay(SMALLTIMEOUT);

  if (!sendATCommand("AT+SHREQ=\"http://harvest.soracom.io\",3\r\n",POSTTIMEOUT)) {
    Serial.println("Error: AT+SHREQ");
    return;
  }
  delay(NORMALTIMEOUT);

  if (!sendATCommand("AT+SHDISC\r\n",NORMALTIMEOUT)) {
    Serial.println("Error: AT+SHDISC");
    return;
  }

  Serial.println("done");
}

void setup() {
  Serial.begin(PORTLATE);
  // Configure MySerial0 on pins TX=6 and RX=7 (-1, -1 means use the default)
 MySerial0.begin(PORTLATE, SERIAL_8N1, -1, -1);
  count = 0;
}

void loop() {
  delay(5000);
  Serial.println("start");
  delay(1000);
  Serial.println("stand by");
    serial_send(10);
    delay(15000);
    count = 0;
}