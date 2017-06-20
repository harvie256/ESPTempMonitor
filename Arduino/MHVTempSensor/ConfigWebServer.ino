
ESP8266WebServer server ( 80 );


#include <ESP8266WebServer.h>
#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include "constants.h"

#define ledToggleIntervalMs 500
#define tempIntervalMs 5000

#define PASSWORD  "password"      // WiFi password

extern float temp;
extern float batteryVoltage;

#define ConfigurationServerTimeout 120000
bool ConfigurationServerTimeoutEnabled = 0;

void generateAndSendJson(){
  char sendBuffer[512];
  DynamicJsonBuffer jsonBuffer(512);
  File f = SPIFFS.open("/config/webcfg.txt", "r");
  if (!f) {
    // TODO Error handle there not being a config file...
    Serial.println("file open failed");
    return;
  }
  JsonObject& root = jsonBuffer.createObject();
  root["SSID"] = readLineAndTrim(f);
  root["password"] = "";
  f.readStringUntil('\n'); // Will not send back password...
  root["ipAddress"] = readLineAndTrim(f);
  root["netmask"] = readLineAndTrim(f);
  root["gateway"] = readLineAndTrim(f);
  root["apikey"] = readLineAndTrim(f);
  root.printTo(sendBuffer, sizeof(sendBuffer));
  server.send(200, "application/json", sendBuffer);
  f.close();
}

void processWebConfigSave(){
  DynamicJsonBuffer jsonBuffer(512);
  JsonObject& root = jsonBuffer.parseObject(server.arg("plain"));
  File f = SPIFFS.open("/config/webcfg.txt", "w");
  if (!f) {
      // TODO Error handle there not being a config file...
      Serial.println("file open failed");
  }

  f.println(root["SSID"].as<String>());
  f.println(root["password"].as<String>());
  f.println(root["ipAddress"].as<String>());
  f.println(root["netmask"].as<String>());
  f.println(root["gateway"].as<String>());
  f.println(root["apikey"].as<String>());
  f.close();

  server.send(200, "text/plain", "");
}

void WebServerSetupWiFi()
{
  WiFi.mode(WIFI_AP);

  char AP_Name[32];
  sprintf (AP_Name, "MHV Sensor %08X", ESP.getChipId());
  WiFi.softAP(AP_Name, PASSWORD);
}


void WebServerSetup() {
  setupIO();
  Serial.begin ( 115200 );

  WebServerSetupWiFi();
  
  if (!SPIFFS.begin())
  {
    // TODO Needs error handling...
    Serial.println("SPIFFS Mount failed");
  } else {
    Serial.println("SPIFFS Mount succesfull");
  }

  server.serveStatic("/js", SPIFFS, "/js", "max-age=86400");
  server.serveStatic("/html", SPIFFS, "/html");
  server.serveStatic("/css", SPIFFS, "/css", "max-age=86400");
  server.serveStatic("/", SPIFFS, "/html/index.html");

  server.on("/sensor", [](){
    ConfigurationServerTimeoutEnabled = 0; // This will get called automatically when the index page is opened.  There probably is a cleaner way in code to set this.
    server.send(200, "application/json", "{ \"BatteryVoltage\": " + String(batteryVoltage,2) + ", \"Temperature\": "+String(temp,2)+" }");
  });

  server.on("/config", HTTP_GET, generateAndSendJson);
  server.on("/config", HTTP_POST, processWebConfigSave);
  server.on("/reset", [](){ESP.reset(); });
  


  server.onNotFound([](){
      server.send(404, "text/plain", "FileNotFound");
  });

  server.begin();
  Serial.println ( "HTTP server started" );

  // Check if webcfg file exists, if not there's no point restarting to run mode
  ConfigurationServerTimeoutEnabled = SPIFFS.exists("/config/webcfg.txt");

}

void WebServerLoop() {
  static unsigned long ledTimer = 0;
  static unsigned long tempTimer = 0;

  server.handleClient();

  if(millis() > ledTimer)
  {
    ledTimer = millis() + ledToggleIntervalMs;
    if(digitalRead(LED_PIN))
      digitalWrite(LED_PIN, LOW);
    else
      digitalWrite(LED_PIN,HIGH);
  }

  if(millis() > tempTimer)
  {
    tempTimer = millis() + tempIntervalMs;

    updateBatteryVoltage();
    updateTemperature();

  }

  if(ConfigurationServerTimeoutEnabled && (millis() > 120000))
  {
      ESP.reset();
  }
  
}


