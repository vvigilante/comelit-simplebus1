#include "SimplebusReceiver.h"
#include "SimplebusTransmitter.h"

#ifndef SIMPLEBUS_H_
#define SIMPLEBUS_H_

class Simplebus : public SimplebusReceiver, SimplebusTransmitter{
    public:
        Simplebus(int receiver_pin, int transmitter_pin):
            SimplebusReceiver(receiver_pin),
            SimplebusTransmitter(transmitter_pin){

        }        
        void putMessage(uint8_t command, uint8_t id){
            this->disableReceiver();
            SimplebusTransmitter::putMessage(command, id);
            this->enableReceiver();
        }
        void putMessage(SimplebusMessage m){
            this->disableReceiver();
            SimplebusTransmitter::putMessage(m);
            this->enableReceiver();
        }
        void putAck(){
            this->disableReceiver();
            SimplebusTransmitter::putAck();
            this->enableReceiver();
        }

};

#endif /* SIMPLEBUS_H_ */