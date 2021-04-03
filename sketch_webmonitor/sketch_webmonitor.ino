#include "Simplebus.h"
#include "SimplebusState.h"
#include "SimplebusIntercom.h"
#include "CircularQueue.h"
#include "Logger.h"
#include "WebsocketLogger.h"
#include "SerialLogger.h"
void makeCall(int userid);
void setMic(int state);
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
#define MIC_ENABLE_PIN       GPIO_NUM_15
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

bool mic_enabled = 0;
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
void setMic(int state){
  LOG("Mic enable: %d", state);
  mic_enabled = state;
  digitalWrite(MIC_ENABLE_PIN, state);
}

static void audioreadTask(void * pvParameters){
  while(1){
      vTaskDelay(pdMS_TO_TICKS(10));
      audio_readbus();
  }
}
static void ledTask(void * pvParameters){
  while(1){
      digitalWrite(LED_BUILTIN, LOW);
      int duty = 970;
      if (WiFi.status() != WL_CONNECTED) {
          duty = 500;
      }
      if(mic_enabled){
          duty = 20;
      }
      vTaskDelay(pdMS_TO_TICKS(duty));
      digitalWrite(LED_BUILTIN, HIGH);
      vTaskDelay(pdMS_TO_TICKS(1000-duty));
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
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(MIC_ENABLE_PIN, 0);
  pinMode(MIC_ENABLE_PIN, OUTPUT);
  //logger = new WebsocketLogger();
  logger = new SerialLogger(115200);
  
  xTaskCreatePinnedToCore ( ledTask, "ledTask", 512, NULL, 31, &th5, 1 );
  
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
  xTaskCreatePinnedToCore ( busTask, "busTask", 1024, NULL, 29, &th0, 0 );
  xTaskCreatePinnedToCore ( callTask, "callTask", 512, NULL, 30, &th1, 0 );
  xTaskCreatePinnedToCore ( audioreadTask, "audioreadTask", 2048, NULL, 30, &th2, 0 );
  xTaskCreatePinnedToCore ( audiowriteTask, "audiowriteTask", 2048, NULL, 30, &th3, 0 );
  xTaskCreatePinnedToCore ( serverTask, "serverTask", 4096, NULL, 30, &th4, 1 );
  ArduinoOTA.begin();
  // Make sure there is at least 60k per connection (2 in our case -> 120k)
  LOG("Init ok, free heap: %lu", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

void sendmessage(SimplebusMessageStamped* element){
  String state_str = String(state.get_state());
  String userid_str = String(state.get_userid());
  String message = state_str+String(',')+userid_str+String(',')+element->toString();
  server_ws_broadcast(message.c_str());
}

void loop(){
  vTaskDelay(pdMS_TO_TICKS(500));
}
