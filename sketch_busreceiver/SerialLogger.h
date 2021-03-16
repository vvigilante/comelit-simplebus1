#include "Logger.h"
#include <WiFi.h>
#include <HTTPClient.h>

#ifndef SERIALLOGGER_H_ 
#define SERIALLOGGER_H_ 

#define BUF_SIZE_DEFAULT 1024

class SerialLogger : public Logger{
    private:
    public:
        SerialLogger(int serial_speed)
            :Logger(BUF_SIZE_DEFAULT){
            Serial.begin(serial_speed);
        }
        void flush(bool force = false){
            Serial.print(this->buffer);
            this->pos=0;
            this->buffer[0]='\0';
            if(force)
                Serial.flush();
        }
};

#endif /* SERIALLOGGER_H_ */
