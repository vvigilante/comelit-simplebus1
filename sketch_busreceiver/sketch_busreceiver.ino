/*
 * 
 * To receive: connect the bus to the arduino D2 through a 
 * voltage divider (1000 KOhm to bus - 160 KOhm to gnd).
 * 
 * To trasmit: D1 controls the bus though an NPN transistor
 * 
 * GND -----.-------------  BUS-  ---------.-------.
 *          |                              |       |
 *      (160KOhm)                          |       |
 *          |                              |       |
 * D5 ------'----(1MOhm)--  BUS+  ---.     |       |
 *                                   |     |       |
 *                                    \   /^    (5.6KOhm)
 *                                NPN -----        |
 *                    10uF              |          |
 * D1 -----------------||---------------'----------'
 *                     +
 */
#include <ESP8266WiFi.h>
#include "SimpleBus.h"

#define ENABLE_SLEEP true
#define SLEEP_TIME_MS 20000
/* Wemos D1 mini pins */
#define BUS_RECEIVE_PIN 14 //D5
#define BUS_TRANSMIT_PIN 5 //D1

#define WIFI_SSID "your_ssid"
#define WIFI_PASS "your_password"

Simplebus* sb = NULL;
long long time_last_event = 0;
#if ENABLE_SLEEP

#define FPM_SLEEP_MAX_TIME  0xFFFFFFF
bool wakeup;
ICACHE_RAM_ATTR void wakeupCallback(){
  sb->busCallback();
  wifi_fpm_close();
  wakeup=true;
  time_last_event = millis();
  digitalWrite(LED_BUILTIN, LOW);
  Serial.print((unsigned)time_last_event);
  Serial.println(" - Wake up.");
  Serial.flush();
  sb->enableReceiver();
}
inline void goToSleep(){
  Serial.print(millis());
  Serial.println(" - Going to sleep.");
  Serial.flush();
  digitalWrite(LED_BUILTIN, HIGH);
  wifi_set_opmode(NULL_MODE);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  wakeup=false;
  wifi_fpm_open();
  gpio_pin_wakeup_enable(digitalPinToInterrupt(BUS_RECEIVE_PIN), GPIO_PIN_INTR_LOLEVEL);
  wifi_fpm_set_wakeup_cb(wakeupCallback);
  wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);
}
#else /* ENABLE_SLEEP */
void goToSleep(){
}
#endif /* ENABLE_SLEEP */

void setup() {
  unsigned long us_start = micros();
  ESP.wdtDisable();
  *((volatile uint32_t*) 0x60000900) &= ~(1);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  unsigned long us_bus = micros();
  sb = new Simplebus(BUS_RECEIVE_PIN, BUS_TRANSMIT_PIN);
  unsigned long us_conn = micros();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long us_ready = micros();
  Serial.println("Init ok ");
  Serial.flush();
  //sb->putMessage(CMD_CALL, SimplebusMessage::idFromInt(27));
  //Serial.println(millis());
}

bool connected = false;


void loop() {
  delay(2);
  long long message_time = -1;

  if(!connected && (WiFi.status() == WL_CONNECTED) ){
    Serial.print(millis());
    Serial.print(" - Wifi connected, ip: [");
    Serial.print(WiFi.localIP());
    Serial.println("].");
    Serial.flush();
    connected = true;
  }
  if(connected && (WiFi.status() != WL_CONNECTED) ){
    Serial.print(millis());
    Serial.println(" - Wifi disconnected.");
    Serial.flush();
    connected = false;
  }
  
  SimplebusMessage message = sb->getMessage(&message_time); // Non blocking
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
  bool ack = sb->getAck();
  if(ack){
    Serial.print("time: [");
    Serial.print(millis()/1000);
    Serial.print(".");
    Serial.print(millis()%1000);
    Serial.println("s] ack");
  }
  if(millis()-time_last_event > SLEEP_TIME_MS && !sb->isReceivingMessage()){
    time_last_event = millis();
    goToSleep();
  }
  //if( (millis()/2) % 1000 == 0){    Serial.println("alive");  }
}
