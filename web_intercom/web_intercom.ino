#include "SimplebusIntercom.h"
#include "Logger.h"
#include "WebsocketLogger.h"
#include "audio.hpp"
#include "server.hpp"
#include "SerialLogger.h"
#include <ArduinoOTA.h>

#define RING_INTERVAL_MS 1000

// CONFIGURATION
#define BUS_ID 27
#define BUS_RECEIVE_PIN      GPIO_NUM_5
#define BUS_TRANSMIT_PIN     GPIO_NUM_18
#define LED_BUILTIN          GPIO_NUM_2
#define MIC_ENABLE_PIN       GPIO_NUM_15

#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"


// Global state
unsigned long last_ring_sent=0;
bool audio_enabled = 0;
bool ringer_enabled = 0;
Simplebus* sb = NULL;
SimplebusIntercom* sbi = NULL;
IntercomAudio* sba = NULL;
IntercomServer* svr = NULL;



// Tasks

static void ledTask(void * pvParameters){
  while(1){
      digitalWrite(LED_BUILTIN, LOW);
      int duty = 970;
      if (WiFi.status() != WL_CONNECTED) {
          duty = 500;
      }
      if(audio_enabled){
          duty = 20;
      }
      vTaskDelay(pdMS_TO_TICKS(duty));
      digitalWrite(LED_BUILTIN, HIGH);
      vTaskDelay(pdMS_TO_TICKS(1000-duty));
  }
}
static void audioTask1(void * pvParameters){
  while(1){
      vTaskDelay(pdMS_TO_TICKS(10));
      sba->readbus();
  }
}
static void audioTask2(void * pvParameters){
  while(1){
      vTaskDelay(pdMS_TO_TICKS(10));
      sba->writebus();
  }
}
static void serverTask(void * pvParameters){
  while(1){
        svr->loop();
        ArduinoOTA.handle();
        if(svr->get_num_connected_clients()){
            logger->flush(true);
            sba->send_to_ws();
        }
        logger->flush(true);
        if(ringer_enabled && millis()>=last_ring_sent+RING_INTERVAL_MS){
            svr->server_ws_broadcast("R");
            last_ring_sent = millis();
        }
        vTaskDelay(10);
    }
}

static void busTask(void * pvParameters){
  while(1){
    sbi->loop();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}




// Logic

void ringerCb(bool enable){
  LOG("Set ringer: %d", enable);
  ringer_enabled=enable;
  if(!enable)
    svr->server_ws_broadcast("r");
}
void audioCb(bool enable){
  LOG("Set audio: %d", enable);
  audio_enabled = enable;
  svr->server_ws_broadcast(enable?"A":"a");
  digitalWrite(MIC_ENABLE_PIN, enable);
}
void audio_to_ws_cb(const uint8_t* data, size_t size){
  if(audio_enabled)
    svr->server_ws_broadcast_bin(data, size);
}
void audio_from_ws_cb(const uint8_t* data, size_t size){
  if(audio_enabled)
    sba->recv_from_ws(data,size);
}
void command_from_ws_cb(char command){
  //LOG("Read %d bytes of audio from ws", size);
  switch(command){
    case 'P':
      sbi->setUserPickup();
      break;
    case 'H':
      sbi->setUserHangup();
      break;
    case 'O':
      sbi->sendOpen();
      break;
    default:
      LOG("Command not understood: %c", command);
      break;
  }
}



void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(MIC_ENABLE_PIN, 0);
  pinMode(MIC_ENABLE_PIN, OUTPUT);
  logger = new WebsocketLogger(svr->server_ws_broadcast);
  //logger = new SerialLogger(115200);
  xTaskCreatePinnedToCore ( ledTask, "ledTask", 1024, NULL, 31, NULL, 1 );

  // Digital bus
  sb = new Simplebus(BUS_RECEIVE_PIN, BUS_TRANSMIT_PIN);
  sbi = new SimplebusIntercom(sb, BUS_ID, audioCb, ringerCb);
  
  // Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  logger->log(false, "Waiting for wifi now");
  while (WiFi.status() != WL_CONNECTED) {
    logger->log(false, ".");
    delay(500);
  }
  LOG("Wifi ok");
  LOG("IP: %s", WiFi.localIP().toString().c_str());
  
  // Server
  svr = new IntercomServer(audio_from_ws_cb, command_from_ws_cb);
  
  // Analog audio
  sba = new IntercomAudio(audio_to_ws_cb);

  // Tasks
  xTaskCreatePinnedToCore ( audioTask1, "audioTask1", 2048, NULL, 30, NULL, 0 );
  xTaskCreatePinnedToCore ( audioTask2, "audioTask2", 2048, NULL, 30, NULL, 0 );
  xTaskCreatePinnedToCore ( busTask, "busTask", 4096, NULL, 29, NULL, 0 );
  xTaskCreatePinnedToCore ( serverTask, "serverTask", 4096, NULL, 30, NULL, 1 );
  
  // OTA
  ArduinoOTA.begin();
  // Make sure there is at least 60k per connection (2 in our case -> 120k)
  LOG("Init ok, free heap: %lu", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}


void loop(){
  vTaskDelay(pdMS_TO_TICKS(500));
}
