#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "wireless2.peetersweb.nl";
const char* password = "4VNNCATA93CFH";
IPAddress mqttIp(192, 168, 1, 10);


WiFiServer server(80);
WiFiClient wfClient;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print(topic);
  Serial.print(" : ");
  Serial.write(payload, length);
  Serial.println("");
}

PubSubClient mqtt(mqttIp, 1883, callback, wfClient);

void buildInLedOn(){
  Serial.println("buildInLedOn");
  digitalWrite(D0, HIGH);
}
void buildInLedOff(){
  Serial.println("buildInLedOff");
  digitalWrite(D0, LOW);
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    pinMode(D0, OUTPUT);

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
    server.begin();
    Serial.println("Server started");

    // Print the IP address
    Serial.print("Use this URL : ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");

    if (mqtt.connect("arduinoClient", "testuser", "testpass")) {
      Serial.println("mqtt connected");
      mqtt.publish("outTopic","hello world");
      mqtt.subscribe("inTopic");
    }
}

void loop() {
  mqtt.loop();
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  // int count = 0;
  // while(!client.available()){
  //   delay(100);
  //   Serial.println("Cancelling new request");
  //   if(count = count++ > 50){
  //     return;
  //   }
  // }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request

  int value = LOW;
  if (request.indexOf("/LED=ON") != -1) {
    buildInLedOn();
    value = HIGH;
  }
  if (request.indexOf("/LED=OFF") != -1){
    buildInLedOff();
    value = LOW;
  }



  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("Led pin is now: ");

  if(value == HIGH) {
    client.print("On");
  } else {
    client.print("Off");
  }
  client.println("<br><br>");
  client.println("Click <a href=\"/LED=ON\">here</a> turn the LED on pin 5 ON<br>");
  client.println("Click <a href=\"/LED=OFF\">here</a> turn the LED on pin 5 OFF<br>");
  client.println("</html>");
  client.stop();
  Serial.println("Client disconnected");
  Serial.println("");
}
