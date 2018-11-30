
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
#include "WiFiEsp.h"

// Emulate Serial1 on pins 3/2 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(3, 2); // RX, TX
#endif

char ssid[] = "d.A";            // your network SSID (name)
char pass[] = "223530145522";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "192.168.0.3";

// Initialize the Ethernet client object
WiFiEspClient client;
#define ANALOGPIN A1



uint8_t buffer[128];
char data[250];



MQ135 gasSensor = MQ135(ANALOGPIN);


/* Valor de gas */
float ppm;
/* Valor de luz ambiente */
unsigned int AnalogValue;


/* Funciones de sensado */
void mq135_sensar(void);





void setup()
{
    
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
    
    int luz=16;
    int gas=18;
    sprintf(data, "%s%d%s%s%s%s", "POST /insertar HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: */*\r\nHost: 192.168.0.10:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)gas+"&luz="+(String)luz)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)gas).c_str(),"&luz=",((String)luz).c_str());
    client.print(data);

    
  }  
  

}


void loop()
{

    mq135_sensar();
    AnalogValue = analogRead(A0);
    Serial.println(AnalogValue);


    

   // if there are incoming bytes available
  // from the server, read them and print them
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client
  if (!client.connected()) {
    Serial.println();
    Serial.println("Disconnecting from server...");
    client.stop();

    // do nothing forevermore
    while (true);
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
