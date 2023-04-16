#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>

const char* ssid = "ESP8266";
const char* password = "987654321";
uint8_t max_connection = 2;
int current_stations=0, new_stations=0;

const char* www_username = "admin";
const char* www_password = "1234";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Adafruit_MPU6050 mpu;

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

String getMPUValues() {
  DynamicJsonDocument mpuData(1024);
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  mpuData["mpu6050"]["accelX"] = a.acceleration.x;
  mpuData["mpu6050"]["accelY"] = a.acceleration.y;
  mpuData["mpu6050"]["accelZ"] = a.acceleration.z;
  mpuData["mpu6050"]["rotX"] = g.gyro.x;
  mpuData["mpu6050"]["rotY"] = g.gyro.y;
  mpuData["mpu6050"]["rotZ"] = g.gyro.z;
  String output;
  serializeJson(mpuData, output);
  return output;
}

String getGPSSpeed() {
  DynamicJsonDocument topSpeedData(1024);

  int topSpeed = rand() % 260 + 1;

  topSpeedData["speed"]["speed"] = topSpeed;
  String output;
  serializeJson(topSpeedData, output);
  return output;
}

String getGPSSpeedTop() {
  DynamicJsonDocument topSpeedData(1024);

  int topSpeed = rand() % 110 + 1;

  topSpeedData["speed"]["top"] = topSpeed;
  String output;
  serializeJson(topSpeedData, output);
  return output;
}

void initWiFi() {
  if(WiFi.softAP(ssid,password,1,false,max_connection)==true)
  {
    Serial.print("Access Point is Created with SSID: ");
    Serial.println(ssid);
    Serial.print("Max Connections Allowed: ");
    Serial.println(max_connection);
    Serial.print("Access Point IP: ");
    Serial.println(WiFi.softAPIP());
  }
  else
  {
    Serial.println("Unable to Create Access Point");
  }
}

void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  Serial.println("LittleFS mounted successfully");
}

void initMPU() {
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "getMPUValues") == 0) {
      ws.textAll(getMPUValues());
    }
    if (strcmp((char*)data, "getGPSSpeed") == 0) {
     ws.textAll(getGPSSpeed());
    }
    if (strcmp((char*)data, "getGPSSpeedTop") == 0) {
     ws.textAll(getGPSSpeedTop());
    }
  }  
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  initFS();
  initMPU();
  initWiFi();
  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });
  
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/script.js", "text/javascript");
  });

  server.serveStatic("/", LittleFS, "/");
  server.onNotFound(notFound);

  server.begin();
}

void loop() {
  new_stations=WiFi.softAPgetStationNum();

  if(current_stations<new_stations)
  {
    current_stations=new_stations;
    Serial.print("New Device Connected to SoftAP... Total Connections: ");
    Serial.println(current_stations);
  }
   
  if(current_stations>new_stations)
  {
    current_stations=new_stations;
    Serial.print("Device disconnected from SoftAP... Total Connections: ");
    Serial.println(current_stations);
  }
}