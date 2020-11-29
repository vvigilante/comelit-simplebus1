#include "SimplebusReceiver.h"
#include "SimplebusTransmitter.h"

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
        SimplebusMessage getMessageAndAck(){
            long long msg_time; 
            SimplebusMessage msg = this->getMessage(&msg_time);
            if(!msg.valid)
                return msg;
            else{
                if(msg.isChecksumValid()){
                    while( millis() - msg_time < MIN_TIME_BEFORE_ACK_MS )
                        delay(2);
                    this->putAck();
                    return msg;
                }else{
                    msg.valid = false;
                    return msg;
                }
            }
        }

};