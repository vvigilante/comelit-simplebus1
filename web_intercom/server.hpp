#include "cert.h"
#include "private_key.h"

#include <WiFi.h>
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPURLEncodedBodyParser.hpp>
#include <WebsocketHandler.hpp>

using namespace httpsserver;
SSLCert cert = SSLCert(
  example_crt_DER, example_crt_DER_len,
  example_key_DER, example_key_DER_len
);
#define MAX_CLIENTS 2



static void handle404(HTTPRequest * req, HTTPResponse * res) {
  req->discardRequestBody();
  res->setStatusCode(404);
  res->setStatusText("Not Found");
  res->setHeader("Content-Type", "text/html");
  res->println("<html><head><title>Not Found</title></head><body><h1>404 Not Found");
}

static void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  res->setHeader("Content-Type", "text/html");
  res->setHeader("Connection", "close");
  res->setHeader("Access-Control-Allow-Origin", "*");
  res->print("<html><head><title>Simplebus Intercom</title></head><body><a href=\"https://www.vvigilante.com/intercom/?id=");
  res->print(WiFi.macAddress());
  res->println("\">Go");
}

static HTTPSServer secureServer(&cert, 443, MAX_CLIENTS);
class IntercomServer{
  public:
    typedef void(*audio_cb_t)(const uint8_t*, size_t);
    typedef void(*command_cb_t)(char);
  private:
    class MyWsHandler : public WebsocketHandler {
        public:
          void onMessage(WebsocketInputStreambuf * input);
          void onClose();
    };
    static MyWsHandler* activeClients[MAX_CLIENTS];
    static audio_cb_t audio_cb;
    static command_cb_t command_cb;
    
    
    static WebsocketHandler* create_wsh(){
      LOG("New ws client connected!");
      MyWsHandler * handler = new MyWsHandler();
      for(int i = 0; i < MAX_CLIENTS; i++) {
        if (activeClients[i] == nullptr) {
          activeClients[i] = handler;
          break;
        }
      }
      return handler;
    }
    

  public:
    
    IntercomServer(audio_cb_t audio_cb, command_cb_t command_cb){
        this->command_cb = command_cb;
        this->audio_cb = audio_cb;
        for(int i = 0; i < MAX_CLIENTS; i++) activeClients[i] = nullptr;
        ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
        ResourceNode * node404     = new ResourceNode("", "GET", &handle404);
        secureServer.registerNode(nodeRoot);
        secureServer.setDefaultNode(node404);
        WebsocketNode * wsNode = new WebsocketNode("/ws", &create_wsh);
        secureServer.registerNode(wsNode);
        LOG("Starting server...");
        secureServer.start();
        if (secureServer.isRunning()) {
          LOG("Server OK");
        }
    }
    void loop() {
      secureServer.loop();
    }
    
    int get_num_connected_clients(){
      int n=0;
      for(int i = 0; i < MAX_CLIENTS; i++) {
        if (activeClients[i] != nullptr) {
          n++;
        }
      }
      return n;
    }
    static bool server_ws_broadcast(const char* msg){
      bool done = false;
      for(int i = 0; i < MAX_CLIENTS; i++) {
        if (activeClients[i] != nullptr) {
          activeClients[i]->send(msg, MyWsHandler::SEND_TYPE_TEXT);
          done = true;
        }
      }
      return done;
    }
    static bool server_ws_broadcast_bin(const uint8_t* msg, size_t len){
      bool done = false;
      for(int i = 0; i < MAX_CLIENTS; i++) {
        if (activeClients[i] != nullptr) {
          activeClients[i]->send((uint8_t*)msg, len, MyWsHandler::SEND_TYPE_BINARY);
          done = true;
        }
      }
      return done; 
    }
};



void IntercomServer::MyWsHandler::onClose() {
  Serial.println("Ws client disconnected!");
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] == this) {
      activeClients[i] = nullptr;
    }
  }
}

void IntercomServer::MyWsHandler::onMessage(WebsocketInputStreambuf * inbuf) {
    static unsigned char mydata[NUM_SAMPLES];
    if(inbuf->getRecordSize()==1){
        // Single char command
        inbuf->sgetn((char*)mydata, 1);
        if(IntercomServer::command_cb)
          IntercomServer::command_cb(mydata[0]);
    }else if(inbuf->getRecordSize()==NUM_SAMPLES){
        inbuf->sgetn((char*)mydata, NUM_SAMPLES);
        if(IntercomServer::audio_cb)
          IntercomServer::audio_cb(mydata, NUM_SAMPLES);
    }else{
        LOG("Invalid size received %u expected %u", inbuf->getRecordSize(), NUM_SAMPLES);
    }
}

// TODO move in .c
IntercomServer::MyWsHandler* IntercomServer::activeClients[MAX_CLIENTS] = {NULL};
IntercomServer::audio_cb_t IntercomServer::audio_cb;
IntercomServer::command_cb_t IntercomServer::command_cb;
