#include "Simplebus.h"
#include "Logger.h"
#include <stdint.h>
#define MIN_TIME_BEFORE_ACK_MS  27  // ack must be sent this amount of ms after the end of the message
#define ACK_TIMEOUT_MS  (7*MIN_TIME_BEFORE_ACK_MS) // Message is considered lost if ack is not received within this time
#define PUP2_TIMEOUT_MS        250 // The external unit is expected to pick up within this time
#define POLLING_TIME_MS          1 // Time to delay when no message is received before checking again
#define MAX_TRIES                3 // Try 3 times to send a message if no ack is received

#define MY_ID UINT8_MAX


typedef enum{
    SB_STATE_HUP = 0,
    SB_STATE_CALL = 1
} SimplebusIntercomState;

class SimplebusIntercom{
    private:
        uint8_t my_id;
        Simplebus* sb;
        bool put_message_and_wait_ack(uint8_t cmd, uint8_t id=MY_ID){
            if (MY_ID == id) id = this->my_id;
            this->sb->getAck();
            this->sb->putMessage(cmd, id);
            unsigned long time_start = millis();
            bool ack = false;
            while(millis()-time_start<ACK_TIMEOUT_MS && !ack ){
                ack = this->sb->getAck();
                if(!ack)
                    delay(POLLING_TIME_MS);
            }
            return ack;
        }
        
        bool put_message_and_wait_ack_with_retransmission(uint8_t cmd, uint8_t id=MY_ID){
            int num_tries = 0;
            bool ack_received = false;
            while (num_tries<MAX_TRIES && !ack_received){
                ack_received = put_message_and_wait_ack(cmd, id);
                if(!ack_received){
                    LOG("retry");
                    num_tries++;
                }
            }
            return ack_received;
        }

        bool wait_for_message_and_put_ack(uint8_t cmd, unsigned long timeout_ms, uint8_t id=MY_ID){
            if (MY_ID == id) id = this->my_id;
            unsigned long time_start = millis();
            bool received = false;
            unsigned long message_time = -1;
            while(millis()-time_start<timeout_ms){
                SimplebusMessage message = sb->getMessage(&message_time);
                if(message.valid && message.isChecksumValid() && id==message.id && message.command == cmd){
                    while(millis()-message_time >= MIN_TIME_BEFORE_ACK_MS){
                        delay(POLLING_TIME_MS);
                    }
                    sb->putAck();
                    return true;
                }
                delay(POLLING_TIME_MS);
            }
            return false;
        }
    public:
        SimplebusIntercom(Simplebus* sb, int my_id){
            this->my_id = SimplebusMessage::idFromInt(my_id);
            this->sb = sb;
        }
        bool lockOpen(){
            return put_message_and_wait_ack_with_retransmission(CMD_OPEN);
        }
        bool lightButton(){
            return put_message_and_wait_ack_with_retransmission(CMD_LIGHT);
        }
        bool waitForCall(unsigned long timeout_ms){
            return wait_for_message_and_put_ack(CMD_CALL, timeout_ms);
        }
        bool answerCall(){
            bool pup1_ack_received = put_message_and_wait_ack_with_retransmission(CMD_PICKUP1);
            if(!pup1_ack_received)
                return false;
            // Wait for external unit to pickup (should be immediate)
            return wait_for_message_and_put_ack(CMD_PICKUP2, PUP2_TIMEOUT_MS);
        }
        bool hangupCall(){
            return put_message_and_wait_ack_with_retransmission(CMD_HANGUP);
        }

        
        /** 
         * This function does not belong here, but it is useful for tests, i.e. to 
         * make a call from one intercom to another without involving the external unit
         * **/
        bool makeCall(uint8_t id){
            uint8_t dest_id = SimplebusMessage::idFromInt(id);
            bool call_success = put_message_and_wait_ack_with_retransmission(CMD_CALL, dest_id);
            if(!call_success){
                LOG("Failed to place call.");
                return false;
            }
            LOG("Call ok, waiting for the receiver to pick up.");
            const unsigned long call_timeout_ms = 10000;
            bool pup1 = wait_for_message_and_put_ack(CMD_PICKUP1, call_timeout_ms, dest_id);
            if(!pup1){
                LOG("Recipient did not pick up.");
                return false;
            }
            LOG("Receiver pick up ok, sending pick up response.");
            bool pup2 = put_message_and_wait_ack_with_retransmission(CMD_PICKUP2, dest_id);
            if(!pup2){
                LOG("Failed to pick up (external side).");
                return false;
            }
            return true;
        }
};