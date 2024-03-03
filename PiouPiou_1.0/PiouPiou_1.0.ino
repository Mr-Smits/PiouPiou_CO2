#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <DS3231.h>
#include "Date_Time.h"
#include <AHT20.h>
#include <ScioSense_ENS160.h>

DS3231 myRTC;
AHT20 aht20;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);

char Date_Time_STR[16]; //eg : "25-12-24 00:00," = 15 caratéres + \0
char Sensors_Values_STR[15];  //eg : "25.0,55.0,550," = 14 caractères + \0

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
  save_to_SD();

  Serial.print(Date_Time_STR);Serial.println(Sensors_Values_STR);
  delay(300000);
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
  bool h12, PM_time, Century = false;
  sprintf(Date_Time_STR,"%02d-%02d-%02d %02d:%02d,", myRTC.getDate(), myRTC.getMonth(Century), myRTC.getYear(), myRTC.getHour(h12, PM_time), myRTC.getMinute());
}

void get_sensors_values(void){
  if(aht20.available() && ens160.available()){
    float Temperature;
    float Humidity;
    uint16_t eCO2;

    char Temp_STR[5];
    char Hum_STR[5];

    Temperature = aht20.getTemperature();
    Humidity = aht20.getHumidity();

    ens160.set_envdata(Temperature, Humidity);
    ens160.measure(true);
    ens160.measureRaw(true);
    eCO2 = ens160.geteCO2();

    dtostrf(Temperature, 4, 1, Temp_STR);
    dtostrf(Humidity, 4, 1, Hum_STR);
    sprintf(Sensors_Values_STR,"%s,%s,%03d",Temp_STR, Hum_STR, eCO2);
  }
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
      }
      else Serial.println(F("Fail opening file !"));
    }
    else Serial.println(F("File doesn't exist !"));
}