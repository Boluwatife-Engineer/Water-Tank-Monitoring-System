#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <FirebaseClient.h>
#include "secrets.h"
#include <time.h>

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

int level = 0;
int lastLevel = -1;


void processData(AsyncResult &aResult)
{
  if (!aResult.isResult()) return;

  if (aResult.isError())
  {
    Firebase.printf(
      "Error: %s, code: %d\n",
      aResult.error().message().c_str(),
      aResult.error().code()
    );
  }

  if (aResult.available())
  {
    Firebase.printf("Result: %s\n", aResult.c_str());
  }
}

int stableRead(int pin)
{
  int lowCount = 0;

  for (int i = 0; i < 10; i++)
  {
    if (digitalRead(pin) == LOW)
      lowCount++;

    delay(5);
  }

  return (lowCount >= 7) ? LOW : HIGH;
}


int readLevel()
{
  int s1 = stableRead(L1);
  int s2 = stableRead(L2);
  int s3 = stableRead(L3);
  int s4 = stableRead(L4);

  if (s4 == LOW) return 100;
  if (s3 == LOW) return 75;
  if (s2 == LOW) return 50;
  if (s1 == LOW) return 25;

  return 0;
}


void setup()
{
  Serial.begin(115200);

  pinMode(L1, INPUT_PULLUP);
  pinMode(L2, INPUT_PULLUP);
  pinMode(L3, INPUT_PULLUP);
  pinMode(L4, INPUT_PULLUP);

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS failed");
    return;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());

  
  configTime(0, 0, "pool.ntp.org");

  time_t now = time(nullptr);
  while (now < 100000)
  {
    delay(100);
    now = time(nullptr);
  }

  Serial.println("Time synced");

  
  ssl.setInsecure();

  initializeApp(
    aClient,
    app,
    getAuth(user_auth),
    processData,
    "authTask"
  );

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
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(LittleFS, "/style.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(LittleFS, "/script.js", "application/javascript");
  });

  server.begin();
}


void loop()
{
  app.loop();

  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate >= 1000)
  {
    lastUpdate = millis();

    int currentLevel = readLevel();

    
    String json = "{\"level\":" + String(currentLevel) + "}";
    ws.textAll(json);

  
    if (app.ready() && currentLevel != lastLevel)
    {
      lastLevel = currentLevel;

      time_t now = time(nullptr);
      String logPath = "/tank/logs/" + String(now);

      
      Database.set<int>(
        aClient,
        "/tank/current",
        currentLevel,
        processData,
        "currentLevel"
      );

      
      Database.set<int>(
        aClient,
        logPath + "/level",
        currentLevel,
        processData,
        "logLevel"
      );

      Serial.println("Logged level change");
    }
  }
}