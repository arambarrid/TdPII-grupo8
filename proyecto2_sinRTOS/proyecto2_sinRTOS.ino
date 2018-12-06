
#include "MQ135.h"
#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspUdp.h>
#include <PubSubClient.h>
#include "ArduinoJson.h" 
#define ANALOGPIN A1

char ssid[] = "FDG";            // your network SSID (name)
char pass[] = "loquito123__";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "192.168.0.10";
unsigned int estado;
unsigned int muestreo;
unsigned int tMuestreo;
char data[230];

/* Valor de gas */
unsigned int ppm;
MQ135 gasSensor = MQ135(ANALOGPIN);
/* Valor de luz ambiente */
unsigned int luz;

byte tiempoConnMQTT;
byte tiempoConnHTTP;

int tiempo1;
int tiempo2;

// Initialize the Ethernet client object
WiFiEspClient client;
WiFiEspClient espClient;
PubSubClient mqttClient(espClient);




void setup()
{
  estado=0;
  muestreo=10000;
  tMuestreo=0;
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial2.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial2);
  
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
      client.stop();
    }
    //connect to MQTT server
    mqttClient.setServer(server, 1883);
    mqttClient.setCallback(callback);
    reconnect();
  if (client.connect(server, 8888)) {
    sprintf(data, "%s%d%s%s%s%s", "POST /tcon HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: /\r\nHost: 192.168.0.3:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)tiempoConnHTTP+"&luz="+(String)tiempoConnMQTT)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)tiempoConnHTTP).c_str(),"&luz=",((String)tiempoConnMQTT).c_str());
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
    String json;
    tiempo1=millis(); //toma el tiempo para medir cuanto tarda en ejecutarse el loop
    if (client.connect(server, 8888)) {
      client.print("GET /parametros_cliente\r\nHTTP/1.1Host: 192.168.0.10:8888\r\nConnection: close\r\n\r\n");
       while(!client.available());
      while(client.available()) {
        String line = client.readString();
        Serial.print("Valores");
        Serial.print(line);
        json=line;
        if(json.length()>0){
          Serial.println("from server: "+json);        
          const size_t bufferSize = JSON_ARRAY_SIZE(2) + 3*JSON_OBJECT_SIZE(2);
          DynamicJsonBuffer jsonBuffer(bufferSize);
          JsonObject& root = jsonBuffer.parseObject(json);
          estado = root.get<int>("protocolo_medicion"); 
          muestreo = root.get<int>("periodo_sensado") * 1000; 
          Serial.println(estado); 
          Serial.println(muestreo);             
        }

      } 
      client.stop();
    } 
    switch(estado){
      case 0:
        Serial.print("Tiempo de muestreo");
        Serial.println(tMuestreo);
        if(tMuestreo>=muestreo){
          Serial.println("Sensado");
          sensado();
          tMuestreo=0;
        }
        break;
      case 1:  
        Serial.println("Metrica HTTP");
        rafagaHTTP();
        estado=0;
        break;
      case 2:
        Serial.println("Metrica MQTT");
        rafagaMQTT();
        estado=0;
        break;
    }
    delay(1000);
    tiempo2=millis(); 
    tMuestreo=tMuestreo+(tiempo2-tiempo1);
    reconnect();

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
  if(myString=="estado"){
    if((payload[0]-'0')<3)
      estado=payload[0]-'0';
  }
  if(myString=="muestreo"){
    muestreo=payload[0]-'0';
  }
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
  for (int i=1; i <= 24; i++){
    ppm = 50;
    luz = 60;
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
  //mqttClient.loop();
  mqttClient.publish("datos","0,2");
  for (int i=1; i <= 24; i++){
    ppm =  50;
    luz = 60;
    //mqttClient.loop();
    sprintf(data, "%s%s%s", String(ppm).c_str(), ",", String(luz).c_str());
    mqttClient.publish("datos",data);
  }     
  //mqttClient.loop();
  mqttClient.publish("datos","2,0");
}

void sensado(void){
  ppm = gasSensor.getPPM() * 100;
  //mqttClient.loop();
  luz = analogRead(A0);
  //mqttClient.loop();
  Serial.print("Luz: ");
  Serial.println(luz);  
  Serial.print("Gas: ");
  Serial.println(ppm);
 //mqttClient.loop();
 if (client.connect(server, 8888)) {
    sprintf(data, "%s%d%s%s%s%s", "POST /insertar_valores HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\ncache-control: no-cache\r\nAccept: /\r\nHost: 192.168.0.3:8888\r\naccept-encoding: gzip, deflate\r\ncontent-length: ",((String("\nco2" + (String)ppm+"&luz="+(String)luz)).length()),"\r\nConnection: keep-alive\r\n\r\nco2=",((String)ppm).c_str(),"&luz=",((String)luz).c_str());
    //mqttClient.loop();
    client.print(data);
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
      //mqttClient.loop();

    }
    client.stop();
  }
}
