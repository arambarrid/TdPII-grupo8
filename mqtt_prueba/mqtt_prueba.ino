#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspUdp.h>
#include "SoftwareSerial.h"
#include <PubSubClient.h>


char ssid[] = "FDG"; // your network SSID (name)
char pass[] = "JRRtalkien123_"; // your network password
int status = WL_IDLE_STATUS; // the Wifi radio's status

// Initialize the Ethernet client object
WiFiEspClient espClient;


void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}

PubSubClient client(espClient);

SoftwareSerial Serial1(2,3); // RX, TX   //ESP8266 RX,TX connected to these pins
void setup() {
// initialize serial for debugging
Serial.begin(9600);
// initialize serial for ESP module
Serial1.begin(9600);
// initialize ESP module
WiFi.init(&Serial1);

// check for the presence of the shield
if (WiFi.status() == WL_NO_SHIELD) {
Serial.println("WiFi shield not present");
// don't continue
while (true);
}

// attempt to connect to WiFi network
while ( status != WL_CONNECTED) {
Serial.print("Attempting to connect to WPA SSID: ");
Serial.println(ssid);
// Connect to WPA/WPA2 network
status = WiFi.begin(ssid, pass);
}

// you're connected now, so print out the data
Serial.println("You're connected to the network");

//connect to MQTT server
client.setServer("192.168.0.10", 1883);
client.setCallback(callback);
reconnect();

}


void loop() {

client.loop();
delay(100);
}

void reconnect() {
// Loop until we're reconnected
while (!client.connected()) {
Serial.print("Attempting MQTT connection...");
// Attempt to connect, just a name to identify the client
if (client.connect("NANO")) {
Serial.println("connected");
// Once connected, publish an announcement...
//client.publish("outpic","Hello World");
// ... and resubscribe
  Serial.print("Cliente suscrito a topico");
  client.subscribe("esp/test");

} else {
  Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(5000);
}
}
}
