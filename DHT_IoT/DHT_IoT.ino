#include <ESP8266WiFi.h>
#include "DHT.h"

#define DHTPIN 14 
// #define DHTTYPE DHT11
#define DHTTYPE DHT22

#define ledGRN 12
#define ledRED 16

#define ledRD1 2
#define ledGN1 13

#define ledRD2 4

#define inpDBG 0
#define outPIN 5

DHT sensor(DHTPIN, DHTTYPE);

/////////////////////////////////////
// Constants

const char* ssid = "diginet_BG";
const char* pass = "9704567952";
const char* host = "172.16.76.102";
const int   port = 1880;
const int   dly = 60;

/////////////////////////////////////
// Variables

int         cnt = 0;
bool        dbg = false;
bool        hum = true;
bool        err = false;
bool        led = false;

String      uuid = "";

/////////////////////////////////////
//

void setDevice(){
  
}

String build(){
  String uri = "/sensor/";
  uri += "?uuid=";
  uri += uuid;
  uri += input();
  return uri;
}

String input() {
  float h = 0;
  float f = 0;
  float hif = 0;
  String dat = "";
  
  f = sensor.readTemperature(true);
  
  if (isnan(f)) {
    err = true;
    Serial.println("Failed to read from sensor!");
    Serial.println(f);
    return "&err=Failed To Read From Sensor";
  }
  err = false;
  
  h = sensor.readHumidity();
  hif = sensor.computeHeatIndex(f, h);
  
  dat += "&typ1=TMP";
  dat += "&val1=";
  dat += f;
  dat += "&typ2=HUM";
  dat += "&val2=";
  dat += h;
  return dat;
}

void setUuid(){
   uuid = WiFi.macAddress();
   uuid.replace(":","");
   Serial.println(uuid);
}

void connectToAp(){
  int md = 4;
  if(dbg){
    Serial.println();
    Serial.print("Attempting Connection to ");
    Serial.println(ssid);  
  }
  
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if(md == 3) md = 4;
    else md = 3;
    setLED(md);
    if(dbg) Serial.print(".");
  }

  if(dbg){
    Serial.println("");  
    Serial.println("WiFi connected");  
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());  
  }
}

void setLED(int led){
  switch(led){
    case 1: {
      digitalWrite(ledRED, HIGH);
      digitalWrite(ledGRN, LOW);
      break;
    }
    case 2: {
      digitalWrite(ledGRN, HIGH);
      digitalWrite(ledRED, LOW);
      break;
    }
     case 3: {
      digitalWrite(ledGN1, LOW);
      digitalWrite(ledRD1, HIGH);
      break;
    }
     case 4: {
      digitalWrite(ledRD1, LOW);
      digitalWrite(ledGN1, HIGH);
      break;
    }
    default: {
      digitalWrite(ledGRN, LOW);
      digitalWrite(ledRED, LOW);
      digitalWrite(ledGN1, LOW);
      digitalWrite(ledRD1, LOW);
      break;
    }
  }
}

void snd(){
  WiFiClient client;
  String uri = build();
  
  if (!client.connect(host, port)) {
    setLED(1);
    if(dbg) {
      Serial.print("Failed To Connect To Server At ");
      Serial.println(host); 
    } 
    
    return;
  } else {
    setLED(2);
  }
  
  if(dbg) {
    Serial.print("Requesting URI: ");
    Serial.println(uri); 
  }
  
  client.print(String("GET ") + uri + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  while(client.available()){
    String line = client.readStringUntil('\r');
    if(dbg){
      Serial.print(line);
    }
  }
  if(dbg){
    Serial.println("");
  }
  return;
}

void setup(){
  Serial.begin(115200);
  dbg = true;
  
  pinMode(ledRED, OUTPUT);
  pinMode(ledGRN, OUTPUT);

  pinMode(ledRD1, OUTPUT);
  pinMode(ledGN1, OUTPUT);
  
  pinMode(inpDBG, INPUT);
  
  setLED(1);
  setLED(3);
  
  connectToAp();
  setUuid(); 
  sensor.begin();

  delay(1000);
  snd();
}

void loop() {
  delay(1000);
  cnt += 1;
  
  if(cnt >= dly) {
    cnt = 0;
    snd();
  }
  
  if(err == true) {
    setLED(3);
  } else {
    setLED(4);
  }
}
