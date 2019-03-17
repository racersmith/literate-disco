#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <WiFiNINA.h>

// #include "Secret.h"
#include "APDS.h"
#include "AK975X.h"
#include "AS7265X.h"
#include "aws.h"
#include "label_switch.h"
#include "status.h"

#define SAMPLE_INDICATOR LED_BUILTIN

/*
These Secrets are defined withing the Secret.h file

const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_SSID_PASSWORD;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;
*/

HumanPresenceSensor hps;
GestureSensor gesture;
SpectralSensor spectral;
LabelSwitch label_switch;
Status led_status(LED_BUILTIN);

unsigned long last_read;
unsigned long max_read_interval = 1000;

void setup() {
  Wire.setClock(400000);
  led_status.toggle();

  Serial.begin(115200);
  led_status.toggle();

  unsigned long wait_time = millis() + 2000;
  led_status.set_pulse(100, 100, false);
  while(!Serial && millis() < wait_time){
    led_status.update();
  }
  led_status.toggle();

  hps.Initialize();
  hps.SetMinSampleInterval(50);
  led_status.toggle();
  
  gesture.Initialize();
  gesture.SetMinSampleInterval(50);
  led_status.toggle();

  spectral.Initialize();
  spectral.SetMinSampleInterval(100);
  led_status.toggle();

  awsSetup();
  led_status.toggle();
  
  awsHandle();
  led_status.toggle();
  
  while(gesture.IsActive()){}
  led_status.toggle();

  last_read = millis();
  Serial.println("Setup Complete");
  
  led_status.set_pulse(50, 1000, false);
}

void loop() {
  led_status.update();
  awsHandle();
  sampleHandle();
}

void sendHeader(unsigned long timestamp, unsigned int label){
  const size_t capacity = JSON_OBJECT_SIZE(8);
  // StaticJsonBuffer<capacity> jsonBuffer;
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.createObject();

  root["timestamp"] = timestamp;
  root["type"] = "header";
  JsonObject& data = root.createNestedObject("data");
  data["uid"] = uid;
  data["label"] = label;
  data["batt"] = analogRead(ADC_BATTERY);
  awsPublishMessage(root);
}

void sendHPS(unsigned long timestamp, unsigned long start_time){
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

void sendGesture(unsigned long timestamp, unsigned long start_time){
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

void sendSpectral(unsigned long timestamp, unsigned long start_time){
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

void sendSpectralUV(unsigned long timestamp, unsigned long start_time){
  const size_t capacity = JSON_OBJECT_SIZE(27);
  StaticJsonBuffer<capacity> jsonBuffer;
  // DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.createObject();
  root["timestamp"] = timestamp;
  root["dt_start"] = millis() - start_time;
  spectral.ReadUV(root);
  root["dt_end"] = millis() - start_time;
  awsPublishMessage(root);
}

void sendSpectralVis(unsigned long timestamp, unsigned long start_time){
  const size_t capacity = JSON_OBJECT_SIZE(27);
  StaticJsonBuffer<capacity> jsonBuffer;
  // DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.createObject();
  root["timestamp"] = timestamp;
  root["dt_start"] = millis() - start_time;
  spectral.ReadVis(root);
  root["dt_end"] = millis() - start_time;
  awsPublishMessage(root);
}

void sendSpectralNIR(unsigned long timestamp, unsigned long start_time){
  const size_t capacity = JSON_OBJECT_SIZE(27);
  StaticJsonBuffer<capacity> jsonBuffer;
  // DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.createObject();
  root["timestamp"] = timestamp;
  root["dt_start"] = millis() - start_time;
  spectral.ReadNIR(root);
  root["dt_end"] = millis() - start_time;
  awsPublishMessage(root);
}


void sampleHandle(){
  if(gesture.IsActive() && millis() > last_read + max_read_interval){
    spectral.Illuminate(true);
    spectral.triggerRead();
    led_status.on();
    
    unsigned long timestamp = getTime();
    unsigned long start_time = millis();
    unsigned int label = label_switch.get_label();

    sendHeader(timestamp, label);
    sendHPS(timestamp, start_time);    
    sendGesture(timestamp, start_time);
    // sendSpectral(timestamp, start_time);
    spectral.WaitForData();
    sendSpectralUV(timestamp, start_time);
    sendSpectralVis(timestamp, start_time);
    sendSpectralNIR(timestamp, start_time);


    spectral.Illuminate(false);
    led_status.off();

    last_read = millis();
  }
}
