/*
 * Falling edge to falling edge time for each type of signal
 * short=0 ->  6ms
 *  long=1 ->  9ms
 *   start -> 20ms
 *     
 * The command is 18 bits long:
 * 6 bit command
 * 8 bit id
 * 4 bit checksum
 * 
 * It appears that messages sent by the apartment unit end in 0,
 * while messages sent by the external unit end in 1.
 * 
 * Example messages:
 * 0x3D86
 * 0x2582
 * 0x2D8A
 * 
 */

#ifndef SIMPLEBUS_MESSAGE_H_
#define SIMPLEBUS_MESSAGE_H_

#define DEBUG true /**< Enable for verbose serial print inside the class */

#define CMD_OPEN     2 // 000010
#define CMD_CALL     3 // 000011
#define CMD_LIGHT   50 // 110010
#define CMD_PICKUP1 34 // 100010
#define CMD_PICKUP2 35 // 100011
#define CMD_HANGUP  18 // 010010
#define CMD_CLEAR   63 // 111111

// Transmission times
#define LOW_TIME_MS         3
#define HIGH_TIME_SHORT_MS  3
#define HIGH_TIME_LONG_MS   6
#define HIGH_TIME_START_MS 17

#define MIN_TIME_BEFORE_ACK_MS 22

// Reception times
#define START_PULSE_DURATION_MIN_MS 14
#define START_PULSE_DURATION_MAX_MS 23
#define SHORT_PULSE_DURATION_MIN_MS  4
#define SHORT_PULSE_DURATION_MAX_MS  7
#define LONG_PULSE_DURATION_MIN_MS   8
#define LONG_PULSE_DURATION_MAX_MS  11

#define MAX_MESSAGE_IDLE_MS 70 /**< Abort receiving message if no signal for this amount of milliseconds */

#define NUM_ACK_PULSES 3 /**< Three short pulses represent an ack */

// Message format

#define CMD_LEN 6
#define ID_LEN 8
#define CHK_LEN 4
#define MESSAGE_LEN (CMD_LEN+ID_LEN+CHK_LEN)


struct SimplebusMessage{
  uint8_t command;
  uint8_t id;
  uint8_t checksum;
  bool valid;
  SimplebusMessage(){
    this->valid = false;
  }
  SimplebusMessage(uint32_t msg){
    this->valid = true;
    this->checksum = msg & 0x0F;
    this->id = (msg>>CHK_LEN) & 0xFF;
    this->command = (msg>>(CHK_LEN+ID_LEN)) & 0xFF;
  }
  SimplebusMessage(uint8_t command, uint8_t id){
    this->valid = true;
    this->command = command;
    this->id = id;
    this->checksum = computeChecksum();
  }
  String toString(){
    if(!this->valid)
      return String("(nothing)");

    char msg_string[CMD_LEN+ID_LEN+CHK_LEN+3];
    char* msg_string_ptr = msg_string;
    for(int i=CMD_LEN-1; i>=0; i--)
      *(msg_string_ptr++) = '0'+((this->command>>i)&1);
    *(msg_string_ptr++)=' ';
    for(int i=ID_LEN-1; i>=0; i--)
      *(msg_string_ptr++) = '0'+((this->id>>i)&1);
    *(msg_string_ptr++)=' ';
    for(int i=CHK_LEN-1; i>=0; i--)
      *(msg_string_ptr++) = '0'+((this->checksum>>i)&1);
    *msg_string_ptr='\0';

    String s = String(msg_string) + " - " + cmdToString(this->command) + " " + idFromInt(this->id);
    if(this->isChecksumValid()){
      s += " chkOK";
    }else{
      s += " chkERR";
    }
    return s;
  }
  uint8_t computeChecksum(){
    uint8_t n = 0;
    for(int i=0; i<CMD_LEN; i++)
      n += (this->command>>i)&1;
    for(int i=0; i<ID_LEN; i++)
      n += (this->id>>i)&1;
    // Reverse order
    uint8_t nrev = ((n&1)<<3) | ((n&2)<<1) | ((n&4)>>1) | ((n&8)>>3);
    return nrev;
  }
  bool isChecksumValid(){
      return this->computeChecksum() == this->checksum;
  }
  static uint8_t idFromInt(uint8_t n ){
    uint8_t id = ((n&1)<<7) | ((n&2)<<5) | ((n&4)<<3) | ((n&8)<<1) | ((n&16)>>1) | ((n&32)>>3) | ((n&64)>>5) | ((n&128)>>7);
    return id;
  }
  static String cmdToString(uint8_t cmd){
    switch(cmd){
      case CMD_OPEN: return "OPEN";
      case CMD_CALL: return "CALL";
      case CMD_LIGHT: return "LIGHT";
      case CMD_PICKUP1: return "PUP1";
      case CMD_PICKUP2: return "PUP2";
      case CMD_HANGUP: return "HUP";
      case CMD_CLEAR: return "CLEAR";
      default: return "????";
    }
  }
};


#endif /* SIMPLEBUS_MESSAGE_H_ */
