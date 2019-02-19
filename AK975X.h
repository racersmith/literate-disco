#include <Wire.h>

 //Use Library Manager or download here: https://github.com/sparkfun/SparkFun_AK975X_Arduino_Library
// #include <SparkFun_AK975X_Arduino_Library.h>
#include <SparkFun_AK9750_Arduino_Library.h>
#include <ArduinoJson.h>

class HumanPresenceSensor: private AK9750{
  private:
    uint16_t millis_between_samples = 0;
    uint16_t next_read_millis = millis();
  
    void Read(JsonArray& json_array){
      JsonObject& sample = json_array.createNestedObject();
      Read(sample);
    }

    void Read(JsonObject& sample){
      sample["type"] = F("AK975X");
      
      JsonObject& data = sample.createNestedObject("data");
      data["zone_1"] = getIR1();
      data["zone_2"] = getIR2();
      data["zone_3"] = getIR3();
      data["zone_4"] = getIR4();
      data["tempHPS"] = getTemperature()/10;
      refresh();
    }
    
  public:
    void Initialize(){
      if(begin()){
        Serial.println(F("AK975X - Human Presence Sensor Initiallized"));
        refresh();
      }
      else{
        Serial.println(F("## AK975X - Human Presence Sensor FAILED ##"));
      }
    }

    void SetMinSampleInterval(uint8_t sample_interval){
      millis_between_samples = sample_interval;
    }
  
    void AvailableRead(JsonArray& data){
      if(available()){
        Read(data);
      }
    }

    void BlockingRead(JsonArray& data){
      while(!available());
      Read(data);
    }

    void BlockingRead(JsonObject& data){
      while(!available());
      Read(data);
    }
    
    void ScheduledRead(JsonArray& data){
      if (millis() >= next_read_millis){
        BlockingRead(data);
        next_read_millis = millis() + millis_between_samples;
      }
    }
};
