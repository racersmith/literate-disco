#include <Arduino.h>

class Status{
    private:
    unsigned long last_change = millis();
    int pin;
    unsigned long high_time;
    unsigned long low_time;
    bool one_shot = false;

    public:
    Status(int pin){
        this->pin = pin;
        pinMode(pin, OUTPUT);
    }

    void set_pulse(unsigned long high_time, unsigned long low_time, bool one_shot){
        this->high_time = high_time;
        this->low_time = low_time;
        this->one_shot = one_shot;
        if (one_shot){
            on();
        }
    }

    void toggle(){
        last_change = millis();
        digitalWrite(pin, !digitalRead(pin));
    }

    void on(){
        last_change = millis();
        digitalWrite(pin, HIGH);
    }

    void off(){
        last_change = millis();
        digitalWrite(pin, LOW);
    }

    void update(){
        bool state = digitalRead(pin);
        unsigned long now = millis();
        if (state){
            if (now - last_change > high_time){
                digitalWrite(pin, !state);
                last_change = now;
            }
        }
        else if (!one_shot){
            if (now - last_change > low_time){
                digitalWrite(pin, !state);
                last_change = now;
            }
        }
        
    }
};