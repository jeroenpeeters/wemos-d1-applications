#include <string>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleTimer.h>
#include "utils.h"
const std::string OPEN ("OPEN");
const std::string CLOSE ("CLOSE");

const char* ssid = "wireless.peetersweb.nl";
const char* password = "4VNNCATA93CFH";
const char* topic = "home/livingroom/curtains/set";
const char* pos_topic = "home/livingroom/curtains/position";
const IPAddress mqttIp(192, 168, 1, 10);
const int mqttPort = 1883;

WiFiClient wfClient;
SimpleTimer timer;

int S_STOPPED = 0;
int S_OPEN = 1;
int S_CLOSED = 2;
int S_OPENING = 3;
int S_CLOSING = 4;


int STATE = S_STOPPED;

void buildInLedOn(){
  digitalWrite(D4, LOW);
}
void buildInLedOff(){
  digitalWrite(D4, HIGH);
}
void open(){
  Serial.println("Opening curtains...");
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  delay(250);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  STATE = S_OPENING;
}

void close(){
  Serial.println("Closing curtains...");
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  delay(250);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, LOW);
  digitalWrite(D3, HIGH);
  STATE = S_CLOSING;
}

void stop(){
  Serial.println("Stopping curtains...");
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  STATE = S_STOPPED;
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

  if(OPEN.compare(message) == 0){
    open();
  } else if(CLOSE.compare(message) == 0){
    close();
  } else {
    stop();
  }
  free(message);
}

PubSubClient mqtt(mqttIp, mqttPort, callback, wfClient);

int pos = 0;
char buffer[100 + 1];

void repeatMe(){
  if(STATE == S_OPENING){
    pos++;
    sprintf(buffer,"%d",pos);
    mqtt.publish(pos_topic, buffer);
    if(pos >= 11){
      stop();
    }
  }else if(STATE == S_CLOSING){
    pos--;
    sprintf(buffer,"%d",pos);
    mqtt.publish(pos_topic, buffer);
    if(pos <= 0){
      stop();
    }
  }
}

void setup() {
    setupOTA();
    // put your setup code here, to run once:
    Serial.begin(115200);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D3, OUTPUT);
    pinMode(D4, OUTPUT);

    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    digitalWrite(D3, LOW);

    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      buildInLedOn();
      delay(250);
      buildInLedOff();
      delay(250);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Print the IP address
    Serial.print("My IP is : ");
    Serial.println(WiFi.localIP());

    if (mqtt.connect("curtain-controller", "testuser", "testpass")) {
      Serial.println("mqtt connected");
      mqtt.publish("discover",("curtain-controller," + WiFi.localIP().toString()).c_str());
      mqtt.subscribe(topic);
    }

    timer.setInterval(1000, repeatMe);
}

void loop() {
  loopOTA();
  mqtt.loop();
  timer.run();
}
