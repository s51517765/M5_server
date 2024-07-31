#include <M5Stack.h> //0.3.9
#include <WiFi.h>
#include <HTTPClient.h>
#include "auth.h"
#include <string.h>

#define REFLASH_MS 100
#define TRI_POSI_Y 120
#define TRI_SIZE 30

#define LED_PIN 16
#define GND_PIN 17

enum
{
  STOP = 0,
  MONITORING,
  CALLING,
  NET_ERROR
};

uint8_t state = MONITORING;

void setup()
{
  Serial.begin(115200);
  M5.begin(true, false, true);
  M5.Power.begin();
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    M5.Lcd.printf(".");
    Serial.print(".");
  }
  // GPIOの初期化
  pinMode(LED_PIN, OUTPUT);
  pinMode(GND_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(GND_PIN, LOW);

  M5.Lcd.println(" CONNECTED");
}

uint8_t getState(String html, uint8_t len)
{
  char result[len];
  html.toCharArray(result, len);
  char *pattern_calling = "CALLING";
  int i = 0;
  int j;

  while (result[i] != NULL)
  {
    j = 0;
    while (result[i] == pattern_calling[j])
    {
      j++;
      i++;
      if (j == sizeof(pattern_calling) / sizeof(char))
      {
        return CALLING;
      }
    }
    i++;
  }

  char *pattern_stop = "STOP"; // STOP表示の時はACTIVE
  i = 0;
  while (result[i] != NULL)
  {
    j = 0;
    while (result[i] == pattern_stop[j])
    {
      j++;
      i++;
      if (j == sizeof(pattern_stop) / sizeof(char))
      {
        return MONITORING;
      }
    }
    i++;
  }

  return STOP;
}

void callHTTP()
{
  HTTPClient http;
  http.begin("http://172.16.80.140/");
  int httpCode = http.GET();

  if (httpCode == 200)
  {
    Serial.println("Connecting Server OK.");

    String html = http.getString();
    int len = html.length();
    state = getState(html, len);
  }
  else
  {
    Serial.print("\nCode = ");
    Serial.println(httpCode);
    state = NET_ERROR;
  }
  http.end();
}
void call()
{
  digitalWrite(LED_PIN, HIGH);
  delay(2000);
  digitalWrite(LED_PIN, LOW);
  state = MONITORING;
}

void showProgress()
{
  for (int i = 0; i < 10; i++)
  {
    int x = i * TRI_SIZE;
    M5.Lcd.fillTriangle(0 + x, TRI_POSI_Y, 0 + x, TRI_POSI_Y + TRI_SIZE, TRI_SIZE + x, TRI_POSI_Y + TRI_SIZE / 2, GREEN);
    delay(REFLASH_MS);
  }
}
void loop()
{
  callHTTP();
  switch (state)
  {
  case STOP:
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.print("STOP");
    delay(1000);
    break;
  case MONITORING:
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.print("Monitoring... ");
    showProgress();
    break;
  case CALLING:
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.print("Calling... ");
    call();
    break;
  case NET_ERROR:
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.print("Connecting Server Error!");
    M5.Lcd.setTextColor(WHITE);
    break;
  default:
    break;
  }
}
