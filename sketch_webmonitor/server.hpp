#include "cert.h"
#include "private_key.h"
#include "data.h"

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


HTTPSServer secureServer = HTTPSServer(&cert, 443, MAX_CLIENTS);
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handleMakeCall(HTTPRequest * req, HTTPResponse * res);
void handleSetMic(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);

class MyWsHandler : public WebsocketHandler {
public:
  static WebsocketHandler* create();
  void onMessage(WebsocketInputStreambuf * input);
  void onClose();
};
MyWsHandler* activeClients[MAX_CLIENTS] = {NULL};


void server_setup() {
  for(int i = 0; i < MAX_CLIENTS; i++) activeClients[i] = nullptr;
  ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * nodeMakeCall    = new ResourceNode("/makeCall", "POST", &handleMakeCall);
  ResourceNode * nodeSetMic    = new ResourceNode("/setMic", "POST", &handleSetMic);
  ResourceNode * node404     = new ResourceNode("", "GET", &handle404);
  secureServer.registerNode(nodeRoot);
  secureServer.registerNode(nodeMakeCall);
  secureServer.registerNode(nodeSetMic);
  secureServer.setDefaultNode(node404);

  WebsocketNode * wsNode = new WebsocketNode("/ws", &MyWsHandler::create);
  secureServer.registerNode(wsNode);
  
  LOG("Starting server...");
  secureServer.start();
  if (secureServer.isRunning()) {
    LOG("Server OK");
  }
}

void server_loop() {
  secureServer.loop();
}


void handle404(HTTPRequest * req, HTTPResponse * res) {
  req->discardRequestBody();
  res->setStatusCode(404);
  res->setStatusText("Not Found");
  res->setHeader("Content-Type", "text/html");
  res->println("<!DOCTYPE html><html><head><title>Not Found</title></head><body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body></html>");
  res->finalize();
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  res->setHeader("Content-Type", "text/html");
  res->setHeader("Connection", "close");
  res->setHeader("Access-Control-Allow-Origin", "*");
  res->print( index_html );
}

void handleMakeCall(HTTPRequest * req, HTTPResponse * res){
  HTTPURLEncodedBodyParser parser(req);
  const int max_id_str_len = 8;
  char id_str[max_id_str_len] = {'\0'};
  while(parser.nextField()) {
      std::string name = parser.getFieldName();
      if (name == "userid") {
        size_t readLength = parser.read((byte *)id_str, max_id_str_len-1);
      }
  }
  req->discardRequestBody();
  res->setHeader("Content-Type", "application/json");
  res->setHeader("Access-Control-Allow-Origin", "*");
  res->setHeader("Connection", "close");
  makeCall(atoi(id_str));
  res->print( "{\"status\":\"ok\", \"id\":");
  res->print(id_str);
  res->println("}");
  res->finalize();
}
void handleSetMic(HTTPRequest * req, HTTPResponse * res){
  HTTPURLEncodedBodyParser parser(req);
  const int max_state_str_len = 3;
  char state_str[max_state_str_len] = {'\0'};
  while(parser.nextField()) {
      std::string name = parser.getFieldName();
      if (name == "state") {
        size_t readLength = parser.read((byte *)state_str, max_state_str_len-1);
      }
  }
  req->discardRequestBody();
  res->setHeader("Content-Type", "application/json");
  res->setHeader("Access-Control-Allow-Origin", "*");
  res->setHeader("Connection", "close");
  setMic( atoi(state_str) );
  res->print( "{\"status\":\"ok\", \"state\":");
  res->print(state_str);
  res->println("}");
  res->finalize();
}

WebsocketHandler * MyWsHandler::create() {
  Serial.println("New ws client connected!");
  MyWsHandler * handler = new MyWsHandler();
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] == nullptr) {
      activeClients[i] = handler;
      break;
    }
  }
  return handler;
}
void MyWsHandler::onClose() {
  Serial.println("Ws client disconnected!");
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] == this) {
      activeClients[i] = nullptr;
    }
  }
}

void MyWsHandler::onMessage(WebsocketInputStreambuf * inbuf) {
    if(inbuf->getRecordSize()!=NUM_SAMPLES){
        LOG("Invalid size received %u expected %u", inbuf->getRecordSize(), NUM_SAMPLES);
    }
    unsigned char mydata[NUM_SAMPLES];
    inbuf->sgetn((char*)mydata, NUM_SAMPLES);
    audio_recv_from_ws(mydata, NUM_SAMPLES);
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
bool server_ws_broadcast(const char* msg){
  bool done = false;
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] != nullptr) {
      activeClients[i]->send(msg, MyWsHandler::SEND_TYPE_TEXT);
      done = true;
    }
  }
  return done;
}
bool server_ws_broadcast_bin(const uint8_t* msg, size_t len){
  bool done = false;
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] != nullptr) {
      activeClients[i]->send((uint8_t*)msg, len, MyWsHandler::SEND_TYPE_BINARY);
      done = true;
    }
  }
  return done; 
}
