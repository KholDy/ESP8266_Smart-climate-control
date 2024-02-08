#include <WiFiManager.h>
#include <ESP8266WiFi.h>                                
#include <Wire.h>                                       
#include <Adafruit_BME280.h>                            
#include <Adafruit_Sensor.h>                            
#include <ArduinoJson.h>                                
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
 
#define WELCOME "Welcome to the climate sensor!\n"\
                "To get climate datas send GET HTTP request: ip/data."
#define ID 1
#define DESCRIPTION "Sensor of temperature, pressure, humidity"
 
Adafruit_BME280 bme;                                                        
 
ESP8266WebServer server(80);                      

DynamicJsonDocument data(512);

//*---------------------Check state led----------------------------------------------------------t
void getData() {
  data["ip"] = WiFi.localIP();
  data["temperature"] = bme.readTemperature();
  data["pressure"] = (bme.readPressure() / 100.0F) * 0.750062;
  data["humidity"] = bme.readHumidity();

  String buf;
  serializeJson(data, buf);
  server.send(200, F("application/json"), buf);
}

//*----------------------Define routing---------------------------------------------------------
void restServerRouting() {
  server.on("/", HTTP_GET, []() {
    server.send(200, F("text/html"),
      F(WELCOME));
  });
  // handle post request
  server.on(F("/data"), HTTP_GET, getData);
}

//*--------------------Manage not found URL-----------------------------------------------------
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
 
void setup() {
  data["id"] = ID;
  data["ip"] = "null";
  data["description"] = DESCRIPTION;
  data["temperature"] = "null";
  data["pressure"] = "null";
  data["humidity"] = "null";

  //*------------------Checking the sensor-----------------------------------------------------                                                    
  if (!bme.begin(0x76)) {                                                  
    Serial.println("Could not find a valid BME280 sensor, check wiring!"); 
    while (1);                                                             
  }
 
  WiFi.mode(WIFI_STA);

    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    //wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    //res = wm.autoConnect(); // auto generated AP name from chipid
    //res = wm.autoConnect("Kitchen-light"); // anonymous ap
    res = wm.autoConnect("The sensor of climate:","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }

    // Set server routing
    restServerRouting();
    // Set not found response
    server.onNotFound(handleNotFound);
    // Start server
    server.begin();                         
}
 
void loop(){
  server.handleClient();
}