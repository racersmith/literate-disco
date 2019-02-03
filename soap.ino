#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <WiFiNINA.h>

// #include "Secret.h"
#include "APDS.h"
#include "AK975X.h"
#include "AS7265X.h"
#include "aws.h"

#define SAMPLE_INDICATOR LED_BUILTIN

/*
These Secrets are defined withing the Secret.h file

WiFi
#define SECRET_SSID
#define SECRET_SSID_PASSWORD

Thinger Credentials
#define SECRET_USERNAME
#define SECRET_DEVICE_ID
#define SECRET_DEVICE_CREDENTIAL
*/

// const char ssid[]        = SECRET_SSID;
// const char pass[]        = SECRET_SSID_PASSWORD;
// const char broker[]      = SECRET_BROKER;
// const char* certificate  = SECRET_CERTIFICATE;

HumanPresenceSensor hps;
GestureSensor gesture;
SpectralSensor spectral;

unsigned long last_read;
unsigned long max_read_interval = 1000;

void setup() {
  Wire.setClock(400000);
  Serial.begin(9600);
  unsigned long wait_time = millis() + 10000;
  while(!Serial && millis() < wait_time);

  hps.Initialize();
  hps.SetMinSampleInterval(50);
  
  gesture.Initialize();
  gesture.SetMinSampleInterval(50);

  spectral.Initialize();
  spectral.SetMinSampleInterval(100);

  pinMode(SAMPLE_INDICATOR, OUTPUT);

  awsSetup();
  // awsHandle();
  
  last_read = millis();
  Serial.println("Setup Complete");
}


void loop() {
  awsHandle();
  test();
}

void sendHeader(unsigned long timestamp, unsigned int label){
  const size_t capacity = JSON_OBJECT_SIZE(8);
  StaticJsonBuffer<capacity> jsonBuffer;
  // DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.createObject();

  root["timestamp"] = timestamp;
  root["type"] = "header";
  JsonObject& data = root.createNestedObject("data");
  data["uid"] = uid;
  data["label"] = label;
  data["batt"] = analogRead(ADC_BATTERY);
  awsPublishMessage(root);
}

void sendHPS(unsigned long timestamp, unsigned long start_time, unsigned int label){
  const size_t capacity = JSON_OBJECT_SIZE(11);
  StaticJsonBuffer<capacity> jsonBuffer;
  // DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.createObject();
  root["timestamp"] = timestamp;
  root["dt_start"] = millis() - start_time;
  hps.BlockingRead(root);
  root["dt_end"] = millis() - start_time;
  awsPublishMessage(root);
}

void sendGesture(unsigned long timestamp, unsigned long start_time, unsigned int label){
  const size_t capacity = JSON_OBJECT_SIZE(12);
  StaticJsonBuffer<capacity> jsonBuffer;
  // DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.createObject();
  root["timestamp"] = timestamp;
  root["dt_start"] = millis() - start_time;
  gesture.BlockingRead(root);
  root["dt_end"] = millis() - start_time;
  awsPublishMessage(root);
}

void sendSpectral(unsigned long timestamp, unsigned long start_time, unsigned int label){
  const size_t capacity = JSON_OBJECT_SIZE(27);
  StaticJsonBuffer<capacity> jsonBuffer;
  // DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.createObject();
  root["timestamp"] = timestamp;
  root["dt_start"] = millis() - start_time;
  spectral.BlockingRead(root);
  root["dt_end"] = millis() - start_time;
  awsPublishMessage(root);
}


void test(){
  if(gesture.IsActive() && millis() > last_read + max_read_interval){
    spectral.Illuminate(true);
    spectral.triggerRead();
    digitalWrite(SAMPLE_INDICATOR, HIGH);
    
    unsigned long timestamp = getTime();
    unsigned long start_time = millis();
    unsigned int label = 0;

    sendHeader(timestamp, label);
    sendHPS(timestamp, start_time, label);    
    sendGesture(timestamp, start_time, label);
    sendSpectral(timestamp, start_time, label);

    spectral.Illuminate(false);
    digitalWrite(SAMPLE_INDICATOR, LOW);

    last_read = millis();
  }
}
