#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "utils.h"

const char* ssid = "wireless2.peetersweb.nl";
const char* password = "4VNNCATA93CFH";
const String topic = "home/livingroom/wallremote";
const char* mqttid = "home/livingroom/wallremote";
const IPAddress mqttIp(192, 168, 1, 10);
const int mqttPort = 1883;
SoftwareSerial HMISerial(D1, D2); // RX, TX

const String brightnessTopic = topic + "/brightness";

const byte nextionSuffix[] = {0xFF, 0xFF, 0xFF};

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient wfClient;
PubSubClient mqtt(mqttIp, mqttPort, callback, wfClient);


void initializeNextion() {
  HMISerial.write(nextionSuffix, sizeof(nextionSuffix));
}

void nextionCmd(const char* cmd){
  HMISerial.write(cmd);
  HMISerial.write(nextionSuffix, sizeof(nextionSuffix));
}

void setup() {
  Serial.begin(115200);
  HMISerial.begin(115200);
  Serial.println("Booting...");
  delay(500);
  initializeNextion();
  nextionCmd("page page0");
  nextionCmd("dims=100");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  setupOTA();

  // setupWifi(ssid, password);

  if (mqtt.connect(mqttid, "testuser", "testpass")) {
    Serial.println("mqtt connected");
    mqtt.subscribe(brightnessTopic.c_str());
  }

  nextionCmd("page page1");
}

void loop() {
  loopOTA();
  if (HMISerial.available() > 0) {
    Serial.println("Read byte");
    Serial.write(HMISerial.read());
  }
  mqtt.loop();
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.write(payload, length);
  Serial.println("");

  char* message = (char*)malloc(length+1);
  memcpy(message,payload,length);
  message[length] = 0;

  std::string str(message);

  if(brightnessTopic.compareTo(topic) == 0){
    const char* x = ("dims=" + str).c_str();
    Serial.println(x);
    HMISerial.write(x);
    initializeNextion();
  }
}
