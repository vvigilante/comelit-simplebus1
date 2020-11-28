/*
 * Connect the bus to the arduino through a voltage divider (1000 KOhm - 180 KOhm to gnd)
 * 
 *                 GND ------- BUS-
 *                  |
 *               180KOhm
 *                  |
 * D2 ----1MOhm-----|
 *                  |
 *                 BUS+
 * 
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
 * Example messages:
 * 0x3D86
 * 0x2582
 * 0x2D8A
 * 
 */

#define START_PULSE_DURATION_MIN_MS 17
#define START_PULSE_DURATION_MAX_MS 23
#define SHORT_PULSE_DURATION_MIN_MS  3
#define SHORT_PULSE_DURATION_MAX_MS  7
#define LONG_PULSE_DURATION_MIN_MS   8
#define LONG_PULSE_DURATION_MAX_MS  10

#define NUM_ACK_PULSES 3 /**< Three short pulses represent an ack */

#define MAX_MESSAGE_IDLE_MS 70 /**< Abort receiving message if no signal for this amount of milliseconds */

#define CMD_LEN 6
#define ID_LEN 8
#define CHK_LEN 4
#define MESSAGE_LEN (CMD_LEN+ID_LEN+CHK_LEN)

#define CMD_OPEN 2 // 000010
#define CMD_CALL 3 // 000011


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
    return String(msg_string);
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
};

class SimplebusReceiver{
  private: 
    int bus_pin;
    long long last_bit_time;
    int num_received_bits;
    uint32_t message;
    bool is_receiving_message;
    int num_ack_pulses;
    bool ack_received;
    bool is_message_pending;
    long long message_time;

    static SimplebusReceiver* instance;
    static void busCallbackAdapter(){ // TODO update with non-static std::function 
      instance->busCallback();
    }

    void busCallback(){
      long long now = millis();
      int duration = now - last_bit_time;
      if( duration>=START_PULSE_DURATION_MIN_MS && duration<=START_PULSE_DURATION_MAX_MS){
        this->message=0;
        this->num_received_bits=0;
        this->is_receiving_message=true;
      }
      if( duration>=SHORT_PULSE_DURATION_MIN_MS && duration<=SHORT_PULSE_DURATION_MAX_MS){
        if(this->isReceivingMessage()){
          this->message <<= 1;
          this->num_received_bits++;
        }else{
          this->num_ack_pulses++;
          if(this->num_ack_pulses==NUM_ACK_PULSES){ // Ack received?
            this->ack_received = true;
          }
        }
      }
      if( duration>=LONG_PULSE_DURATION_MIN_MS && duration<=LONG_PULSE_DURATION_MAX_MS){
        if(this->isReceivingMessage()){
          this->message <<= 1;
          this->message &= 1;
          this->num_received_bits++;
        }
      }
      if(this->num_received_bits == MESSAGE_LEN){ // Message received?
        this->is_message_pending = true;
        this->is_receiving_message = false;
        this->message_time = now;
      }
      this->last_bit_time = now;
    }
  public:
    SimplebusReceiver(int pin){
      this->bus_pin = pin;
      this->last_bit_time = -1;
      this->num_received_bits = 0;
      this->message = 0;
      this->is_receiving_message = false;
      this->num_ack_pulses = 0;
      this->ack_received = false;
      this->is_message_pending = false;
      this->message_time = -1;
      SimplebusReceiver::instance = this;
      pinMode(pin, INPUT);
      this->enableReceiver();
    }  
    SimplebusMessage getMessage(){
      return this->getMessage(NULL);
    }
    SimplebusMessage getMessage(long long* time){
      if(this->is_message_pending){
        if(time)
          *time = this->message_time;
        return SimplebusMessage(this->message);
      }
      if(time)
        *time = -1;
      return SimplebusMessage();
    }
    bool getAck(){
      bool result = this->ack_received;
      if(result){
        this->ack_received = false;
        this->num_ack_pulses = 0;
      }
      return result;
    }
    void disableReceiver(){
      while( isReceivingMessage() )
        delay(5);
      detachInterrupt(digitalPinToInterrupt(this->bus_pin));
    }
    void enableReceiver(){
      attachInterrupt(digitalPinToInterrupt(this->bus_pin), SimplebusReceiver::busCallbackAdapter, FALLING);
    }
    bool isReceivingMessage(){
        if(millis()-last_bit_time > MAX_MESSAGE_IDLE_MS){
          this->is_receiving_message = false;
        }
        return this->is_receiving_message;
    }
};
SimplebusReceiver* SimplebusReceiver::instance= NULL;

#define LOW_TIME_MS         3
#define HIGH_TIME_SHORT_MS  3
#define HIGH_TIME_LONG_MS   6
#define HIGH_TIME_START_MS 17

class SimplebusTransmitter{
  private:
    int bus_pin;
    void transmitValue(bool value){
      digitalWrite(this->bus_pin, LOW);
      delay(LOW_TIME_MS);
      digitalWrite(this->bus_pin, HIGH);
      delay(value?HIGH_TIME_LONG_MS:HIGH_TIME_SHORT_MS);
    }
    void transmitStart(){
      digitalWrite(this->bus_pin, LOW);
      delay(LOW_TIME_MS);
      digitalWrite(this->bus_pin, HIGH);
      delay(HIGH_TIME_START_MS);
    }
    void transmitEnd(){
      digitalWrite(this->bus_pin, LOW);
      delay(LOW_TIME_MS);
      digitalWrite(this->bus_pin, HIGH);
    }
  public:
    SimplebusTransmitter(int pin){
      this->bus_pin = pin;
      pinMode(this->bus_pin, OUTPUT);
    }
    void putMessage(SimplebusMessage message){
      this->transmitStart();
      for(int i=CMD_LEN-1; i>=0; i--)
        this->transmitValue((message.command>>i)&1);
      for(int i=ID_LEN-1; i>=0; i--)
        this->transmitValue((message.id>>i)&1);
      for(int i=CHK_LEN-1; i>=0; i--)
        this->transmitValue((message.checksum>>i)&1);
      this->transmitEnd();
    }
    void putMessage(uint8_t command, uint8_t id){
      SimplebusMessage message(command, id);
      this->putMessage(message);
    }
    void putAck(){
      transmitValue(0);
      transmitValue(0);
      transmitValue(0);
      transmitValue(0);
    }
};



#define SLEEP_TIME_MS 2000
#define BUS_RECEIVE_PIN 2

SimplebusReceiver br = SimplebusReceiver(BUS_RECEIVE_PIN);

//#include <ArduinoLowPower.h>
void deepSleep(){
  //LowPower.deepSleep(); TODO
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  long long time_last_event = 0;
  delay(5);
  long long message_time = -1;
  SimplebusMessage message = br.getMessage(&message_time); // Non blocking
  if(message.valid){
    time_last_event = millis();
    Serial.print((int)message_time);
    Serial.print(" ");
    Serial.println(message.toString());
  }
  if(millis()-time_last_event > SLEEP_TIME_MS){
    Serial.print("Going to sleep.");
    time_last_event = millis();
    deepSleep();
  }
}
