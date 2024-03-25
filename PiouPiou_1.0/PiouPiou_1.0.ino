#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <DS3231.h>
#include "Date_Time.h"
#include <AHT20.h>
#include <ScioSense_ENS160.h>

#define led_pin 13
#define _5SECONDES 5000
#define _5MINUTES 300000

DS3231 myRTC;
AHT20 aht20;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);

char Date_Time_STR[20]; // Increase size to accommodate formatted date/time
char Sensors_Values_STR[20]; // Increase size to accommodate sensor values

void setup() {
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH); 
  Serial.begin(9600);
  Serial.println(F("Serial communication Started :"));
  Wire.begin();

  while (!SD.begin(10)) {
    Serial.println(F("SD card initialization failed!"));
    delay(1000);
  }
  Serial.println(F("Connected to SD_Card !"));

  while (!aht20.begin()) {
    Serial.println(F("AHT20 initialization failed!"));
    delay(1000);
  }
  Serial.println(F("Connected to AHT20 !"));

  while (!ens160.begin()) {
    Serial.println(F("ENS160 initialization failed!"));
    delay(1000);
  }
  ens160.setMode(ENS160_OPMODE_STD);
  Serial.println(F("Connected to ENS160 !"));
  digitalWrite(led_pin, LOW); 
}

void loop() {
  if (get_Date_Time() && get_sensors_values()) {
    Serial.print(Date_Time_STR); Serial.println(Sensors_Values_STR);
    save_to_SD();
  }
  else digitalWrite(led_pin, HIGH);
  delay(_5SECONDES);
  //delay(_5MINUTES);
}

bool get_Date_Time() {
  bool h12, PM_time, Century = false;
  sprintf(Date_Time_STR,"%02d-%02d-%02d %02d:%02d,", myRTC.getDate(), myRTC.getMonth(Century), myRTC.getYear(), myRTC.getHour(h12, PM_time), myRTC.getMinute());
  return true; // Assuming RTC always returns valid data
}

bool get_sensors_values() {
  if(aht20.available() && ens160.available()){
    float Temperature;
    float Humidity;
    uint16_t eCO2;

    char Temp_STR[6]; // Increase size to accommodate sensor values
    char Hum_STR[6]; // Increase size to accommodate sensor values

    Temperature = aht20.getTemperature();
    Humidity = aht20.getHumidity();

    ens160.set_envdata(Temperature, Humidity);
    ens160.measure(true);
    ens160.measureRaw(true);
    eCO2 = ens160.geteCO2();

    dtostrf(Temperature, 5, 1, Temp_STR);
    dtostrf(Humidity, 5, 1, Hum_STR);
    sprintf(Sensors_Values_STR,"%s,%s,%03d",Temp_STR, Hum_STR, eCO2);
    return true;
  }
  return false; // Data not available
}

void save_to_SD() {
  File SD_File = SD.open("DATA.TXT", FILE_WRITE);
  if (SD_File) {
    digitalWrite(led_pin, LOW); 
    SD_File.print(Date_Time_STR);
    SD_File.println(Sensors_Values_STR);
    SD_File.close();
    Serial.println(F("Data written to SD card !"));
  } else {
    Serial.println(F("Failed opening file !"));
    digitalWrite(led_pin, HIGH); 
  }
}
