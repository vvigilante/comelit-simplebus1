#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>
#include "WebSockets4WebServerSecure.h"

const char *ssid = "ssid";
const char *password = "pass";
const char *dname = "esp8266";

BearSSL::ESP8266WebServerSecure server(443);
WebSockets4WebServerSecure webSocket;

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIC6jCCAlOgAwIBAgIUZIw0cBcWDPJZe8ZIDu6bDqdwwvwwDQYJKoZIhvcNAQEL
BQAwejELMAkGA1UEBhMCUk8xCjAIBgNVBAgMAUIxEjAQBgNVBAcMCUJ1Y2hhcmVz
dDEbMBkGA1UECgwST25lVHJhbnNpc3RvciBbUk9dMRYwFAYDVQQLDA1PbmVUcmFu
c2lzdG9yMRYwFAYDVQQDDA1lc3A4MjY2LmxvY2FsMB4XDTE5MDQxMzE1NTMzOFoX
DTIwMDQxMjE1NTMzOFowejELMAkGA1UEBhMCUk8xCjAIBgNVBAgMAUIxEjAQBgNV
BAcMCUJ1Y2hhcmVzdDEbMBkGA1UECgwST25lVHJhbnNpc3RvciBbUk9dMRYwFAYD
VQQLDA1PbmVUcmFuc2lzdG9yMRYwFAYDVQQDDA1lc3A4MjY2LmxvY2FsMIGfMA0G
CSqGSIb3DQEBAQUAA4GNADCBiQKBgQCiZmrefwe6AwQc5BO+T/18IVyJJ007EASn
HocT7ODkL2HNgIKuQCnPimiysLh29tL1rRoE4v7qtpV4069BrMo2XqFvZkfbZo/c
qMcLJi43jSvWVUaWvk8ELlXNR/PX4627MilhC4bLD57VB7Q2AF4jrAVhBLzClqg0
RyCS1yab+wIDAQABo20wazAdBgNVHQ4EFgQUYvIljCgcnOfeRn1CILrj38c7Ke4w
HwYDVR0jBBgwFoAUYvIljCgcnOfeRn1CILrj38c7Ke4wDwYDVR0TAQH/BAUwAwEB
/zAYBgNVHREEETAPgg1lc3A4MjY2LmxvY2FsMA0GCSqGSIb3DQEBCwUAA4GBAI+L
mejdOgSCmsmhT0SQv5bt4Cw3PFdBj3EMFltoDsMkrJ/ot0PumdPj8Mukf0ShuBlL
alf/hel7pkwMbXJrQyt3+EN/u4SjjZZJT21Zbxbmo1BB/vy1fkugfY4F3JavVAQ/
F49UaclGs77AVkDYwKlRh5VWhmnfuXPN6NXkfV+z
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN PRIVATE KEY-----
MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBAKJmat5/B7oDBBzk
E75P/XwhXIknTTsQBKcehxPs4OQvYc2Agq5AKc+KaLKwuHb20vWtGgTi/uq2lXjT
r0GsyjZeoW9mR9tmj9yoxwsmLjeNK9ZVRpa+TwQuVc1H89fjrbsyKWELhssPntUH
tDYAXiOsBWEEvMKWqDRHIJLXJpv7AgMBAAECgYA5Syqu3mAKdt/vlWOFw9CpB1gP
JydvC+KoVvPOysY4mqLFjm4MLaTSjIENcZ1SkxewBubkDHVktw+atgvhfqVD4xnC
ewMpuN6Rku5A6EELhUoDrgMEt6M9D/0/iPaMm3VDtLXJq5SuKTpnM+vyE4/uM2Gu
4COfL4GQ0A5KWTzGcQJBANfpU/kwdZf8/oaOvpNZPGRsryjIXXuWMzKKM+M1RqSA
UQV596MGXjo8k8YG/A99rTmVhbeTMC2/7gIyGTePe/kCQQDAjZg2Ujz7wY3gf1Fi
ZETL7DHsss74sZyWZI490yIX0TQqKpXqEIKlkV+UZTOoSZiAaUyPjokblPmTkKfu
uMyTAkBIBjfS+o1fxC+L53Y/ZRc2UOMlcaFtpq8xftTMSGtmWL+uWf93zJoGR0rs
VkwjRsNQYEaY9Gqv+ESHSvsKg7zRAkEAoOLuhpzqVZThHe5jqumKzjS5dkPlScjl
xIeaji/msa3cf0r73goTj5HLIev5YKi1or3Y+a4oA4LTkifxGTcRvwJBAJB+qUE6
y8y+4yxStsWu362tn2o4EjyPL2UGc40wtlQng2GzPZ20+xVYcLxsJXE5/Jqg8IeI
elVVC46RfjDK9G0=
-----END PRIVATE KEY-----
)EOF";


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    Serial.printf("[%u] ev!\n", num);
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);
            break;
    }

}


bool connectToWifi() {
  byte timeout = 50;

  Serial.println("\n\n");

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  for (int i = 0; i < timeout; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi");
      Serial.print("Server can be accessed at https://");
      Serial.print(WiFi.localIP());
      if (MDNS.begin(dname)) {
        // https://superuser.com/questions/491747/how-can-i-resolve-local-addresses-in-windows
        Serial.print(" or at https://");
        Serial.print(dname);
        Serial.println(".local");
      }
      return true;
    }
    delay(5000);
    Serial.print(".");
  }

  Serial.println("\nFailed to connect to WiFi");
  Serial.println("Check network status and access data");
  Serial.println("Push RST to try again");
  return false;
}

void showWebpage() {
  String content = "<!DOCTYPE html><html>";
  content += "<head><title>ESP8266 Test Server</title>";
  content += "<link rel=\"shortcut icon\" href=\"data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA/4QAAAAA/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARAiIgEQAiAQASAAEAEgAhABAgAQASACARAAIBEQIiAQASACAQACABABIAIAEAAgARACIAABAAIAAAAAAAAAAAAAAAAAAAAAEREAERAAEAAQAAAAAQAQABAAAAABABAAERAAARAAERAQAAAQAAAQARAAABAAABABEREAAREAERCQmQAAZ2YAAGtmAACdEQAAZrsAAGbdAACZ7gAA//8AAP//AAAMdwAAf7cAAH+3AAAecQAAffYAAH32AAAOMQAA\" />";
  content += "</head><body>";
  content += "<script> var socket=null; "
  "function test(){ "
  "socket = new WebSocket('wss://esp8266.local/ws'); "
  "socket.onopen = function(event){console.log('conn');console.log(event);}; "
  "socket.onclose = function(event){console.log('close');console.log(event);}; "
  "}</script>";
  content += "<p><button onclick=\"test()\">test</button>.</p></body></html>";

  server.send(200, "text/html", content);
}

void setup() {
  Serial.begin(115200);

  if (!connectToWifi()) {
    delay(60000);
    ESP.restart();
  }

  server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
  server.on("/", showWebpage);
  server.begin();
  server.addHook(webSocket.hookForWebserver("/ws", webSocketEvent));
  
  Serial.println("Server is ready");
}

void loop() {
  server.handleClient();
  webSocket.loop();
  MDNS.update();
}
