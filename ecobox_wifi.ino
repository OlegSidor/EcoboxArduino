#include <DHT.h>
#include "MHZ19.h"
#include "PMS.h"
#include <SoftwareSerial.h>

#define temperaturePin 5
#define DHTPIN A1
#define DHTTYPE DHT22
#define soundPin A0    

#define pmTimeOut 20000
#define temperatureTimeout 2000
#define soundTimeout 500
#define co2Timeout 1000
#define pmTimeout 1000
#define sendTimeout 300000
//#define sendTimeout 60000
//#define sendTimeout 1000




SoftwareSerial pmsSerial(10, 9);
PMS pms(pmsSerial);

SoftwareSerial ppmSerial(3, 2);
MHZ19 mhz;

//SoftwareSerial pmsSerial(10, 11);
//PMS pms(pmsSerial);
//
//SoftwareSerial ppmSerial(12, 13);
//MHZ19 mhz(&ppmSerial);

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

int device_id = 9; 

void setup() {

  Serial.begin(115200);
  pinMode(soundPin, INPUT);
  pinMode(DHTPIN, INPUT);
  dht.begin();

  ppmSerial.begin(9600);
  mhz.begin(ppmSerial);
  mhz.setRange(2000);
  mhz.calibrateZero();
  mhz.setSpan(2000);
  mhz.autoCalibration(false); 
  ppmSerial.end();
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
     Serial.print("/air/add/?lat=48.524283&lon=24.984165&sat=9&alt=245.00&speed=0.20&pm0_1="+String(pm0_1)+"&pm2_5="+String(pm2_5)+"&pm10="+String(pm10)+"&temperature="+String(temperature)+"&humidity="+String(humiture)+"&co2="+String(co2)+"&noise="+String(sound)+"&uid=0");
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
      
    co2 = mhz.getCO2();
    
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
