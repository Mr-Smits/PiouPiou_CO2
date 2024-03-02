#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <DS3231.h>
#include "Date_Time.h"
#include <AHT20.h>
#include <ScioSense_ENS160.h>

DS3231 myRTC;
RTClib Date;
bool h12Flag, pmFlag, century = false;
DateTime now;
char Date_Time_STR[21]; //eg : "25/12/2024,00:00:00," = 20 caratéres + \0

AHT20 aht20;
float Temperature;
char Temp_STR[6];
float Humidity;
char Hum_STR[6];

ScioSense_ENS160 ens160(ENS160_I2CADDR_1);
uint16_t eCO2;

char Sensors_Values_STR[17];  //eg : "25.00,55.00,550," = 16 caractères + \0

void setup() {
  Serial.begin(9600);
  Wire.begin();

  while(SD.begin(10) == false);
  init_Date_Time();
  while(aht20.begin() == false);
  while(ens160.begin() == false);
  ens160.setMode(ENS160_OPMODE_STD);
}

void loop() {
  get_Date_Time();
  get_sensors_values();
  Serial.print(Date_Time_STR);Serial.println(Sensors_Values_STR);
  save_to_SD();
  delay(5000);
}

void init_Date_Time(void){
  myRTC.setClockMode(false);  // set to 24h
  
  myRTC.setDoW(init_DoW);
  myRTC.setDate(init_date);
  myRTC.setMonth(init_month);
  myRTC.setYear(init_year);  
  myRTC.setHour(init_hour);
  myRTC.setMinute(init_minute);
  myRTC.setSecond(init_second);
}

void get_Date_Time(void){
  DateTime now = Date.now();
  sprintf(Date_Time_STR,"%02d/%02d/%02d,%02d:%02d:%02d,", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
}

void get_sensors_values(void){
  if(aht20.available()){
    Temperature = aht20.getTemperature();
    Humidity = aht20.getHumidity();
  }
  
  if(ens160.available()){
    ens160.set_envdata(Temperature, Humidity);
    ens160.measure(true);
    ens160.measureRaw(true);
    eCO2 = ens160.geteCO2();
  }

  dtostrf(Temperature, 5, 2, Temp_STR);
  dtostrf(Humidity, 5, 2, Hum_STR);
  sprintf(Sensors_Values_STR,"%s,%s,%03d",Temp_STR, Hum_STR, eCO2);
}

void save_to_SD(void){
  if(SD.exists("DATA.TXT"))
    {
      File SD_File = SD.open("DATA.TXT", FILE_WRITE);
      if (SD_File)
      {
        SD_File.print(Date_Time_STR);
        SD_File.println(Sensors_Values_STR);
        SD_File.close();
        Serial.println("Data writed to file !");
      }
      else Serial.println("Fail opening file !");
    }
    else Serial.println("File doesn't exist !");
}