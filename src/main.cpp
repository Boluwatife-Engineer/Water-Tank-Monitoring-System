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

void processData(AsyncResult &aResult) {
  if (!aResult.isResult()) return;
  if (aResult.isError())
    Firebase.printf("Error: %s, code: %d\n", aResult.error().message().c_str(), aResult.error().code());
  if (aResult.available())
    Firebase.printf("Result: %s\n", aResult.c_str());
}

int readLevel() {
  if (digitalRead(L4) == LOW) return 100;
  if (digitalRead(L3) == LOW) return 75;
  if (digitalRead(L2) == LOW) return 50;
  if (digitalRead(L1) == LOW) return 25;
  return 0;
}

void setup() {
  Serial.begin(115200);

  pinMode(L1, INPUT_PULLUP);
  pinMode(L2, INPUT_PULLUP);
  pinMode(L3, INPUT_PULLUP);
  pinMode(L4, INPUT_PULLUP);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  ssl.setInsecure();

  initializeApp(aClient, app, getAuth(user_auth), processData, "authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);

  ws.onEvent([](AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType type, void *, uint8_t *, size_t) {
    if (type == WS_EVT_CONNECT) Serial.println("WS connected");
  });

  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.begin();
}

void loop() {
  app.loop();

  static unsigned long lastUpdate = 0;
  static int lastLevel = -1;

  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();

    level = readLevel();

    String json = "{\"level\":" + String(level) + "}";
    ws.textAll(json);

    if (app.ready() && level != lastLevel) {
      lastLevel = level;
      Database.set<int>(aClient, "/tank/level", level, processData, "setLevel");
    }
  }
}