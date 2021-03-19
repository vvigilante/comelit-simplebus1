#include "SimplebusMessage.h"
#include <stdint.h>

class SimplebusState{
    public:    
        enum bus_state_t{
            CLEAR     = 1, // Init state, return here whenever external sends CMD_CLEAR
            CALL_REQ  = 2, // The external unit sent CMD_CALL
            RINGING   = 3, // The internal unit acked the call 
            PICKUP2   = 6, // The external unit sent CMD_PICKUP2
            CONNECTED = 7, // The external unit sent CMD_PICKUP2, internal acked
            HANGED    = 8, // The internal unit sent CMD_HANGUP
        };
    private:
        bus_state_t state = CLEAR;
        uint16_t userid = 0;
        unsigned long message_time;
    public:
        void feed_message(SimplebusMessage m){
            if(CMD_CALL==m.command){
                state = CALL_REQ;
                userid = m.id;
            }else if(CMD_PICKUP2==m.command){
                state = PICKUP2;
                userid = m.id;
            }else if(CMD_HANGUP==m.command){
                state = HANGED;
                userid = m.id;
            }else if(CMD_CLEAR==m.command){
                state = CLEAR;
                userid = 0;
            }
        }
        void feed_ack(){
            if(CALL_REQ==state){
                state = RINGING;
            }else if(PICKUP2==state){
                state = CONNECTED;
            }

        }
        uint16_t get_userid() const{
            return userid;
        } 
        bus_state_t get_state() const{
            return state;
        } 
};