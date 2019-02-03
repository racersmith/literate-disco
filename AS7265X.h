#include <SparkFun_AS7265X.h> //Click here to get the library: http://librarymanager/All#SparkFun_AS7265X
#include <Wire.h>
#include <ArduinoJson.h>


class SpectralSensor: private AS7265X{
  private:
    uint16_t millis_between_samples = 0;
    uint16_t next_read_millis = millis();
    
    uint8_t int_cycles_dark = 64;  //64 is the shortest time that gives full resolution
    uint8_t int_cycles_illum = 20; //20 is the default...

    void Read(JsonObject& sensor){
      sensor["sensor"] = "AS7265X";
      // sensor["time"] = millis();
      JsonObject& sensor_data = sensor.createNestedObject("data");
      
      // UV
      sensor_data["A"] = getCalibratedA();  // A = 410nm
      sensor_data["B"] = getCalibratedB();  // B = 435nm
      sensor_data["C"] = getCalibratedC();  // C = 460nm
      sensor_data["D"] = getCalibratedD();  // D = 485nm
      sensor_data["E"] = getCalibratedE();  // E = 510nm
      sensor_data["F"] = getCalibratedF();  // F = 535nm
      sensor_data["tempUV"] = getTemperature(AS72653_UV);
    
      // Visible
      sensor_data["G"] = getCalibratedG();  // G = 560nm
      sensor_data["H"] = getCalibratedH();  // H = 585nm
      sensor_data["I"] = getCalibratedI();  // I = 645nm
      sensor_data["J"] = getCalibratedJ();  // J = 705nm
      sensor_data["K"] = getCalibratedK();  // K = 900nm
      sensor_data["L"] = getCalibratedL();  // L = 940nm
      sensor_data["tempV"] = getTemperature(AS72652_VISIBLE);

      // NIR
      sensor_data["R"] = getCalibratedR();  // R = 610nm
      sensor_data["S"] = getCalibratedS();  // S = 680nm
      sensor_data["T"] = getCalibratedT();  // T = 730nm
      sensor_data["U"] = getCalibratedU();  // U = 760nm
      sensor_data["V"] = getCalibratedV();  // V = 810nm
      sensor_data["W"] = getCalibratedW();  // W = 860nm
      sensor_data["tempNIR"] = getTemperature(AS72651_NIR);
    }

    void Read(JsonArray& json_array){
      JsonObject& sensor = json_array.createNestedObject();
      Read(sensor);
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
      
      // Illuminate(false);
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
        enableBulb(AS7265x_LED_WHITE);
        enableBulb(AS7265x_LED_IR);
        enableBulb(AS7265x_LED_UV);
        disableIndicator();
        setIntegrationCycles(int_cycles_illum);
      }
      else{
        disableBulb(AS7265x_LED_WHITE);
        disableBulb(AS7265x_LED_IR);
        disableBulb(AS7265x_LED_UV);
        enableIndicator();
        setIntegrationCycles(int_cycles_dark);
      }
        
    }

    bool available(){
      return dataAvailable();
    }

    void AvailableRead(JsonArray& data){
      if(dataAvailable()){
        Read(data);
      }
    }

    void triggerRead(uint8_t int_cycles=20){
      setIntegrationCycles(int_cycles);
      setMeasurementMode(AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT);
      // getG();
      // getR();
      // getA();
    }

    void BlockingRead(JsonArray& data){
      while(!dataAvailable());
      Read(data);
    }

    void BlockingRead(JsonObject& data){
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
