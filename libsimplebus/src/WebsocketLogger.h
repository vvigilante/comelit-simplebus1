#include "Logger.h"
extern bool server_ws_broadcast(const char* );

#ifndef WEBSOCKETLOGGER_H_ 
#define WEBSOCKETLOGGER_H_  

#define BUF_SIZE_DEFAULT 1024
#define MIN_TIME_BETWEEN_FLUSH_MS_DEFAULT 1000

/**
 * Logs by sending text to a websocket through a callback
 */
class WebsocketLogger : public Logger{
    public:
        typedef bool(*broadcast_fcn_t)(const char*);
    private:
        unsigned long last_flush_ms = 0;
        unsigned long min_time_between_flush_ms;
        broadcast_fcn_t ws_broadcast_fcn;
    public:
        WebsocketLogger(broadcast_fcn_t ws_broadcast_fcn, unsigned long min_time_between_flush_ms=MIN_TIME_BETWEEN_FLUSH_MS_DEFAULT)
            :Logger(BUF_SIZE_DEFAULT){
            this->last_flush_ms = millis();
            this->min_time_between_flush_ms = min_time_between_flush_ms;
            this->pos=1;
            this->buffer[0]='L';
            this->buffer[1]='\0';
            this->ws_broadcast_fcn = ws_broadcast_fcn;
        }
        void flush(bool force = false){
            if(!force)
              return;
            if(this->pos<=1)
              return;
            unsigned long time_since_last_flush_ms = millis() - last_flush_ms;
            if(time_since_last_flush_ms > min_time_between_flush_ms){
                bool done = (this->ws_broadcast_fcn)(this->buffer);
                if(done){
                  this->pos=1;
                  this->buffer[1]='\0';
                  this->last_flush_ms = millis();
                }
            }
        }
};

#endif /* WEBSOCKETLOGGER_H_ */
