#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "env.h"


void setup(){
 Serial.begin(115200); 
 pinMode(LED_PIN,OUTPUT);
 analogReadResolution(12);

 WiFi.begin(SSID, PASS);

 while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");   
 }

 Serial.print("Connected to the Wifi Network with IP addess: ");
 Serial.println(WiFi.localIP());
}

float readTemperature() {
  int sensor_value = analogRead(TEMP_SENSOR);
  Serial.print("Analog value: ");
  Serial.println(sensor_value);

  float voltage = (sensor_value / 1023.0) * 3.3;  // Use 5.0 if using 5V reference
  Serial.print("Voltage: ");
  Serial.println(voltage, 3);

  float temperature = (voltage - 0.5) * 100.0;
  return temperature;
}


void loop(){
    if (WiFi.status()== WL_CONNECTED){
        HTTPClient httpGet;
        httpGet.begin(ENDPOINT);

        httpGet.addHeader("api-key", API_KEY);

        int responseCode = httpGet.GET();

        if (responseCode <= 0){
            Serial.print("An error occured with code: ");
            Serial.println(responseCode);
            httpGet.end();
            return;
        }

    Serial.print("HTTP Response Code: ");
    Serial.println(responseCode);

    String response = httpGet.getString();
    Serial.println(response);
    httpGet.end();


    JsonDocument object;

    DeserializationError error = deserializeJson(object, response);

    if (error) {
     Serial.print("deserializeJson() failed: ");
     Serial.println(error.c_str());
     return;
    }

  bool light = object["light"]; 
  digitalWrite(LED_PIN, light);

  float temperature = readTemperature();

  HTTPClient httpPut;
  httpPut.begin(PUT_ENDPOINT);
  httpPut.addHeader("Content-Type", "application/json");
  httpPut.addHeader("api-key", API_KEY);
        
  object.clear(); 
  object["temp"] = temperature;
  
  String request_body;
  serializeJson(object, request_body);
           
  int status_code = httpPut.PUT(request_body);

  if (status_code <= 0){
    Serial.print("An error occured with code: ");
    Serial.println(status_code);
    httpPut.end();
    return;
}

Serial.print("Status code: ");
Serial.println(status_code);
String response_body = httpPut.getString();
Serial.println(response_body);
httpPut.end();

Serial.printf("Temperature: %.2f°C\n", temperature);
  }
  
  delay(5000);
}