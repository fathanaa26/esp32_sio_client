#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>
// #include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const int PIN_AOUT = 33;
const int PIN_OneWire = 32;
#define ssid "samsunga8"
#define pw "78empatx"

OneWire oneWire(PIN_OneWire);
DallasTemperature sensors(&oneWire);
WiFiMulti WiFiMulti;
SocketIOclient socketIO;
WebSocketsClient wsc;

int prevNow = 0;

void event(
  socketIOmessageType_t type,
  uint8_t* payload,
  size_t length
){
  switch (type)
  {
  case sIOtype_DISCONNECT:
    Serial.println("[DISCONNECTED] ");
    // Serial.printf("%s\n",payload);
    break;
  case sIOtype_CONNECT:
    Serial.print("[CONNECTED] ");
    Serial.printf("%s %d\n",payload,length);
    // initial http req upgrade to switcing protocol
    socketIO.send(sIOtype_CONNECT, "/");
    break;
  case sIOtype_ERROR:
    Serial.print("[ERR] ");
    Serial.printf("%s \n",payload);
    break;
  case sIOtype_EVENT:
    // Serial.print("[EVENT] ");
    // Serial.printf("%s \n",payload);
  break;

  default:
    break;
  }
}

void setup(){
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  WiFiMulti.addAP(ssid, pw);
  sensors.begin();
  
  while(WiFiMulti.run() != WL_CONNECTED){}

  Serial.print("WiFi Connected ");
  Serial.println(WiFi.localIP().toString());

  socketIO.begin(
    "192.168.43.201", 
    8880, 
    "/socket.io/?EIO=4"
    );
    
  socketIO.onEvent(event);

}

volatile unsigned long msgTs = 0;

void loop(){

  uint64_t now = millis();

  sensors.setWaitForConversion(true);  // makes it async
  sensors.requestTemperatures();

  socketIO.loop();

  DynamicJsonDocument doc(1024);
  JsonArray payload = doc.to<JsonArray>();

  payload.add("ev_sn1");
  JsonObject data = payload.createNestedObject();
  data["suhu"] = (uint32_t) sensors.getTempCByIndex(0);
  data["kelembaban"] = (uint32_t) analogRead(PIN_AOUT);
  data["rssi"] = WiFi.RSSI();
  data["latency"] = (uint32_t) millis() - now;
  // data["latency"] = WiFi.ping("10.133.226.68");
  
  String output;
  serializeJson(doc,output);

  socketIO.sendEVENT(output);

  delay(1);
}