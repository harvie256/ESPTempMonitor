
#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "constants.h"

const char* thingspeakUrl = "api.thingspeak.com";

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
    if (client.connect(thingspeakUrl,80)) {
  String postStr = String(apikey);
      postStr +="&field1=";
      postStr += String(temp);
      postStr +="&field2=";
      postStr += String(batteryVoltage);
      postStr += "\r\n\r\n";
  
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+ String(apikey) +"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print(" degrees Celcius : ");
    Serial.print(batteryVoltage);
    Serial.println("\nsend to Thingspeak");
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
    //WiFi connexion is OK
  Serial.println ( "wifi ok" );
  Serial.print ( "Connected to " ); Serial.println ( wifi_ssid );
  Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );
}


void MainProgSetup(){
  setupIO();
  Serial.begin ( 115200 );
  _setupWifi();

  updateTemperature();
  updateBatteryVoltage();

  _postToThingspeak();
}

void MainProgLoop(){
    ESP.deepSleep(SLEEP_TIME,WAKE_NO_RFCAL);
  }
 

