#include "SimplebusMessage.h"

#ifndef SIMPLEBUS_RECEIVER_H_
#define SIMPLEBUS_RECEIVER_H_

class SimplebusReceiver{
  private: 
    int bus_pin;
    long long last_bit_time;
    int num_received_bits;
    uint32_t message;
    bool is_receiving_message;
    int num_ack_pulses;
    bool ack_received;
    bool is_message_pending;
    long long message_time;

    static SimplebusReceiver* instance;
    ICACHE_RAM_ATTR static void busCallbackAdapter(){ // TODO update with non-static std::function 
      instance->busCallback();
    }

    void busCallback(){
      if(DEBUG){
        Serial.println("Here!");
        Serial.flush();
      }
      long long now = millis();
      int duration = now - last_bit_time;
      if( duration>=START_PULSE_DURATION_MIN_MS && duration<=START_PULSE_DURATION_MAX_MS){
        this->message=0;
        this->num_received_bits=0;
        this->is_receiving_message=true;
      }
      if( duration>=SHORT_PULSE_DURATION_MIN_MS && duration<=SHORT_PULSE_DURATION_MAX_MS){
        if(this->isReceivingMessage()){
          this->message <<= 1;
          this->num_received_bits++;
        }else{
          this->num_ack_pulses++;
          if(this->num_ack_pulses==NUM_ACK_PULSES){ // Ack received?
            this->ack_received = true;
          }
        }
      }
      if( duration>=LONG_PULSE_DURATION_MIN_MS && duration<=LONG_PULSE_DURATION_MAX_MS){
        if(this->isReceivingMessage()){
          this->message <<= 1;
          this->message |= 1;
          this->num_received_bits++;
        }
      }
      if(this->num_received_bits == MESSAGE_LEN){ // Message received?
        this->is_message_pending = true;
        this->is_receiving_message = false;
        this->num_received_bits = 0;
        this->message_time = now;
      }
      if(DEBUG){
        Serial.print("DEBUG: last:[");
        Serial.print((int)this->last_bit_time);
        Serial.print("] now:[");
        Serial.print((int)now);
        Serial.println("]");
      }
      this->last_bit_time = now;
    }
  public:
    SimplebusReceiver(int pin){
      this->bus_pin = pin;
      this->last_bit_time = -1;
      this->num_received_bits = 0;
      this->message = 0;
      this->is_receiving_message = false;
      this->num_ack_pulses = 0;
      this->ack_received = false;
      this->is_message_pending = false;
      this->message_time = -1;
      SimplebusReceiver::instance = this;
      if(DEBUG){
        Serial.println("Setting receiver pin.");
        Serial.flush();
      }
      pinMode(pin, INPUT);
      if(DEBUG){
        Serial.println("Receiver pin set.");
        Serial.print("Receiver pin state ");
        Serial.print(digitalRead(pin));
        Serial.println(".");
        Serial.flush();
      }
      this->enableReceiver();
    }  
    SimplebusMessage getMessage(){
      return this->getMessage(NULL);
    }
    SimplebusMessage getMessage(long long* time){
      if(this->is_message_pending){
        if(time)
          *time = this->message_time;
        this->is_message_pending = false;
        return SimplebusMessage(this->message);
      }
      if(time)
        *time = -1;
      return SimplebusMessage();
    }
    bool getAck(){
      bool result = this->ack_received;
      if(result){
        this->ack_received = false;
        this->num_ack_pulses = 0;
      }
      return result;
    }
    void disableReceiver(){
      while( isReceivingMessage() )
        delay(5);
      detachInterrupt(digitalPinToInterrupt(this->bus_pin));
      if(DEBUG){
        Serial.println("Receiver disabled.");
        Serial.flush();
      }
    }
    void enableReceiver(){
      attachInterrupt(digitalPinToInterrupt(this->bus_pin), SimplebusReceiver::busCallbackAdapter, FALLING);
      if(DEBUG){
        Serial.println("Receiver enabled.");
        Serial.flush();
      }
    }
    bool isReceivingMessage(){
        if(millis()-last_bit_time > MAX_MESSAGE_IDLE_MS){
          this->is_receiving_message = false;
        }
        return this->is_receiving_message;
    }
};
SimplebusReceiver* SimplebusReceiver::instance= NULL;

#endif /* SIMPLEBUS_RECEIVER_H_ */