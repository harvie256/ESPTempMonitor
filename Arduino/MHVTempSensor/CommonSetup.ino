#include <ESP8266WebServer.h>
#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "constants.h"

float temp = -1000.0;
float batteryVoltage = 0.0;

