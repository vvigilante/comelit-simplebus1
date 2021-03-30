#include "Simplebus.h"
#include "SimplebusState.h"
#include "SimplebusIntercom.h"
#include "CircularQueue.h"
#include "Logger.h"
#include "WebsocketLogger.h"
#include "SerialLogger.h"
void makeCall(int userid);
#include "audio.hpp"
#include "server.hpp"
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

TaskHandle_t th0, th1, th2, th3, th4, th5;
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

static void audioreadTask(void * pvParameters){
  while(1){
      vTaskDelay(pdMS_TO_TICKS(10));
      audio_readbus();
  }
}
static void audiorecvtestTask(void * pvParameters){
  const size_t len = 1000;
  uint8_t data[len] = {127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111,127,143,159,174,188,202,214,225,234,242,248,252,254,254,252,248,242,234,225,214,202,188,174,159,143,127,111,95,80,66,52,40,29,20,12,6,2,0,0,2,6,12,20,29,40,52,66,80,95,111};
  audio_recv_from_ws(data, len);
  while(1){
      audio_recv_from_ws(data, len);
      vTaskDelay(pdMS_TO_TICKS(125));
  }
}
static void audiowriteTask(void * pvParameters){
  while(1){
      vTaskDelay(pdMS_TO_TICKS(10));
      audio_writebus();
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
          audio_send_to_ws();
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
  //logger = new WebsocketLogger();
  logger = new SerialLogger(115200);
  sb = new Simplebus(BUS_RECEIVE_PIN, BUS_TRANSMIT_PIN);
  sbi = new SimplebusIntercom(sb, 0);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  logger->log(false, "Waiting for wifi now");
  while (WiFi.status() != WL_CONNECTED) {
    logger->log(false, ".");
    delay(500);
  }
  LOG("Wifi ok");
  LOG("IP: %s", WiFi.localIP().toString().c_str());
  server_setup();
  audio_setup();
  xTaskCreatePinnedToCore ( busTask, "busTask", 4096, NULL, 29, &th0, 0 );
  xTaskCreatePinnedToCore ( callTask, "callTask", 4096, NULL, 30, &th1, 0 );
  xTaskCreatePinnedToCore ( audioreadTask, "audioreadTask", 4096, NULL, 31, &th2, 0 );
  xTaskCreatePinnedToCore ( audiowriteTask, "audiowriteTask", 4096, NULL, 31, &th3, 0 );
  xTaskCreatePinnedToCore ( serverTask, "serverTask", 6144, NULL, 31, &th4, 1 );
  //xTaskCreatePinnedToCore ( audiorecvtestTask, "audiorecvtestTask", 10000, NULL, 31, &th5, 1 );
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
  LOG("free:%lu block:%lu",heap_caps_get_free_size(MALLOC_CAP_DEFAULT), heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
  delay(1000);
}
