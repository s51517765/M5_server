#include <WiFi.h>
#include <WebServer.h>
#include "auth.h"

// https://qiita.com/mine820/items/01d5b7dbf65296c6f6e8

#define LED_PIN 18
#define SW_PIN 26
#define GND_PIN 19

enum
{
  STOP = 0,
  MONITORING,
  CALLING
};

WebServer server(80); // ポート80番

// HTMLを組み立てる
const String html_stateMonitoring = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>M5server</title></head><body><p><font size =\"20\">リンクをクリックすると監視状態を変更します</font></p><font size=\"20\"><a href=\"/?button=off\">STOP</a></font></body></html>";
const String html_stateStop = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>M5server</title></head><body><p><font size =\"20\">リンクをクリックすると監視状態を変更します</font></p><font size=\"20\"><a href=\"/?button=on\">START</a></font></body></html>";
const String html_stateCalling = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>M5server</title></head><body><p><font size =\"20\">リンクをクリックすると監視状態を変更します</font></p><font size=\"20\">CALLING</font></body></html>";

String html = html_stateMonitoring;
uint8_t state = STOP;

// 状態の切り替え
void handleState()
{
  if (server.hasArg("button"))
  {
    if (server.arg("button").equals("on"))
    {
      html = html_stateMonitoring;
      state = MONITORING;
    }
    else
    {
      html = html_stateStop;
      state = STOP;
    }
    Serial.println(server.arg("button"));
  }
  server.send(200, "text/html", html);
}

// 存在しないアドレスが指定された時の処理
void handleNotFound(void)
{
  server.send(404, "text/plain", "Not Found.");
}

// 初期化
void setup()
{
  // GPIOの初期化
  pinMode(LED_PIN, OUTPUT);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(GND_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(GND_PIN, LOW);

  // シリアルポートの初期化
  Serial.begin(115200);

  // WiFiのアクセスポイントに接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  // ESP32のIPアドレスを出力
  Serial.println("WiFi Connected.");
  Serial.print("IP = ");
  Serial.println(WiFi.localIP());
  Serial.println(html);

  // このアドレスにアクセスしたときに処理される関数を指定
  server.on("/", handleState);
  server.onNotFound(handleNotFound);
  // Webサーバーを起動
  server.begin();
}

uint32_t t;
void loop()
{
  Serial.println(state);
  server.handleClient();
  if ((state == MONITORING) && (digitalRead(SW_PIN) == LOW))
  {
    digitalWrite(LED_PIN, HIGH);
    html = html_stateCalling;
    state = CALLING;
    t = millis();
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
  }
  if ((millis() - t > 2 * 1000) && (state == CALLING))
  {
    html = html_stateMonitoring;
    state = MONITORING;
  }
}
