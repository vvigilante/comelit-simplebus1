#include <Arduino.h>
#include "SimplebusMessage.h"

#ifndef SIMPLEBUS_RECEIVER_H_
#define SIMPLEBUS_RECEIVER_H_

class SimplebusReceiver{
  private: 
    int bus_pin;
    long unsigned last_bit_time;
    int num_received_bits;
    uint32_t message;
    bool is_receiving_message;
    int num_ack_pulses;
    bool ack_received;
    bool is_message_pending;
    long unsigned message_time;
    
    static SimplebusReceiver* instance;
    ICACHE_RAM_ATTR static void busCallbackAdapter();
  public:
    void busCallback();
    SimplebusReceiver(int pin);
    SimplebusMessage getMessage();
    SimplebusMessage getMessage(long unsigned* time);
    bool getAck();
    void disableReceiver();
    void enableReceiver();
    bool isReceivingMessage();
};

#endif /* SIMPLEBUS_RECEIVER_H_ */
