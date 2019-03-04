#include <SparkFun_APDS9960.h>
#include <Wire.h>
#include <ArduinoJson.h>

class GestureSensor: private SparkFun_APDS9960{
  private:
  uint16_t ambient_light = 0;
  uint16_t red_light = 0;
  uint16_t green_light = 0;
  uint16_t blue_light = 0;
  uint8_t proximity_data = 0;
  
  uint8_t rising_threshold = 180;
  uint8_t falling_threshold = 80;
  bool threshold_rising = true;
  
  bool up_to_date = false;

  uint16_t millis_between_samples = 0;
  uint16_t next_read_millis = millis();


  void Update(){
    up_to_date = true;
        
    readProximity(proximity_data);
    readAmbientLight(ambient_light);
    readRedLight(red_light);
    readGreenLight(green_light);
    readBlueLight(blue_light);
    }

    
  void Read(JsonArray& json_array){
    JsonObject& sample = json_array.createNestedObject();
    Read(sample);
    }

  void Read(JsonObject& sample){
    if(!up_to_date){
      Update();
    }
    
    sample["type"] = "APDS9960";
    JsonObject& data = sample.createNestedObject("data");
    data["proximity"] = proximity_data;
    data["ambient"] = ambient_light;
    data["red"] = red_light;
    data["green"] = green_light;
    data["blue"] = blue_light;
    
    up_to_date = false;
    }
    
  public:
    void Initialize(){
      if (init()){
        Update();
        up_to_date = false;
        Serial.println(F("APDS9960 - Gesture Sensor Initiallized"));
      }
      else{
        Serial.println(F("## APDS9960 - Gesture Sensor FAILED ##"));
      }

      // Start running the APDS-9960 light sensor (no interrupts)
      // bool is to enable hardware interrupts.
      if (enableLightSensor(false)) {
        Serial.println(F("Light sensor is now running"));
      } else {
        Serial.println(F("Something went wrong during light sensor init!"));
      }
    
      // Adjust the Proximity sensor gain
      if (!setProximityGain(PGAIN_2X)) {
        Serial.println(F("Something went wrong trying to set PGAIN"));
      }
    
      // Start running the APDS-9960 proximity sensor (no interrupts)
      if (enableProximitySensor(false)) {
        Serial.println(F("Proximity sensor is now running"));
      } else {
        Serial.println(F("Something went wrong during sensor init!"));
      }
  
      // Start running the APDS-9960 gesture sensor engine
//      if(enableGestureSensor(false)) {
//        Serial.println(F("Gesture sensor is now running"));
//      } else {
//        Serial.println(F("Something went wrong during gesture sensor init!"));
//      }
    }

 
    void BlockingRead(JsonArray& data){
      Read(data);
    }

    void BlockingRead(JsonObject& data){
      Read(data);
    }


    void SetMinSampleInterval(uint8_t sample_interval){
      millis_between_samples = sample_interval;
    }


    void ScheduledRead(JsonArray& data){
      if (millis() >= next_read_millis){
        BlockingRead(data);
        next_read_millis = millis() + millis_between_samples;
      }
    }


    void SetThresholds(uint8_t rising, uint8_t falling){
      rising_threshold = rising;
      falling_threshold = falling;
      threshold_rising = true;
    }


    bool IsActive(){
      readProximity(proximity_data);

      if (threshold_rising){
        if (proximity_data > rising_threshold){
          threshold_rising = false;
          return true;
        }
        else{
          return false;
        }
      }
      else{
        if (proximity_data < falling_threshold){
          threshold_rising = true;
          return false;
        }
        else{
          return true;
        }
      }
    }
};
