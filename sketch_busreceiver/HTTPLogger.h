#include "Logger.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#ifndef HTTPLOGGER_H_ 
#define HTTPLOGGER_H_ 

#define BUF_SIZE_DEFAULT 4096
#define MIN_TIME_BETWEEN_FLUSH_MS_DEFAULT 20000
#define URL_DEFAULT "http://www.vvigilante.com/log/"

class HTTPLogger : public Logger{
    private:
        unsigned long last_flush_ms = 0;
        unsigned long min_time_between_flush_ms;
        const char* url;
    public:
        HTTPLogger(size_t buf_size=BUF_SIZE_DEFAULT, unsigned long min_time_between_flush_ms=MIN_TIME_BETWEEN_FLUSH_MS_DEFAULT, const char* url=URL_DEFAULT)
            :Logger(buf_size){
                this->min_time_between_flush_ms = min_time_between_flush_ms;
                this->url = url;
        }
        void flush(bool force = false){
            if(0==this->pos)
                return;
            unsigned long time_since_last_flush_ms = millis() - last_flush_ms;
            if(force || time_since_last_flush_ms > min_time_between_flush_ms){
                HTTPClient http;
                http.begin(this->url);
                http.addHeader("Content-Type", "text/plain");
                http.addHeader("X-Sender", WiFi.macAddress());
                int httpCode = http.POST((const uint8_t*)this->buffer, this->pos);
                http.end();
                if(HTTP_CODE_OK == httpCode){
                    this->pos = 0;
                    last_flush_ms = millis();
                }
            }
        }
};

#endif /* HTTPLOGGER_H_ */