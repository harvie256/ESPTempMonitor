#include <ESP8266WebServer.h>
#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

#include "constants.h"

/*
 *  Boot mode, either into the config interface or temp sensing programing, 
 *  is determined by a value in the RTC memory.  The RTC memory will keep
 *  its value across deep sleep, but will reset when power is lost.
 *  
 *  Therefore by removing power (i.e. pulling out the batteries), the ESP
 *  will reboot into the config interface.  A timer will then expire if no
 *  web requests are made to start the reading loop.
 * 
 */
bool bootModePowerOn;

void IsBootPowerOn()
{
  bootModePowerOn = ESP.getResetReason().startsWith("External");
}

void setup() {
  IsBootPowerOn();
  if(bootModePowerOn){
    WebServerSetup();
  }else {
    MainProgSetup();
  }
}

void loop() {
  if(bootModePowerOn){
    WebServerLoop();
  }else{
    MainProgLoop();
  }
}



