#include <Wire.h> 
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <dht.h>
#include <Wire.h> 
#include <SPI.h>
#include <SoftwareSerial.h>
#include "PMS.h"

#define temperaturePin 5
#define humiturePin 4
#define soundPin A0    

#define temperatureTimeout 1000
#define humitureTimeout 2000
#define soundTimeout 500
#define co2Timeout 1000
#define sendTimeout 60000
//#define sendTimeout 1000
#define pwmPin 9

dht DHT;
OneWire oneWire(temperaturePin); 
DallasTemperature sensors(&oneWire);
SoftwareSerial pmsSerial(2, 3);

PMS pms(pmsSerial);
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

int device_id = 8; 

void setup() {

  Serial.begin(115200);
  pmsSerial.begin(9600);
  
  pinMode(pwmPin, INPUT);
  pinMode(soundPin, INPUT);
}



unsigned long temperatureTimer = millis();
unsigned long temperatureTimer_Screen = millis();
unsigned long humitureTimer = millis();
unsigned long humitureTimer_Screen = millis();
unsigned long soundTimer = millis();
unsigned long soundTimer_Screen = millis();
unsigned long co2Timer = millis();
unsigned long co2Timer_Screen = millis();
unsigned long sendTime = millis();

void loop() { 
  if(millis() - temperatureTimer >= temperatureTimeout){
    temperatureTimer = millis();
    temperature = getTemperature();
  }      

  if(millis() - humitureTimer >= humitureTimeout){
    humitureTimer = millis();
    humiture = getHumiture();
  }
  if(millis() - soundTimer >= soundTimeout) {
    soundTimer = millis();
    sound = getSound();
  }

  if (pms.readUntil(data)) {
    pm0_1 = data.PM_AE_UG_1_0;
    pm2_5 = data.PM_AE_UG_2_5;
    pm10  = data.PM_AE_UG_10_0;
  }
  
  unsigned long tt = millis();
  int haveData = digitalRead(pwmPin);
  if (haveData == HIGH) {
    if (haveData != prevVal) {
      h = tt;
      tl = h - l;
      prevVal = haveData;
    }
   }  else {
    if (haveData != prevVal) {
      l = tt;
      th = l - h;
      prevVal = haveData;
      ppm = 5000 * (th - 2) / (th + tl - 4);
      co2 = ppm;
    }
   }

  if(millis() - sendTime >= sendTimeout) {
     sendTime = millis();
     Serial.println("/air/add/?lat=48.524283&lon=24.984165&sat=9&alt=245.00&speed=0.20&pm0_1="+String(pm0_1)+"&pm2_5="+String(pm2_5)+"&pm10="+String(pm10)+"&temperature="+String(temperature)+"&humidity="+String(humiture)+"&co2="+String(co2)+"&noise="+String(sound)+"&device_id="+String(device_id)+"&uid=0");
  }
  
}

float getTemperature(){
        sensors.requestTemperatures();
        float temp = sensors.getTempCByIndex(0);
        if(temp == -127.00) return 0;
        return temp;
}
float getHumiture(){
   DHT.read11(humiturePin);
   float humn = DHT.humidity;
   if(humn == -999.00) return 0;
   return humn;
}
float getSound(){
  float analog = analogRead(soundPin);
  float dB = (analog+83.2073) / 11.003;
  return dB;
}
