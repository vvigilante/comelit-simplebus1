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
 * 
 * Flash configuration: Wemos D1 R2 & mini, IwIP v2, NoAssert-NDEBUG
 * 
 */
#include <ESP8266WiFi.h>
#include "SimpleBus.h"
#include "HTTPLogger.h"
#include "SimplebusIntercom.h"

HTTPLogger logger;

#define ENABLE_SLEEP true
#define SLEEP_TIME_MS 10000
/* Wemos D1 mini pins */
#define BUS_RECEIVE_PIN 14 //D5
#define BUS_TRANSMIT_PIN 5 //D1

#define WIFI_SSID "your_ssid"
#define WIFI_PASS "your_pass"

Simplebus* sb = NULL;
long unsigned time_last_event = 0;
#if ENABLE_SLEEP

#define FPM_SLEEP_MAX_TIME  0xFFFFFFF
unsigned long wakeup_time = 0;
bool wakeup_now = false;
ICACHE_RAM_ATTR void wakeupCallback(){
  wakeup_now = true;
}
inline void goToSleep(){
  logger.log("Going to sleep.");
  digitalWrite(LED_BUILTIN, HIGH);
  wifi_set_opmode(NULL_MODE);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  wifi_fpm_open();
  gpio_pin_wakeup_enable(digitalPinToInterrupt(BUS_RECEIVE_PIN), GPIO_PIN_INTR_LOLEVEL);
  wifi_fpm_set_wakeup_cb(wakeupCallback);
  wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);
  delay(1);
}
#else /* ENABLE_SLEEP */
void goToSleep(){
}
#endif /* ENABLE_SLEEP */

ADC_MODE(ADC_VCC); 

void setup() {
  ESP.wdtDisable();
  *((volatile uint32_t*) 0x60000900) &= ~(1);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  sb = new Simplebus(BUS_RECEIVE_PIN, BUS_TRANSMIT_PIN);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  logger.log("Init ok.");
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
    logger.log("Wake up.");
    sb->enableReceiver();
  }
  delay(2);
  long unsigned message_time = -1;
  
  if( wakeup_time && millis()-wakeup_time > WIFI_ENABLE_AFTER_WAKEUP_TIME_MS ){
    logger.log("Enabling WiFi...");
    wifi_fpm_do_wakeup();
    wifi_fpm_close();
    wifi_set_sleep_type(NONE_SLEEP_T);
    wifi_set_opmode(STATION_MODE);
    wifi_station_connect();
    wakeup_time = 0;
  }
  if(!connected && (WiFi.status() == WL_CONNECTED) ){
    logger.log("Wifi connected, ip: [%s].", ip_to_str(WiFi.localIP()));
    connected = true;
  }
  if(connected && (WiFi.status() != WL_CONNECTED) ){
    logger.log("Wifi disconnected.");
    connected = false;
  }
  
  SimplebusMessage message = sb->getMessage(&message_time); // Non blocking
  if(message.valid){
    time_last_event = millis();
    logger.log("time: [%d.%03ds] message: [%s].",message_time/1000, message_time%1000, message.toString().c_str());
  }
  bool ack = sb->getAck();
  if(ack){
    logger.log("ack.");
  }
  if(millis()-time_last_event > SLEEP_TIME_MS && !sb->isReceivingMessage()){
    time_last_event = millis();
    logger.log("Vcc: %.3f", ESP.getVcc()/1024.00f);
    logger.flush(true);
    if(!sb->isReceivingMessage()){
      sb->disableReceiver();
      goToSleep();
    }
  }
  //if( (millis()/2) % 1000 == 0){    Serial.println("alive");  }
}
