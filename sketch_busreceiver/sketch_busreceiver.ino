/*
 * 
 * To receive: connect the bus to the arduino D2 through a 
 * voltage divider (1000 KOhm - 180 KOhm to gnd).
 * 
 * To trasmit: arduino D3 controls the bus though an NPN transistor
 * 
 * GND -------------.------  BUS-  --------.-------.
 *                  |                      |       |
 *              (180KOhm)                  |       |
 *                  |                      |       |
 * D2 ---(1MOhm)----'------  BUS+  --.     |       |
 *                                   |     |       |
 *                                    \   /^    (5.6KOhm)
 *                                NPN -----        |
 *                    10uF              |          |
 * A0 -----------------||---------------'----------'
 *                     +
 */

#include "SimpleBus.h"


#define SLEEP_TIME_MS 2000
#define BUS_RECEIVE_PIN 2   //D2
#define BUS_TRANSMIT_PIN 14 //A0

Simplebus sb = Simplebus(BUS_RECEIVE_PIN, BUS_TRANSMIT_PIN);
long long time_last_event = 0;

//#include <ArduinoLowPower.h>
void deepSleep(){
  //Serial.print(millis());
  //Serial.println(" - Going to sleep.");
  //LowPower.deepSleep(); TODO
}

void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  //Serial.println(millis());
  //sb.putMessage(CMD_CALL, SimplebusMessage::idFromInt(27));
  //Serial.println(millis());
}

void loop() {
  delay(2);
  long long message_time = -1;
  
  SimplebusMessage message = sb.getMessage(&message_time); // Non blocking
  if(message.valid){
    time_last_event = millis();
    Serial.print("time: [");
    Serial.print((int)(message_time/1000));
    Serial.print(".");
    Serial.print((int)(message_time%1000));
    Serial.print("s] message: [");
    Serial.print(message.toString());
    Serial.println("]");
  }
  bool ack = sb.getAck();
  if(ack){
    Serial.print("time: [");
    Serial.print(millis()/1000);
    Serial.print(".");
    Serial.print(millis()%1000);
    Serial.println("s] ack");
  }
  if(millis()-time_last_event > SLEEP_TIME_MS){
    time_last_event = millis();
    deepSleep();
  }
}
