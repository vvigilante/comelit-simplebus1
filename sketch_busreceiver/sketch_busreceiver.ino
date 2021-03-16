/*
 * 
 * To receive: connect the bus to the IO5 pin through a 
 * voltage divider (1000 KOhm to bus - 160 KOhm to gnd).
 * 
 * To trasmit: IO18 controls the bus though an NPN transistor
 * 
 * GND -----.-------------  BUS-  ---------.-------.
 *          |                              |       |
 *      (160KOhm)                          |       |
 *          |                              |       |
 * IO5 -----'----(1MOhm)--  BUS+  ---.     |       |
 *                                   |     |       |
 *                                    \   /^    (5.6KOhm)
 *                                NPN -----        |
 *                    10uF              |          |
 * IO18 ---------------||---------------'----------'
 *                     +
 * 
 * Flash configuration: Wemos D1 MINI ESP32
 * 
 */
#include <WiFi.h>
#include "SimpleBus.h"
#include "Seriallogger.h"
#include "HTTPLogger.h"
#include "SimplebusIntercom.h"


/* Configuration */
#define ENABLE_SLEEP true
#define SLEEP_TIME_MS 16000
/* Wemos pins */
#define BUS_RECEIVE_PIN GPIO_NUM_5
#define BUS_TRANSMIT_PIN GPIO_NUM_18
#define LED_BUILTIN GPIO_NUM_2
/* Wifi settings */
#define WIFI_SSID "ssid"
#define WIFI_PASS "pass"

Simplebus* sb = NULL;
long unsigned time_last_event = 0;



#if ENABLE_SLEEP
    #define FPM_SLEEP_MAX_TIME  0xFFFFFFF
    unsigned long wakeup_time = 0;
    bool wakeup_now = false;
    ICACHE_RAM_ATTR void wakeupCallback(){
      digitalWrite(LED_BUILTIN, LOW);
      wakeup_now = true;
    }
    inline void goToSleep(){
      LOG("Going to sleep.");
      logger->flush(true); // todo remove for speed
      digitalWrite(LED_BUILTIN, HIGH);
      gpio_wakeup_enable(BUS_RECEIVE_PIN, GPIO_INTR_LOW_LEVEL);
      esp_sleep_enable_gpio_wakeup();
      esp_light_sleep_start();
      wakeupCallback();
      //delay(1);
    }
#else /* ENABLE_SLEEP */
    void goToSleep(){
    }
#endif /* ENABLE_SLEEP */

void setup() {
  logger = new SerialLogger(115200);
  //logger = new HTTPLogger();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  sb = new Simplebus(BUS_RECEIVE_PIN, BUS_TRANSMIT_PIN);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  LOG("Init ok.");
  //sb->putMessage(CMD_CALL, SimplebusMessage::idFromInt(27));
  //Serial.println(millis());
}

bool connected = false;

#define WIFI_ENABLE_AFTER_WAKEUP_TIME_MS 2500
void loop() {
  if(wakeup_now){
    wakeup_now = false;
    sb->busCallback();
    wakeup_time=millis();
    time_last_event = millis();
    digitalWrite(LED_BUILTIN, LOW);
    LOG("Wake up.");
    sb->enableReceiver();
  }
  delay(2);
  long unsigned message_time = -1;
  
  if( wakeup_time && millis()-wakeup_time > WIFI_ENABLE_AFTER_WAKEUP_TIME_MS ){
    LOG("Enabling WiFi...");
    WiFi.begin();
    wakeup_time = 0;
  }
  if(!connected && (WiFi.status() == WL_CONNECTED) ){
    LOG("Wifi connected, ip: [%s].", WiFi.localIP().toString().c_str());
    connected = true;
  }
  if(connected && (WiFi.status() != WL_CONNECTED) ){
    LOG("Wifi disconnected.");
    connected = false;
  }
  
  SimplebusMessage message = sb->getMessage(&message_time); // Non blocking
  if(message.valid){
    time_last_event = millis();
    LOG("time: [%d.%03ds] message: [%s].",message_time/1000, message_time%1000, message.toString().c_str());
  }
  bool ack = sb->getAck();
  if(ack){
    LOG("ack.");
  }
  if(millis()-time_last_event > SLEEP_TIME_MS && !sb->isReceivingMessage()){
    time_last_event = millis();
    logger->flush(true);
    if(!sb->isReceivingMessage()){
      sb->disableReceiver();
      goToSleep();
    }
  }
  //if( (millis()/2) % 1000 == 0){    Serial.println("alive");  }
}
