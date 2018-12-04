 /***************************************************************************************************
                            ExploreEmbedded Copyright Notice    
****************************************************************************************************
 * File:   01-TaskSwitching
 * Version: 15.0
 * Author: ExploreEmbedded
 * Website: http://www.exploreembedded.com/wiki
 * Description: File contains the free rtos example to demonstarte the task switching.
This code has been developed and tested on ExploreEmbedded boards.  
We strongly believe that the library works on any of development boards for respective controllers. 
Check this link http://www.exploreembedded.com/wiki for awesome tutorials on 8051,PIC,AVR,ARM,Robotics,RTOS,IOT.
ExploreEmbedded invests substantial time and effort developing open source HW and SW tools, to support consider 
buying the ExploreEmbedded boards.
 
The ExploreEmbedded libraries and examples are licensed under the terms of the new-bsd license(two-clause bsd license).
See also: http://www.opensource.org/licenses/bsd-license.php
EXPLOREEMBEDDED DISCLAIMS ANY KIND OF HARDWARE FAILURE RESULTING OUT OF USAGE OF LIBRARIES, DIRECTLY OR
INDIRECTLY. FILES MAY BE SUBJECT TO CHANGE WITHOUT PRIOR NOTICE. THE REVISION HISTORY CONTAINS THE INFORMATION 
RELATED TO UPDATES.
 
Permission to use, copy, modify, and distribute this software and its documentation for any purpose
and without fee is hereby granted, provided that this copyright notices appear in all copies 
and that both those copyright notices and this permission notice appear in supporting documentation.
**************************************************************************************************/
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include "MQ135.h"
#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspUdp.h>
#include <PubSubClient.h>
// Emulate Serial1 on pins 3/2 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(3, 2); // RX, TX
#endif

char ssid[] = "Macri Hetero";            // your network SSID (name)
char pass[] = "ATRACTIVIDADES";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "192.168.1.30";
boolean http;
int estado;
// Initialize the Ethernet client object
WiFiEspClient client;
WiFiEspClient espClient;

#define ANALOGPIN A1

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

PubSubClient mqttClient(espClient);

uint8_t buffer[128];
char data[250];



MQ135 gasSensor = MQ135(ANALOGPIN);


/* Valor de gas */
float ppm;
/* Valor de luz ambiente */
unsigned int luz;


/* Funciones de sensado */
void mq135_sensar(void);

void rafagaHTTP();



void setup()
{
  http=false;
  estado=0;
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
  
  printWifiStatus();

  Serial.println();
  Serial.println("Starting connection to server...");
    // if you get a connection, report back via serial
    if (client.connect(server, 8888)) {
      Serial.println("Connected to server");
      // Make a HTTP request
      //client.print("GET /valores HTTP/1.1\r\nHost: 192.168.0.3\r\n\r\n");

    //connect to MQTT server
    mqttClient.setServer("192.168.1.30", 1883);
    mqttClient.setCallback(callback);
      
    } 
  
  

}


void loop()
{



    switch(estado){
      case 0:
        mq135_sensar();
        luz = analogRead(A0);
        if(http){
          if (client.connect(server, 8888)) {
          Serial.println("Connected to server");
          sprintf(data, "%s%d%s%s%s%s", "POST /insertar HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: */*\r\nHost: 192.168.0.10:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)ppm+"&luz="+(String)luz)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)ppm).c_str(),"&luz=",((String)luz).c_str());
          client.print(data);
          // if there are incoming bytes available
          // from the server, read them and print them
          while (client.available()) {
            char c = client.read();
            Serial.write(c);
          }
          client.stop();
          }
        }
        else {
          reconnect();
          mqttClient.loop();
          mqttClient.publish("/topic1","1");
    
        }
        delay(1000);
        break;
      case 1:

      break;
      
    }




}




void mq135_sensar()   
{
  ppm = gasSensor.getPPM();
  Serial.println(ppm);

}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    if (mqttClient.connect("NANO")) {
    Serial.println("connected");
    // Once connected, publish an announcement...
    //mqttClient.publish("outpic","Hello World");
    // ... and resubscribe
      Serial.print("Cliente suscrito a topico");
      mqttClient.subscribe("/topic1");
    
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void rafagaHTTP(){
        ppm=0;
        luz=1;
        if (client.connect(server, 8888)) {
          sprintf(data, "%s%d%s%s%s%s", "POST /insertar HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: */*\r\nHost: 192.168.1.30:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)ppm+"&luz="+(String)luz)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)ppm).c_str(),"&luz=",((String)luz).c_str());
          client.print(data);
          while (client.available()) {
            char c = client.read();
            Serial.write(c);
          }
          client.stop();

        }        
        for (int i=0; i <= 50; i++){
          mq135_sensar();
          luz = analogRead(A0);
           if (client.connect(server, 8888)) {
              sprintf(data, "%s%d%s%s%s%s", "POST /insertar HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: */*\r\nHost: 192.168.1.30:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)ppm+"&luz="+(String)luz)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)ppm).c_str(),"&luz=",((String)luz).c_str());
              client.print(data);
              while (client.available()) {
                char c = client.read();
                Serial.write(c);
              }
              client.stop();
            }
           }
        ppm=1;
        luz=0;
        if (client.connect(server, 8888)) {
          sprintf(data, "%s%d%s%s%s%s", "POST /insertar HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: */*\r\nHost: 192.168.1.30:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)ppm+"&luz="+(String)luz)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)ppm).c_str(),"&luz=",((String)luz).c_str());
          client.print(data);
          while (client.available()) {
            char c = client.read();
            Serial.write(c);
          }
          client.stop();
        }
        Serial.println("Fin de rafaga HTTP");
}
