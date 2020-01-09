#include <DHT.h>
#include "PMS.h"
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define temperaturePin 5
#define DHTPIN A1
#define DHTTYPE DHT22
#define soundPin A0    

#define pmTimeOut 20000
#define temperatureTimeout 2000
#define soundTimeout 500
#define co2Timeout 2000
#define pmTimeout 1000
#define sendTimeout 300000
//#define sendTimeout 60000
//#define sendTimeout 1000




SoftwareSerial pmsSerial(10, 9);
PMS pms(pmsSerial);

SoftwareSerial ppmSerial(3, 2);

//SoftwareSerial pmsSerial(10, 11);
//PMS pms(pmsSerial);
//
//SoftwareSerial ppmSerial(12, 13);
//MHZ19 mhz;

DHT dht(DHTPIN, DHT22);
PMS::DATA data;


int prevVal = LOW;
long th, tl, h, l, ppm;
float temperature = 0;
float humiture = 0;
float sound = 0;
int co2 = 0;
int pm2_5 = 0;
int pm0_1 = 0;
int pm10 = 0;

byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
unsigned char response[9];

void setup() {

  Serial.begin(115200);
  pinMode(soundPin, INPUT);
  pinMode(DHTPIN, INPUT);
  dht.begin();
}



unsigned long temperatureTimer = millis();
unsigned long temperatureTimer_Screen = millis();
unsigned long humitureTimer = millis();
unsigned long humitureTimer_Screen = millis();
unsigned long soundTimer = millis();
unsigned long soundTimer_Screen = millis();
unsigned long co2Timer = millis();
unsigned long pmTimer = millis();
unsigned long co2Timer_Screen = millis();
unsigned long sendTime = millis();

void loop() { 
  getData();
  if(millis() - sendTime >= sendTimeout) {
     sendTime = millis();
     String json_string = "";
      DynamicJsonDocument doc(512);
      JsonObject data = doc.to<JsonObject>();
      
      data["pm0_1"] = String(pm0_1);
      data["pm2_5"] = String(pm2_5);
      data["pm10"] = String(pm10);
      data["temperature"] = String(temperature);
      data["humidity"] = String(humiture);
      data["co2"] = String(co2);      
      serializeJson(doc, json_string);
     Serial.print(json_string);
  }
  
}

float getTemperature(){
        float temp = dht.readTemperature();
        if (isnan(temp)) return 0;
        return temp;
}
float getHumiture(){
   float humn = dht.readHumidity();
   if (isnan(humn)) return 0;
   return humn;
}
float getSound(){
  float analog = analogRead(soundPin);
  float dB = (analog+83.2073) / 11.003;
  return dB;
}
void getPm(){
    pmsSerial.begin(9600);
    pms.requestRead();
    if (pms.readUntil(data)) {
      pm0_1 = data.PM_AE_UG_1_0;
      pm2_5 = data.PM_AE_UG_2_5;
      pm10  = data.PM_AE_UG_10_0;
    }
    pmsSerial.end();
}
void getCo2(){
  
  ppmSerial.begin(9600);
  
  ppmSerial.write(cmd, 9);
  memset(response, 0, 9);
  ppmSerial.readBytes(response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=response[i];
  crc = 255 - crc;
  crc++;

  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / "+ String(response[8]));
    return;
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256*responseHigh) + responseLow;
    co2 = ppm;
  }

   ppmSerial.end();
}

void getData() {
  if(millis() - temperatureTimer >= temperatureTimeout){
    temperatureTimer = millis();
    temperature = getTemperature();
    
    humitureTimer = millis();
    humiture = getHumiture();
  }      
  if(millis() - soundTimer >= soundTimeout) {
    soundTimer = millis();
    sound = getSound();
  }
  if(millis() - pmTimer >= pmTimeout) {
    delay(500);
    pmTimer = millis();
    getPm();
    delay(500);
  }
  if(millis() - co2Timer >= co2Timeout) {
    delay(500);
    co2Timer = millis();
    getCo2();
    delay(500);
  }
}
