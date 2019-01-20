#include <SparkFun_AS7265X.h> //Click here to get the library: http://librarymanager/All#SparkFun_AS7265X
#include <Wire.h>
#include <ArduinoJson.h>


class SpectralSensor: private AS7265X{
  private:
    uint16_t millis_between_samples = 0;
    uint16_t next_read_millis = millis();
    
    uint8_t int_cycles_dark = 64;  //64 is the shortest time that gives full resolution
    uint8_t int_cycles_illum = 20; //20 is the default...

    void Read(JsonArray& json_array){
      JsonObject& sensor = json_array.createNestedObject();
      sensor["sensor"] = "AS7265X";
      sensor["time"] = millis();
      JsonObject& sensor_data = sensor.createNestedObject("data");
      
      // UV
      sensor_data["A"] = getCalibratedA();
      sensor_data["B"] = getCalibratedB();
      sensor_data["C"] = getCalibratedC();
      sensor_data["D"] = getCalibratedD();
      sensor_data["E"] = getCalibratedE();
      sensor_data["F"] = getCalibratedF();
    
      // Visible
      sensor_data["G"] = getCalibratedG();
      sensor_data["H"] = getCalibratedH();
      sensor_data["I"] = getCalibratedI();
      sensor_data["J"] = getCalibratedJ();
      sensor_data["K"] = getCalibratedK();
      sensor_data["L"] = getCalibratedL();
    
      // NIR
      sensor_data["R"] = getCalibratedR();
      sensor_data["S"] = getCalibratedS();
      sensor_data["T"] = getCalibratedT();
      sensor_data["U"] = getCalibratedU();
      sensor_data["V"] = getCalibratedV();
      sensor_data["W"] = getCalibratedW();
    }
    
  public:
    void Initialize(){
      if(begin()){
        Serial.println(F("AS7265X - Spectral Sensor Initiallized"));
      }
      else{
        Serial.println(F("## AS7265X - Spectral Sensor FAILED ##"));
      }

      setIntegrationCycles(int_cycles_dark);
      setMeasurementMode(AS7265X_MEASUREMENT_MODE_6CHAN_CONTINUOUS);

      setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_WHITE);
      setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_IR);
      setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_UV);
      setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_1MA);
    }


    void SetMinSampleInterval(uint8_t sample_interval){
      millis_between_samples = sample_interval;
    }


    void Illuminate(bool on){
      if(on){
        setIntegrationCycles(int_cycles_illum);
        disableIndicator();
        enableBulb(AS7265x_LED_WHITE);
        enableBulb(AS7265x_LED_IR);
        enableBulb(AS7265x_LED_UV);
      }
      else{
        setIntegrationCycles(int_cycles_dark);
        enableIndicator();
        disableBulb(AS7265x_LED_WHITE);
        disableBulb(AS7265x_LED_IR);
        disableBulb(AS7265x_LED_UV);
      }
        
    }

  
    void AvailableRead(JsonArray& data){
      if(dataAvailable()){
        Read(data);
      }
    }


    void BlockingRead(JsonArray& data){
      while(!dataAvailable());
      Read(data);
    }

    
    void ScheduledRead(JsonArray& data){
      if (millis() >= next_read_millis){
        BlockingRead(data);
        next_read_millis = millis() + millis_between_samples;
      }
    }
};
