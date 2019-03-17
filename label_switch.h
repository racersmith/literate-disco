#include <Arduino.h>

class LabelSwitch{
  private:
  int source_pin = 10;  // this pin will be used as a ground.  because, lazy.
  int n = 4;
  int bit_pins[4] = {6, 7, 8, 9}; // in bit order
  
  bool last_label = -1;
    
  public:
  LabelSwitch(){
    pinMode(source_pin, OUTPUT);
    digitalWrite(source_pin, LOW);

    for (int i=0; i<n; i++){
      pinMode(bit_pins[i], INPUT_PULLUP);
    }
  }

  int get_label(){
    int label = 0;
    for (int i=0; i<n; i++){
      label |= !digitalRead(bit_pins[i]) << n-i-1;
    }
    return label;
  }

  bool is_new(){
    int label = get_label();
    if (last_label != label){
      last_label = label;
      return true;
    }
    else {
      return false;
    }
  }
  
};