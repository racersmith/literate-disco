
#include <ArduinoJson.h>
// #include <WiFiNINA.h>
// #include <ThingerWifi101.h>
#include "Secret.h"
#include "APDS.h"
#include "AK975X.h"
#include "AS7265X.h"

#define SAMPLE_INDICATOR LED_BUILTIN

#define USERNAME  SECRET_USERNAME
#define DEVICE_ID SECRET_DEVICE_ID
#define DEVICE_CREDENTIAL SECRET_DEVICE_CREDENTIAL

#define SSID  SECRET_SSID
#define SSID_PASSWORD SECRET_SSID_PASSWORD

// ThingerWifi101 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

HumanPresenceSensor hps;
GestureSensor gesture;
SpectralSensor spectral;

void setup() {
  Serial.begin(9600);
  while(!Serial);

  hps.Initialize();
  hps.SetMinSampleInterval(50);
  
  gesture.Initialize();
  gesture.SetMinSampleInterval(50);

  spectral.Initialize();
  spectral.SetMinSampleInterval(100);

  pinMode(SAMPLE_INDICATOR, OUTPUT);
  
  Serial.println("Setup Complete");
}


void loop() {
  test();
  Serial.println();
  delay(1000);
}


void test(){
  const size_t bufferSize = 2000;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  root["label"] = 0;
  
  JsonArray& data = root.createNestedArray("data");

  while(!gesture.IsActive()){
    delay(5);
  }

  spectral.Illuminate(true);
  digitalWrite(SAMPLE_INDICATOR, HIGH);
  while(gesture.IsActive()){
    hps.ScheduledRead(data);
    
    if(!gesture.IsActive()) break;
    
    gesture.ScheduledRead(data);
    
    if(!gesture.IsActive()) break;
    
    spectral.AvailableRead(data);
  }
  spectral.Illuminate(false);
  digitalWrite(SAMPLE_INDICATOR, LOW);
  
  root.printTo(Serial);
}
