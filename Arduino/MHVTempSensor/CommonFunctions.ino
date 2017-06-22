#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "constants.h"
OneWire oneWire(DS18B20_PIN);
DallasTemperature DS18B20(&oneWire);

extern float temp;
extern float batteryVoltage;

String readLineAndTrim(File f)
{
  String readLineString = f.readStringUntil('\n');
  readLineString.trim();
  return readLineString;
}

void readLineIntoCharArray(File f, char* buf, int buf_length)
{
  String line = readLineAndTrim(f);
  line.toCharArray(buf, buf_length);
}

void setupIO(){
  pinMode(LED_PIN, OUTPUT);
  pinMode(VDIV_EN_PIN, OUTPUT);
  digitalWrite(VDIV_EN_PIN, LOW);
  digitalWrite(LED_PIN, HIGH);

  #ifdef DEBUG
    pinMode(DEBUG_PIN_1, OUTPUT);
    pinMode(DEBUG_PIN_2, OUTPUT);
  #endif
}

void updateBatteryVoltage(){
    digitalWrite(VDIV_EN_PIN, HIGH);
    delayMicroseconds(1000);
    batteryVoltage = analogRead(A0) * ADC_CAL;
    digitalWrite(VDIV_EN_PIN, LOW);
}

void updateTemperature(){
      do {
      DS18B20.requestTemperatures(); 
      temp = DS18B20.getTempCByIndex(0);
    } while (temp == 85.0 || temp == (-127.0));
}


