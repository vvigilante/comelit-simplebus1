#include "Simplebus.h"
#include "SimplebusState.h"
#include "SimplebusIntercom.h"
#include "CircularQueue.h"
#include "Logger.h"
#include "WebsocketLogger.h"
void makeCall(int userid);
#include "server.hpp"
#include "audio.hpp"
#include <ArduinoOTA.h>

/* Configuration */
#define QUEUE_SIZE          20
#define ACK_DELAY_TIME_MS   50
#define NOACK_DELAY_TIME_MS 10
/* Wemos pins */
#define BUS_RECEIVE_PIN      GPIO_NUM_5
#define BUS_TRANSMIT_PIN     GPIO_NUM_18
#define LED_BUILTIN          GPIO_NUM_2
/* Wifi settings */
#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"

struct SimplebusMessageStamped{
    unsigned long timestamp = -1;
    SimplebusMessage message;
    bool ack = false;
    String toString(){
      return String(timestamp) + String(" ") + message.toString() + String(ack?" ACKED":" noack");
    }
};

Simplebus* sb = NULL;
SimplebusIntercom* sbi = NULL;
SimplebusState state;
CircularQueue<SimplebusMessageStamped> queue(QUEUE_SIZE);

TaskHandle_t th0, th1, th2, th3;
int usertocall=0;
static void callTask(void* pvParameters){
  while(1){
    if(usertocall){
        sbi->makeCall(usertocall);
        usertocall = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void makeCall(int userid){
  usertocall = userid;
}

static void audioTask(void * pvParameters){
  while(1){
      vTaskDelay(pdMS_TO_TICKS(1));
      audio_sampling();
  }
}

static void serverTask(void * pvParameters){
  while(1){
        server_loop();
        ArduinoOTA.handle();
        if(get_num_connected_clients()){
          while(!queue.isEmpty()){
            SimplebusMessageStamped* element = queue.pop();
            sendmessage(element);
          }
          audio_plotting();
        }
        logger->flush(true);
        vTaskDelay(10);
    }
}

static void busTask(void * pvParameters){
  while(1){
    SimplebusMessageStamped m;
    m.message = sb->getMessage(&m.timestamp);
    if(m.message.valid){
      vTaskDelay(pdMS_TO_TICKS(ACK_DELAY_TIME_MS));
    }else{
      vTaskDelay(pdMS_TO_TICKS(NOACK_DELAY_TIME_MS));
    }
    unsigned long ack_time = sb->getAck();
    if(m.message.valid){
        // A new message has arrived
        if(ack_time>=m.timestamp){
            m.ack = true;
        }
        state.feed_message(m.message);
        if(m.ack){
            state.feed_ack();
        }
        queue.push(&m);
    }
  }
}

void setup(){
  logger = new WebsocketLogger();
  sb = new Simplebus(BUS_RECEIVE_PIN, BUS_TRANSMIT_PIN);
  sbi = new SimplebusIntercom(sb, 0);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    logger->log(false, ".");
    delay(500);
  }
  LOG("Wifi ok");
  LOG("IP: %s", WiFi.localIP().toString().c_str());
  server_setup();
  audio_setup();
  xTaskCreatePinnedToCore ( busTask, "busTask", 5000, NULL, 29, &th0, 0 );
  xTaskCreatePinnedToCore ( callTask, "callTask", 5000, NULL, 30, &th1, 0 );
  xTaskCreatePinnedToCore ( audioTask, "audioTask", 5000, NULL, 31, &th2, 0 );
  xTaskCreatePinnedToCore ( serverTask, "serverTask", 5000, NULL, 31, &th3, 1 );
  ArduinoOTA.begin();
  LOG("Init ok");
  
}

void sendmessage(SimplebusMessageStamped* element){
  String state_str = String(state.get_state());
  String userid_str = String(state.get_userid());
  String message = state_str+String(',')+userid_str+String(',')+element->toString();
  server_ws_broadcast(message.c_str());
}

void loop(){
  delay(10);
}
