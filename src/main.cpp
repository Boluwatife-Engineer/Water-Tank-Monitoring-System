#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <FirebaseClient.h>
#include <time.h>

#include "analytics.h"
#include "secrets.h"
#include "email.h"

const int L1 = 21;
const int L2 = 22;
const int L3 = 23;
const int L4 = 4;

WiFiClientSecure ssl;
AsyncClientClass aClient(ssl);

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;
RealtimeDatabase Database;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int lastLevel = -1;

/* EMAIL TIMER */
unsigned long lastEmailTime = 0;
const unsigned long EMAIL_INTERVAL = 2 * 60 * 1000; // test interval

int stableRead(int pin)
{
  int low = 0;

  for (int i = 0; i < 10; i++)
  {
    if (digitalRead(pin) == LOW) low++;
    delay(5);
  }

  return (low >= 7) ? LOW : HIGH;
}

int readLevel()
{
  if (stableRead(L4) == LOW) return 100;
  if (stableRead(L3) == LOW) return 75;
  if (stableRead(L2) == LOW) return 50;
  if (stableRead(L1) == LOW) return 25;
  return 0;
}

void setup()
{
  Serial.begin(115200);

  pinMode(L1, INPUT_PULLUP);
  pinMode(L2, INPUT_PULLUP);
  pinMode(L3, INPUT_PULLUP);
  pinMode(L4, INPUT_PULLUP);

  if (!LittleFS.begin(true))
  {
    Serial.println("LittleFS mount failed");
    return;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("WiFi connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  configTime(0, 0, "pool.ntp.org");

  ssl.setInsecure();

  initializeApp(
    aClient,
    app,
    getAuth(user_auth),
    nullptr,
    "auth"
  );

  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);

  initEmail();

  ws.onEvent([](AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType type, void*, uint8_t*, size_t)
  {
    if (type == WS_EVT_CONNECT)
    {
      Serial.println("WebSocket connected");
    }
  });

  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(LittleFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(LittleFS, "/script.js", "application/javascript");
  });

  server.begin();

  Serial.println("System ready");
}

void loop()
{
  app.loop();

  static unsigned long t = 0;

  if (millis() - t >= 1000)
  {
    t = millis();

    int level = readLevel();

    updateAnalytics(level);

    ws.textAll(String("{\"level\":") + level + "}");

    if (level != lastLevel)
    {
      lastLevel = level;

      time_t now = time(nullptr);
      String path = "/tank/logs/" + String(now);

      Database.set<int>(aClient, "/tank/current", level, nullptr, "cur");
      Database.set<int>(aClient, path + "/level", level, nullptr, "log");

      Database.set<int>(aClient, "/tank/analytics/highest", getHighestLevel(), nullptr, "h");
      Database.set<int>(aClient, "/tank/analytics/lowest", getLowestLevel(), nullptr, "l");
      Database.set<float>(aClient, "/tank/analytics/average", getAverageLevel(), nullptr, "a");
      Database.set<int>(aClient, "/tank/analytics/changes", getChangeCount(), nullptr, "c");

      Serial.println("Firebase updated");
    }

    if (millis() - lastEmailTime >= EMAIL_INTERVAL)
    {
      lastEmailTime = millis();

      Serial.println("Sending email...");

      bool ok = sendDailyEmail(
        getHighestLevel(),
        getLowestLevel(),
        getAverageLevel(),
        getChangeCount()
      );

      Serial.println(ok ? "EMAIL SENT" : "EMAIL FAILED");
    }
  }
}