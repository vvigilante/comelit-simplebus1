#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <Arduino.h>

#ifndef LOGGER_H_
#define LOGGER_H_

#define MAX(a,b) ((a)>(b)?(a):(b))
class Logger
{
public:
    Logger(size_t buf_size){
        this->buffer = (char*)malloc(buf_size);
        this->pos = 0;
        this->buf_size = buf_size;
    }

   void log(const char* format, ...)
   {
        // use x, y and b
        const char* b = this->buffer+this->pos;
        va_list argptr;
        va_start(argptr, format);
        size_t n;
        unsigned long time = millis();
        n = snprintf(this->buffer+this->pos, MAX(0, this->buf_size-this->pos-2), "%d.%03d - ", time/1000, time%1000);
        this->pos += n;
        n = vsnprintf(this->buffer+this->pos, MAX(0, this->buf_size-this->pos-2), format, argptr);
        this->pos += n;
        if(this->pos<this->buf_size-1)
            this->buffer[this->pos++]='\n';
        this->buffer[this->pos]='\0';
        va_end(argptr);
        //Serial.print(b);
   }
   virtual void flush(bool force = false) = 0;

protected:
   char* buffer;
   size_t pos;
   size_t buf_size;
};

const char* ip_to_str(IPAddress ip){
    static char str_ip[16];
    char* pstr_ip=str_ip;
    for(int i=0; i<4; i++){
        if(i)
            *(pstr_ip++)='.';
        pstr_ip+=sprintf(pstr_ip, "%d", ip[i]);
    }
    return str_ip;
}

#endif /* LOGGER_H_ */