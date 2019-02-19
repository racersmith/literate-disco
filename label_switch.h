#include <Arduino.h>

class LabelSwitch{
  private:
  int source_pin = 0;  // this pin will be used as a ground.  because, lazy.
  int n = 4;
  int bit_pins[4] = {4, 3, 2, 1}; // in bit order
    
  public:
  LabelSwitch(){
    pinMode(source_pin, OUTPUT);
    digitalWrite(source_pin, LOW);

    for (int i=0; i<n; i++){
      pinMode(bit_pins[i], INPUT_PULLUP);
    }
    Serial.println("Label Reader Ready");
  }

  int get_state(){
    int label = 0;
    for (int i=0; i<n; i++){
      label |= !digitalRead(bit_pins[i]) << n-i-1;
    }
    return label;
  }
  
};