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
 *     end -> 28ms
 *     
 * The command is 20 bits long:
 * 6 bit command
 * 8 bit id
 * 4 bit checksum
 */
#define START_PULSE_DURATION_MIN_MS 17
#define START_PULSE_DURATION_MAX_MS 23
#define END_PULSE_DURATION_MIN_MS 24
#define END_PULSE_DURATION_MAX_MS 32
#define SHORT_PULSE_DURATION_MIN_MS 3
#define SHORT_PULSE_DURATION_MAX_MS 7
#define LONG_PULSE_DURATION_MIN_MS 8
#define LONG_PULSE_DURATION_MAX_MS 10

#define MAX_MESSAGE_LEN 20

#define DEBUG 0

class BusReceiver{
  private: 
    int bus_pin;
    bool is_receiving_message;
    char message[MAX_MESSAGE_LEN+1];
    size_t message_len;
    int read_time;
    bool message_pending;

    static BusReceiver* instance = NULL;
    static void busCallbackAdapter(){ // TODO update with non-static std::function 
      instance->busCallback();
    }
    void busCallback(){
      int now = millis();
      int duration = now - read_time;
      if( duration>=START_PULSE_DURATION_MIN_MS && duration<=START_PULSE_DURATION_MAX_MS){
        memset(this->message, 0, MAX_MESSAGE_LEN);
        this->message_len=0;
        this->is_receiving_message=true;
        if(DEBUG) Serial.println("START");
      }
      if( duration>=END_PULSE_DURATION_MIN_MS && duration<=END_PULSE_DURATION_MAX_MS){
        this->is_receiving_message=false;
        this->message_pending=true;
        if(DEBUG) Serial.println("END");
      }
      if(this->message_len < MAX_MESSAGE_LEN && this->is_receiving_message){
          if( duration>=SHORT_PULSE_DURATION_MIN_MS && duration<=SHORT_PULSE_DURATION_MAX_MS){
            this->message[this->message_len]='0';
            this->message_len++;
            if(DEBUG) Serial.println("0");
          }
          if( duration>=LONG_PULSE_DURATION_MIN_MS && duration<=LONG_PULSE_DURATION_MAX_MS){
            this->message[this->message_len]='1';
            this->message_len++;
            if(DEBUG) Serial.println("1");
          }
      }
      this->read_time = now;
      if(DEBUG) Serial.println('.');
    }
  public:
    BusReceiver(int pin){
      this->message_len=0;
      memset(this->message, 0, MAX_MESSAGE_LEN+1);
      //this->cb = cb;
      this->bus_pin = pin;
      this->is_receiving_message = false;
      this-> read_time = -1;
      this->message_pending = false;
      BusReceiver::instance = this;
      pinMode(pin, INPUT);
      attachInterrupt(digitalPinToInterrupt(pin), BusReceiver::busCallbackAdapter, FALLING);
    }  
    const char* getMessage(){
      if(this->message_pending){
        message_pending = false;
        return this->message;
      }
      return NULL;
    }
};
BusReceiver* BusReceiver::instance;


#define BUS_PIN 2

BusReceiver br = BusReceiver(BUS_PIN);

void setup() {
  Serial.begin(9600);
}

void loop() {
  delay(100);
  const char* message = br.getMessage();
  if(message){
    Serial.println(message);
  }
}
