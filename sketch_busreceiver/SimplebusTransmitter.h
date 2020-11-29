#include "SimplebusMessage.h"

#ifndef SIMPLEBUS_TRANSMITTER_H_
#define SIMPLEBUS_TRANSMITTER_H_

#define SB_HIGH LOW
#define SB_LOW HIGH

class SimplebusTransmitter{
  private:
    int bus_pin;
    void transmitValue(bool value){
      digitalWrite(this->bus_pin, SB_LOW);
      delay(LOW_TIME_MS);
      digitalWrite(this->bus_pin, SB_HIGH);
      delay(value?HIGH_TIME_LONG_MS:HIGH_TIME_SHORT_MS);
    }
    void transmitStart(){
      digitalWrite(this->bus_pin, SB_LOW);
      delay(LOW_TIME_MS);
      digitalWrite(this->bus_pin, SB_HIGH);
      delay(HIGH_TIME_START_MS);
    }
    void transmitEnd(){
      digitalWrite(this->bus_pin, SB_LOW);
      delay(LOW_TIME_MS);
      digitalWrite(this->bus_pin, SB_HIGH);
    }
  public:
    SimplebusTransmitter(int pin){
      this->bus_pin = pin;
      if(DEBUG){
        Serial.println("Setting transmitter pin.");
        Serial.flush();
      }
      digitalWrite(this->bus_pin, SB_HIGH);
      if(DEBUG){
        Serial.println("Setting transmitter pin.");
        Serial.flush();
      }
      pinMode(this->bus_pin, OUTPUT);
      if(DEBUG){
        Serial.println("Transmitter pin set.");
        Serial.flush();
      }
    }
    void putMessage(SimplebusMessage message){
      this->transmitStart();
      for(int i=CMD_LEN-1; i>=0; i--)
        this->transmitValue((message.command>>i)&1);
      for(int i=ID_LEN-1; i>=0; i--)
        this->transmitValue((message.id>>i)&1);
      for(int i=CHK_LEN-1; i>=0; i--)
        this->transmitValue((message.checksum>>i)&1);
      this->transmitEnd();
    }
    void putMessage(uint8_t command, uint8_t id){
      SimplebusMessage message(command, id);
      this->putMessage(message);
    }
    void putAck(){
      transmitValue(0);
      transmitValue(0);
      transmitValue(0);
      transmitValue(0);
    }
};

#endif /* SIMPLEBUS_TRANSMITTER_H_ */