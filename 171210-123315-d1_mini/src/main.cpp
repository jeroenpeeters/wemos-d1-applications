#include <string>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleTimer.h>
// #include "utils.h"
const std::string OPEN ("OPEN");
const std::string CLOSE ("CLOSE");

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define MAX_TEMP_READINGS 10
#define DHTPIN            2         // Pin which is connected to the DHT sensor.

// Uncomment the type of sensor in use:
#define DHTTYPE           DHT11


DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;


const char* ssid = "wireless2.peetersweb.nl";
const char* password = "4VNNCATA93CFH";
const char* topic = "home/livingroom/curtains/set";
const char* pos_topic = "home/livingroom/curtains/position";
const IPAddress mqttIp(192, 168, 1, 10);
const int mqttPort = 1883;

WiFiClient wfClient;
SimpleTimer timer;

float temps[MAX_TEMP_READINGS];
int temp_pos = 0;

void buildInLedOn(){
  digitalWrite(D2, LOW);
}
void buildInLedOff(){
  digitalWrite(D2, HIGH);
}

void callback(char* topic, byte* payload, unsigned int length) {
}

PubSubClient mqtt(mqttIp, mqttPort, callback, wfClient);

void setup() {
    // setupOTA();
    // put your setup code here, to run once:
    Serial.begin(115200);

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

    if (mqtt.connect("temp-sensor1", "testuser", "testpass")) {
      Serial.println("mqtt connected");
    }

    // timer.setInterval(100, repeatMe);

    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Temperature");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");
    Serial.println("------------------------------------");
    // Print humidity sensor details.
    dht.humidity().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Humidity");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");
    Serial.println("------------------------------------");
    // Set delay between sensor readings based on sensor details.
    delayMS = sensor.min_delay / 1000;
}

char buffer[100];

void loop() {
  // loopOTA();
  mqtt.loop();
  timer.run();

  delay(delayMS);
   // Get temperature event and print its value.
   sensors_event_t event;
   dht.temperature().getEvent(&event);
   if (isnan(event.temperature)) {
     Serial.println("Error reading temperature!");
   }
   else {
     Serial.print("Temperature: ");
     Serial.print(event.temperature);
     Serial.println(" *C");
     temps[temp_pos++] = event.temperature;
     if(temp_pos == MAX_TEMP_READINGS){
       temp_pos = 0;
       float avg_temp = 0;
       for(int i=0; i < MAX_TEMP_READINGS; i++) avg_temp += temps[i];

       avg_temp = avg_temp / MAX_TEMP_READINGS;
       dtostrf(avg_temp,5, 2, buffer);
       mqtt.publish("home/kamerluke/temperature",buffer);
     }
   }
   // Get humidity event and print its value.
   dht.humidity().getEvent(&event);
   if (isnan(event.relative_humidity)) {
     Serial.println("Error reading humidity!");
   }
   else {
     Serial.print("Humidity: ");
     Serial.print(event.relative_humidity);
     Serial.println("%");
   }
}
