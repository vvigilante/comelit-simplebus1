#ifndef SIMPLEBUS_INTERCOM_H_
#define SIMPLEBUS_INTERCOM_H_

#include "Logger.h"
#include "Simplebus.h"
#include "SimplebusReliableTransmitter.h"
#include "SimplebusIntercomFSM.h"

typedef void(*audio_cb_t)(bool);
typedef void(*ringer_cb_t)(bool);

/**
 * Manages the bus, reading messages and responding appropriately
 */
class SimplebusIntercom {
  private:
    Simplebus* sb;
    SimplebusReliableTransmitter* sbt;
    SimplebusIntercomFSM fsm;
    int id;
    bool user_hangup = false;
    bool user_pickup = false;
    audio_cb_t audioCb;
    ringer_cb_t ringerCb;
  public:
    SimplebusIntercom(Simplebus* sb, int my_id, audio_cb_t audioCb, ringer_cb_t ringerCb) {
        this->sb = sb;
        this->id = SimplebusMessage::idFromInt(my_id);
        this->sbt = new SimplebusReliableTransmitter(sb);
        this->audioCb = audioCb;
        this->ringerCb = ringerCb;
    }
    /**
     * Call when the user wants to hangup
     */
    void setUserHangup(){
        this->user_hangup = true;
    }
    /**
     * Call when the user wants to pickup
     */
    void setUserPickup(){
        this->user_pickup = true;
    }
    /**
     * Call when the user wants to open the lock
     */
    void sendOpen(){
        this->sbt->send( SimplebusMessage(CMD_OPEN, this->id) );
    }
    void loop() {
        // Query the bus
        long unsigned messsage_time;
        SimplebusMessage message = this->sb->getMessage(&messsage_time);
        long unsigned ack = this->sb->getAck();
        // Update the state
        SimplebusIntercomFSM::fsm_event_t evt = SimplebusIntercomFSM::EVT_NO_EVENT;
        if(message.valid){
            LOG("Message: %s", message.toString().c_str());
            if(message.command==CMD_CLEAR){
              evt = SimplebusIntercomFSM::EVT_CLEAR;
            }else if(message.isChecksumValid() && ( message.id == this->id)){
                switch(message.command){
                  case CMD_CALL:
                    evt = SimplebusIntercomFSM::EVT_CALL_INCOMING;
                    break;
                  case CMD_PICKUP2:
                    evt = SimplebusIntercomFSM::EVT_EXTERNAL_PICKUP;
                    break;
                  default:
                    LOG("Discarded.");
                    break;
                }
            }else{
                LOG("Discarded (invalid or not addressed to me).");
            }
        }else{
            if(this->user_pickup){
                evt = SimplebusIntercomFSM::EVT_USER_PICKUP;
                this->user_pickup = false;
            }else if(this->user_hangup){
                evt = SimplebusIntercomFSM::EVT_USER_HANGUP;
                this->user_hangup = false;
            }
        }
        SimplebusIntercomFSM::fsm_output_t action = fsm.update(evt);
        switch(action){
            case SimplebusIntercomFSM::OUT_NO_ACTION:
            break;
            case SimplebusIntercomFSM::OUT_ACK_AND_RING:
              delay( messsage_time + MIN_TIME_BEFORE_ACK_MS - millis() );
              this->sb->putAck();
              this->ringerCb(true);
              this->user_pickup = false;
            break;
            case SimplebusIntercomFSM::OUT_PICKUP:
              this->sbt->send( SimplebusMessage(CMD_PICKUP1, this->id) );
              this->ringerCb(false);
            break;
            case SimplebusIntercomFSM::OUT_SILENCE_RING:
              this->ringerCb(false);
            break;
            case SimplebusIntercomFSM::OUT_CONNECT:
              delay( messsage_time + MIN_TIME_BEFORE_ACK_MS - millis() );
              this->sb->putAck();
              this->audioCb(true);
              this->user_hangup = false;
            break;
            case SimplebusIntercomFSM::OUT_DISCONNECT:
              this->audioCb(false);
              this->sbt->send( SimplebusMessage(CMD_HANGUP, this->id) );
            break;
        }
        // Let the reliable transmitter do its thing
        if( ack ){
            this->sbt->ackReceived();
        }
        this->sbt->loop();
    }
};

#endif /* SIMPLEBUS_INTERCOM_H_ */