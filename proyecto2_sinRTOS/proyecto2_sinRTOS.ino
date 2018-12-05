
#include "MQ135.h"
#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspUdp.h>
#include <PubSubClient.h>

#define ANALOGPIN A1
// Emulate Serial1 on pins 3/2 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(3, 2); // RX, TX
#endif

char ssid[] = "d.A";            // your network SSID (name)
char pass[] = "223530145522";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "192.168.0.3";
boolean http;
unsigned int estado;
unsigned int muestreo;
uint8_t buffer[128];
char data[230];
/* Valor de gas */
unsigned int ppm;
MQ135 gasSensor = MQ135(ANALOGPIN);
/* Valor de luz ambiente */
unsigned int luz;

byte tiempoConnMQTT;
byte tiempoConnHTTP;

// Initialize the Ethernet client object
WiFiEspClient client;
WiFiEspClient espClient;
PubSubClient mqttClient(espClient);




void setup()
{
  http=false;
  estado=2;
  muestreo=5000;
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);
  
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }  
  printWifiStatus();
  Serial.println(F("Starting connection to server..."));
    // if you get a connection, report back via serial
    /*----MEDICION DE TIEMPOS----*/
    unsigned long tiempo1=millis();
    if (client.connect(server, 8888)) {
      unsigned long tiempo2=millis();
      tiempoConnHTTP=tiempo2-tiempo1;
      Serial.println(tiempoConnHTTP);
      /*----MEDICION DE TIEMPOS----*/
      Serial.println(F("Connected to server"));
    }
    //connect to MQTT server
    mqttClient.setServer(server, 1883);
    mqttClient.setCallback(callback);
    reconnect();
  if (client.connect(server, 8888)) {
    sprintf(data, "%s%d%s%s%s%s", "POST /insertar HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: /\r\nHost: 192.168.0.3:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)tiempoConnHTTP+"&luz="+(String)tiempoConnMQTT)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)tiempoConnHTTP).c_str(),"&luz=",((String)tiempoConnMQTT).c_str());
    Serial.println(sizeof(data));
    client.print(data);
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
    client.stop();
  }  
    
}


void loop()
{
    switch(estado){
      case 0:
        Serial.println("Sensado");
        sensado();
        break;
      case 1:  
        //Serial.println("Metrica HTTP");
        rafagaHTTP();
        break;
      case 2:
        //Serial.println("Metrica MQTT");
        rafagaMQTT();
        break;
    }
    delay(muestreo);
}






void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect, just a name to identify the client
    /*----MEDICION DE TIEMPOS----*/
    unsigned long tiempo1=millis();
    if (mqttClient.connect("NANO")) {
      unsigned long tiempo2=millis();
      tiempoConnMQTT=tiempo2-tiempo1;
      Serial.println(tiempoConnMQTT);
      /*----MEDICION DE TIEMPOS----*/
      Serial.print(F("Cliente suscrito a topico"));
      mqttClient.subscribe("datos");
      delay(10); // ---- VER SI SON NECESARIOS ESTOS DELAY ----
      mqttClient.subscribe("estado");
      delay(10);
      mqttClient.subscribe("muestreo");
      
    
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(mqttClient.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(topic);
  Serial.print(F("Message:"));
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  String myString = String(topic);
  if(myString=="estado")
    estado=atoi(payload[0]);
    Serial.println(estado);
}

void rafagaHTTP(void){
  ppm=0;
  luz=1;
  if (client.connect(server, 8888)) {
    sprintf(data, "%s%d%s%s%s%s", "POST /insertar HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: /\r\nHost: 192.168.0.3:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)ppm+"&luz="+(String)luz)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)ppm).c_str(),"&luz=",((String)luz).c_str());
    Serial.println(sizeof(data));
    client.print(data);
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
    client.stop();
  }        
  for (int i=1; i <= 99; i++){
    ppm = gasSensor.getPPM() * 100;
    luz = analogRead(A0);
     if (client.connect(server, 8888)) {
        sprintf(data, "%s%d%s%s%s%s", "POST /insertar HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: /\r\nHost: 192.168.0.10:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)ppm+"&luz="+(String)luz)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)ppm).c_str(),"&luz=",((String)luz).c_str());
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
    sprintf(data, "%s%d%s%s%s%s", "POST /insertar HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: /\r\nHost: 192.168.0.3:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)ppm+"&luz="+(String)luz)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)ppm).c_str(),"&luz=",((String)luz).c_str());
    client.print(data);
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
    client.stop();
  }
}

void rafagaMQTT(void){
  mqttClient.loop();
  mqttClient.publish("datos","0,2");
  for (int i=1; i <= 99; i++){
    ppm =  gasSensor.getPPM() * 100;
    luz = analogRead(A0);
    mqttClient.loop();
    sprintf(data, "%s%s%s", String(ppm).c_str(), ",", String(luz).c_str());
    mqttClient.publish("datos",data);
  }     
  mqttClient.loop();
  mqttClient.publish("datos","2,0");
}

void sensado(void){
  ppm = gasSensor.getPPM() * 100;
  luz = analogRead(A0);
  Serial.print("Luz: ");
  Serial.println(luz);  
  Serial.print("Gas: ");
  Serial.println(ppm);
  if(http){
       if (client.connect(server, 8888)) {
          sprintf(data, "%s%d%s%s%s%s", "POST /insertar HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: /\r\nHost: 192.168.0.3:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)ppm+"&luz="+(String)luz)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)ppm).c_str(),"&luz=",((String)luz).c_str());
          client.print(data);
          while (client.available()) {
            char c = client.read();
            Serial.write(c);
          }
          client.stop();
        }
  }
  else {
      mqttClient.loop();
      sprintf(data, "%s%s%s", String(ppm).c_str(), ",", String(luz).c_str());
      mqttClient.publish("estado",data);
  }
}
