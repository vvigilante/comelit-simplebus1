#ifndef SIMPLEBUS_RELIABLE_TRANSMITTER_H_
#define SIMPLEBUS_RELIABLE_TRANSMITTER_H_

#include "Simplebus.h"

#define MAX_TRIES 5
#define ACK_WAIT_TIME_MS (MIN_TIME_BEFORE_ACK_MS*2)

/**
 * Send and resends a message until an ack is received
 */
class SimplebusReliableTransmitter {
  private:
    Simplebus* sb;
    SimplebusMessage message_to_send;
    unsigned long timestamp_sent;
    int num_tries;
    bool ack_received;
    SemaphoreHandle_t mutex;
    enum e_sending_result{
      RES_UNKNOWN, /*< No message to send or not  */
      RES_OK,      /*< Message sent */
      RES_FAIL,    /*< Message set MAX_TRIES times but no ack received */
      RES_PENDING  /*< Sending / waiting for Ack */
    } result = RES_UNKNOWN;
  public:
    SimplebusReliableTransmitter(Simplebus* sb) {
      this->sb = sb;
      this->message_to_send.valid = false;
      this->mutex = xSemaphoreCreateMutex();
    }
    ~SimplebusReliableTransmitter() {
      vSemaphoreDelete( this->mutex );
    }
    /**
     * Returns the state of the message to be sent
     * (RES_OK = sent and acknowledged)
     */
    enum e_sending_result getResult(SimplebusMessage m) {
      return result;
    }
    /**
     * Aborts sending the current message
     */
    void abort(SimplebusMessage m) {
      xSemaphoreTakeFromISR(this->mutex, NULL);
      this->message_to_send.valid = false;
      xSemaphoreGiveFromISR(this->mutex, NULL);
    }
    /**
     * Sets a new message to be sent.
     * If a message is pending, it gets aborted
     */
    void send(SimplebusMessage m) {
      xSemaphoreTakeFromISR(this->mutex, NULL);
      this->message_to_send = m;
      this->num_tries = 0;
      this->ack_received = false;
      xSemaphoreGiveFromISR(this->mutex, NULL);
    }
    /**
     * Notifies that an ack was received from the bus.
     */
    void ackReceived() {
      xSemaphoreTakeFromISR(this->mutex, NULL);
      this->ack_received = true;
      xSemaphoreGiveFromISR(this->mutex, NULL);
    }
    /**
     * Process the pending operations, if any
     */
    void loop() {
      SimplebusMessage msg_to_send_now;
      msg_to_send_now.valid = false;
      xSemaphoreTakeFromISR(this->mutex, NULL);
      if (this->message_to_send.valid) { // If there is a message to send
          if(ack_received){
              this->result = RES_OK;
              this->message_to_send.valid = false; // We done.
              LOG("Message sent and received.");
          }else{
              bool timeout = millis() > (timestamp_sent+ACK_WAIT_TIME_MS);
              if(0==this->num_tries || timeout){
                  this->num_tries++;
                  if(this->num_tries>MAX_TRIES){ // Give up
                      this->result = RES_FAIL;
                      LOG("Message not received after %d tries.", MAX_TRIES);
                      this->message_to_send.valid = false; // We done.
                  }else{ // Send now
                      msg_to_send_now = this->message_to_send;
                      this->result = RES_PENDING;
                      this->ack_received = false;
                  }
              }
          }
      }else{
        this->result = RES_UNKNOWN;
      }
      xSemaphoreGiveFromISR(this->mutex, NULL);
      if(msg_to_send_now.valid){
        sb->putMessage(msg_to_send_now);
        this->timestamp_sent = millis();
      }
    }
};

#endif /* SIMPLEBUS_RELIABLE_TRANSMITTER_H_ */