#ifndef SIMPLEBUS_INTERCOM_FSM_H_
#define SIMPLEBUS_INTERCOM_FSM_H_

#define STATE_RINGING_TIMEOUT_MS   15000
#define STATE_WAITING_TIMEOUT_MS    2000
#define STATE_CONNECTED_TIMEOUT_MS 15000

/**
 * Implements the state logic of an internal intercom unit in abstract terms.
 * @detail This will be a Mealy machine, because if the external unit unexpectededly
 * repeats a command (e.g. because the ack is lost) we want to acknowledge again.
 */
class SimplebusIntercomFSM {
  
  public:
    typedef enum {
      EVT_NO_EVENT,
      EVT_CALL_INCOMING,  /**< A call is incoming */
      EVT_USER_PICKUP,    /**< The user wishes to answer the call */
      EVT_EXTERNAL_PICKUP,/**< The external unit confirms pickup (PUP2) */
      EVT_USER_HANGUP,    /**< The user wishes to terminate the call */
      EVT_CLEAR           /**< The external units wishes to clear the bus (CMD_CLEAR)*/
    } fsm_event_t;
    typedef enum {
      OUT_NO_ACTION,
      OUT_ACK_AND_RING,/**< Acknowledge CALL, start ringing */
      OUT_PICKUP,      /**< Accept call (send PUP1 message), stop ringing */
      OUT_SILENCE_RING,/**< Stop ringing */
      OUT_CONNECT,     /**< Acknowledge PUP2, enable analog communication */
      OUT_DISCONNECT   /**< Disable analog communication, send CMD_HANGUP */
    } fsm_output_t;

  private:
    typedef fsm_output_t (SimplebusIntercomFSM::*state_fcn_t)(fsm_event_t) ;
    state_fcn_t state;
    unsigned long state_change_timestamp = 0;

    void set_state(state_fcn_t new_state) {
      if (this->state != new_state){
          this->state_change_timestamp = millis();
          this->state = new_state;
          if(&SimplebusIntercomFSM::state_clear == this->state){
              LOG("State: clear.");
          }else if(&SimplebusIntercomFSM::state_ringing == this->state){
              LOG("State: ringing.");
          }else if(&SimplebusIntercomFSM::state_waiting == this->state){
              LOG("State: waiting.");
          }else if(&SimplebusIntercomFSM::state_connected == this->state){
              LOG("State: connected.");
          }else{
              LOG("State: UNKNOWN!!!");
          }
      }
    }

    fsm_output_t state_clear(fsm_event_t evt) {
      switch (evt) {
        case EVT_CALL_INCOMING:
          this->set_state( &SimplebusIntercomFSM::state_ringing );
          return OUT_ACK_AND_RING;
        case EVT_USER_PICKUP:
          return OUT_NO_ACTION;
        case EVT_EXTERNAL_PICKUP:
          return OUT_NO_ACTION;
        case EVT_USER_HANGUP:
          return OUT_NO_ACTION;
        case EVT_CLEAR:
          this->set_state( &SimplebusIntercomFSM::state_clear );
          return OUT_NO_ACTION;
        case EVT_NO_EVENT:
          return OUT_NO_ACTION;
      }
    }
    fsm_output_t state_ringing(fsm_event_t evt) {
      switch (evt) {
        case EVT_CALL_INCOMING:
          return OUT_ACK_AND_RING; // Acknowledge againg
        case EVT_USER_PICKUP:
          this->set_state( &SimplebusIntercomFSM::state_waiting );
          return OUT_PICKUP;
        case EVT_EXTERNAL_PICKUP:
          return OUT_NO_ACTION;
        case EVT_USER_HANGUP:
          return OUT_SILENCE_RING;
        case EVT_CLEAR:
          this->set_state( &SimplebusIntercomFSM::state_clear );
          return OUT_SILENCE_RING;
        case EVT_NO_EVENT:
          bool timeout = millis() > state_change_timestamp + STATE_RINGING_TIMEOUT_MS;
          if (timeout) {
            this->set_state( &SimplebusIntercomFSM::state_clear );
            return OUT_SILENCE_RING;
          }
          return OUT_NO_ACTION;
      }
    }
    fsm_output_t state_waiting(fsm_event_t evt) {
      switch (evt) {
        case EVT_CALL_INCOMING:
          this->set_state( &SimplebusIntercomFSM::state_ringing );
          return OUT_ACK_AND_RING;
        case EVT_USER_PICKUP:
          return OUT_NO_ACTION;
        case EVT_EXTERNAL_PICKUP:
          this->set_state( &SimplebusIntercomFSM::state_connected );
          return OUT_CONNECT;
        case EVT_USER_HANGUP:
          return OUT_NO_ACTION;
        case EVT_CLEAR:
          this->set_state( &SimplebusIntercomFSM::state_clear );
          return OUT_NO_ACTION;
        case EVT_NO_EVENT:
          bool timeout = millis() > state_change_timestamp + STATE_WAITING_TIMEOUT_MS;
          if (timeout) {
            this->set_state( &SimplebusIntercomFSM::state_clear );
            return OUT_NO_ACTION;
          }
          return OUT_NO_ACTION;
      }
    }
    fsm_output_t state_connected(fsm_event_t evt) {
      switch (evt) {
        case EVT_CALL_INCOMING:
          return OUT_NO_ACTION;
        case EVT_USER_PICKUP:
          return OUT_NO_ACTION;
        case EVT_EXTERNAL_PICKUP: // If we get this again, let's acknowledge again!
          this->set_state( &SimplebusIntercomFSM::state_connected );
          return OUT_CONNECT;
        case EVT_USER_HANGUP:
          this->set_state( &SimplebusIntercomFSM::state_clear );
          return OUT_DISCONNECT;
        case EVT_CLEAR:
          this->set_state( &SimplebusIntercomFSM::state_clear );
          return OUT_DISCONNECT;
        case EVT_NO_EVENT:
          bool timeout = millis() > state_change_timestamp + STATE_CONNECTED_TIMEOUT_MS;
          if (timeout) {
            this->set_state( &SimplebusIntercomFSM::state_clear );
            return OUT_DISCONNECT;
          }
          return OUT_NO_ACTION;
      }
    }

  public:
    SimplebusIntercomFSM(){
      this->state = &SimplebusIntercomFSM::state_clear;
    }
    fsm_output_t update(fsm_event_t evt) {
      return (this->*state)(evt);
    }

};

#endif /* SIMPLEBUS_INTERCOM_FSM_H_ */