#include "Logger.h"
#include "SimplebusReceiver.h"
#define DEBUG false

ICACHE_RAM_ATTR void SimplebusReceiver::busCallbackAdapter(){ // TODO update with non-static std::function 
  instance->busCallback();
}

void SimplebusReceiver::busCallback(){
  if(DEBUG){
    logger->log(false, "DEBUG: ");
  }
  long unsigned now = millis();
  int duration = now - last_bit_time;
  if( duration>=START_PULSE_DURATION_MIN_MS && duration<=START_PULSE_DURATION_MAX_MS){
    this->message=0;
    this->num_received_bits=0;
    this->is_receiving_message=true;
    this->num_ack_pulses=0;
    if(DEBUG){
      logger->log(false, "START ");
    }
  }
  if( duration>=SHORT_PULSE_DURATION_MIN_MS && duration<=SHORT_PULSE_DURATION_MAX_MS){
    if(this->isReceivingMessage()){
      this->message <<= 1;
      this->num_received_bits++;
    }else{
      this->num_ack_pulses++;
      if(this->num_ack_pulses==NUM_ACK_PULSES){ // Ack received?
        this->ack_received = now;
        this->num_ack_pulses = 0;
      }
    }
    if(DEBUG){
      logger->log(false, "SHORT ");
    }
  }
  if( duration>=LONG_PULSE_DURATION_MIN_MS && duration<=LONG_PULSE_DURATION_MAX_MS){
    if(this->isReceivingMessage()){
      this->message <<= 1;
      this->message |= 1;
      this->num_received_bits++;
    }
    this->num_ack_pulses=0;
    if(DEBUG){
      logger->log(false, "LONG ");
    }
  }
  if(this->num_received_bits == MESSAGE_LEN){ // Message received?
    this->is_message_pending = true;
    this->is_receiving_message = false;
    this->num_received_bits = 0;
    this->message_time = now;
  }
  if(DEBUG){
    LOG(" duration:[%d] last:[%d] now:[%d] message_len:[%d] ack:[%d%c]", 
        (int)duration, (int)this->last_bit_time, (int)now, (int)this->num_received_bits,
        (int)this->num_ack_pulses, this->ack_received?'-':'Y');
  }
  this->last_bit_time = now;
}

SimplebusReceiver::SimplebusReceiver(int pin){
  this->bus_pin = pin;
  this->last_bit_time = -1;
  this->num_received_bits = 0;
  this->message = 0;
  this->is_receiving_message = false;
  this->num_ack_pulses = 0;
  this->ack_received = 0;
  this->is_message_pending = false;
  this->message_time = -1;
  SimplebusReceiver::instance = this;
  if(DEBUG){
    logger->log(true, "Setting receiver pin.");
    logger->flush();
  }
  pinMode(pin, INPUT);
  if(DEBUG){
    LOG("Receiver pin set.");
    LOG("Receiver pin state: %d.", digitalRead(pin));
    logger->flush();
  }
  this->enableReceiver();
}  
SimplebusMessage SimplebusReceiver::getMessage(){
  return this->getMessage(NULL);
}
SimplebusMessage SimplebusReceiver::getMessage(long unsigned* time){
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
unsigned long SimplebusReceiver::getAck(){
  unsigned long result = this->ack_received;
  if(result){
    this->ack_received = 0;
    this->num_ack_pulses = 0;
  }
  return result;
}
void SimplebusReceiver::disableReceiver(){
  while( isReceivingMessage() )
    delay(5);
  detachInterrupt(digitalPinToInterrupt(this->bus_pin));
  if(DEBUG){
    LOG("Receiver disabled.");
    logger->flush();
  }
}
void SimplebusReceiver::enableReceiver(){
  pinMode(this->bus_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(this->bus_pin), SimplebusReceiver::busCallbackAdapter, FALLING);
  if(DEBUG){
    LOG("Receiver enabled.");
    logger->flush();
  }
}
bool SimplebusReceiver::isReceivingMessage(){
    if(millis()-last_bit_time > MAX_MESSAGE_IDLE_MS){
      this->is_receiving_message = false;
    }
    return this->is_receiving_message;
}
SimplebusReceiver* SimplebusReceiver::instance= NULL;
