#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

#define ssid "PDP Atas"
#define pw "kumahaaa"

WiFiMulti WiFiMulti;
SocketIOclient socketIO;

void event(
  socketIOmessageType_t type,
  uint8_t* payload,
  size_t length
){
  switch (type)
  {
  case sIOtype_DISCONNECT:
    Serial.println("[DISCONNECTED] ");
    break;
  case sIOtype_CONNECT:
    Serial.print("[CONNECTED] ");
    Serial.printf("%s\n",payload);
    socketIO.send(sIOtype_CONNECT, "/");
    break;
  case sIOtype_ERROR:
    Serial.print("[ERR] ");
    Serial.println(length);
    break;

  default:
    break;
  }
}

void setup(){
  Serial.begin(115200);
  WiFiMulti.addAP(ssid, pw);

  while(WiFiMulti.run() != WL_CONNECTED){}

  String ip = WiFi.localIP().toString();
  Serial.print("WiFi Connected ");
  Serial.println(ip);

  socketIO.begin(
    "10.133.226.120", 
    8880, 
    "/socket.io/?EIO=4&?transport=websocket",
    "websocket");
    
  socketIO.onEvent(event);
}

volatile unsigned long msgTs = 0;
void loop(){
  socketIO.loop();

  uint64_t now = millis();

  if(now - msgTs > 2000){
    msgTs = now;

    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();

    array.add("hey");
    JsonObject param1 = array.createNestedObject();
    param1["now"] = (uint32_t) now;

    String output;
    serializeJson(doc,output);

    socketIO.sendEVENT(output);

  }
}