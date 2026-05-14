#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

int L1 = 21;
int L2 = 22;
int L3 = 23;
int L4 = 4;

int s1, s2, s3, s4;

const char* ssid = "TIFEH100";
const char* password = "Doyouknowlade^";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket *server,
               AsyncWebSocketClient *client,
               AwsEventType type,
               void *arg,
               uint8_t *data,
               size_t len) {

  if (type == WS_EVT_CONNECT) {
    client->text("connected");
  }
}

void setup() {
  Serial.begin(115200);


  if (!LittleFS.begin()) {
    Serial.println("LittleFS failed");
    return;
  }

  pinMode(L1, INPUT_PULLUP);
  pinMode(L2, INPUT_PULLUP);
  pinMode(L3, INPUT_PULLUP);
  pinMode(L4, INPUT_PULLUP);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.begin();
}

int calculateLevel() {

  s1 = digitalRead(L1);
  s2 = digitalRead(L2);
  s3 = digitalRead(L3);
  s4 = digitalRead(L4);

  if (s4 == 0) return 100;
  if (s3 == 0) return 75;
  if (s2 == 0) return 50;
  if (s1 == 0) return 25;

  return 0;
}

void loop() {

  int level = calculateLevel();

  String json = "{";
  json += "\"level\":" + String(level);
  json += "}";

  ws.textAll(json);
  ws.cleanupClients();

  Serial.println(json);

  delay(300);
}
