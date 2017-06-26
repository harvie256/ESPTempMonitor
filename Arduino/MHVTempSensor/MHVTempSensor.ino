#include <ESP8266WebServer.h>
#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <Ticker.h>

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

/*
 * A seperate timer is being used as a 'watchdog' to prevent system lockup
 * where one of the SDK functions is feeding the watchdog in an infinite loop.
 * See: https://github.com/esp8266/Arduino/issues/1532
 * 
 * Instead of treating it as a normal watchdog, it's being setup as a one-shot
 * affair with a timeout a few times longer than the normal data send loop time.
 */
Ticker tickerOSWatch;

void IsBootPowerOn()
{
  #ifdef DEBUG
    Serial.println(ESP.getResetReason());
  #endif
  bootModePowerOn = ESP.getResetReason().startsWith("External");
}

void ICACHE_RAM_ATTR osWatch(void) {
  ESP.reset();  // hard reset
}

void setup() {
  // Common setup for both modes of operation
  setupIO();
  Serial.begin ( 115200 );

  #ifdef DEBUG
  digitalWrite(DEBUG_PIN_1, HIGH);
  digitalWrite(DEBUG_PIN_2, HIGH);
  #endif

  IsBootPowerOn();
  if(bootModePowerOn){
    WebServerSetup();
  }else {
    tickerOSWatch.attach_ms(OSWATCH_RESET_TIME  * 1000, osWatch);
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



