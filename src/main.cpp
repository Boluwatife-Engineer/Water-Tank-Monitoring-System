#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <FirebaseClient.h>
#include "secrets.h"

#define WIFI_SSID WIFI_SSID
#define WIFI_PASSWORD WIFI_PASSWORD
#define API_KEY API_KEY
#define DATABASE_URL DATABASE_URL
#define USER_EMAIL USER_EMAIL
#define USER_PASSWORD USER_PASSWORD


int L1 = 21;
int L2 = 22;
int L3 = 23;
int L4 = 4;

WiFiClientSecure ssl;
AsyncClientClass aClient(ssl);

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;

RealtimeDatabase Database;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int s1, s2, s3, s4;

int level = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(L1, INPUT_PULLUP);
  pinMode(L2, INPUT_PULLUP);
  pinMode(L3, INPUT_PULLUP);
  pinMode(L4, INPUT_PULLUP);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  ssl.setInsecure();

  initializeApp(aClient, app, getAuth(user_auth));

  app.getApp<RealtimeDatabase>(Database);

  Database.url(DATABASE_URL);

  ws.onEvent([](AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType type, void *, uint8_t *, size_t)
             {
               if (type == WS_EVT_CONNECT)
                 Serial.println("WS connected");
             });

  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              request->send(LittleFS, "/index.html", "text/html");
            });

  server.begin();
}

int readLevel()
{
  s1 = digitalRead(L1);
  s2 = digitalRead(L2);
  s3 = digitalRead(L3);
  s4 = digitalRead(L4);

  if (s4 == LOW) return 100;
  if (s3 == LOW) return 75;
  if (s2 == LOW) return 50;
  if (s1 == LOW) return 25;

  return 0;
}

void loop()
{
  app.loop();

  level = readLevel();

  String json = "{\"level\":" + String(level) + "}";

  ws.textAll(json);

  static int last = -1;

  if (level != last)
  {
    last = level;
    Database.set<int>(aClient, "/tank/level", level);
  }

  delay(1000);
}