
#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "constants.h"


// You can use either the url for dns lookup, or the thingspeak static IP
// const char* thingspeakUrl = "api.thingspeak.com";
const char* thingspeakUrl = "184.106.153.149";

WiFiClient client;

#define SLEEP_TIME (60*1000000)

char wifi_ssid[32];
char wifi_password[32];
IPAddress ip_address;
IPAddress netmask;
IPAddress gateway;
char apikey[20];
char readBuffer[20];

void _readCfgFile(){
  
  if (!SPIFFS.begin())
  {
    // TODO Needs error handling...
    Serial.println("SPIFFS Mount failed");
  }
    
  File f = SPIFFS.open("/config/webcfg.txt", "r");
  if (!f) {
    // TODO Error handle there not being a config file...
    Serial.println("file open failed");
    return;
  }

  readLineIntoCharArray(f, wifi_ssid, 32);
  readLineIntoCharArray(f, wifi_password, 32);

  ip_address.fromString(readLineAndTrim(f));
  netmask.fromString(readLineAndTrim(f));
  gateway.fromString(readLineAndTrim(f));

  readLineIntoCharArray(f, apikey, 20);
  f.close();
}

void _postToThingspeak(){
  #ifdef DEBUG
  digitalWrite(DEBUG_PIN_2, LOW);
  #endif
  
    if (client.connect(thingspeakUrl,80)) {
  String postStr = "/update?key=" + String(apikey);
      postStr +="&field1=";
      postStr += String(temp);
      postStr +="&field2=";
      postStr += String(batteryVoltage);
      postStr += "\r\n\r\n";
  
    client.print("GET " + postStr + " HTTP/1.1\r\n");
    client.print("Host: api.thingspeak.com\r\n");
    client.print("Connection: close\n");
  #ifdef DEBUG
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print(" degrees Celcius : ");
    Serial.print(batteryVoltage);
    Serial.println("\nsend to Thingspeak");

    digitalWrite(DEBUG_PIN_2, HIGH);

  #endif
  }

  client.stop();
}

void _setupWifi(){
  _readCfgFile();
  WiFi.mode(WIFI_STA);
  WiFi.config(ip_address, gateway, netmask);
  WiFi.begin ( wifi_ssid, wifi_password );
   //Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 ); Serial.print ( "." );
  }
  #ifdef DEBUG
  Serial.println ( "wifi ok" );
  Serial.print ( "Connected to " ); Serial.println ( wifi_ssid );
  Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );
  #endif
}


void MainProgSetup(){

  _setupWifi();

  updateTemperature();
  updateBatteryVoltage();

  _postToThingspeak();
}

void MainProgLoop(){
    #ifdef DEBUG
      digitalWrite(DEBUG_PIN_1, LOW);
    #endif
    ESP.deepSleep(SLEEP_TIME,WAKE_NO_RFCAL);
  }
 

