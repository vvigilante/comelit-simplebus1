
#include "CircularQueue.h"
#define RATE 4010
#define PERIOD_US (1000000/RATE)
#define BUFSIZE (8*1024)
long last_sample_time = 0;
long last_print_time = 0;
#define PRINT_PERIOD_US 1000000
CircularQueue<uint8_t> samples(BUFSIZE);

#define ANALOG_PIN A0

void setup(){
    Serial.begin(1000000);
    pinMode(ANALOG_PIN, INPUT);
    Serial.println("Hello!");
}
int x_int = -1;
void loop(){
    long now = micros();
    if( now - last_sample_time >= PERIOD_US ){
        last_sample_time = now;
        x_int = analogRead(ANALOG_PIN);
        uint8_t x = (x_int-1)/4;
        samples.push(&x);
    }
    if( now - last_print_time >= PRINT_PERIOD_US ){
        last_print_time = now;
        uint8_t* first = samples.pop();
        uint8_t* last = samples.getLast(1);
        Serial.printf("First: %d, Last: %d, Size: %d, x_int=%d\n", *first, *last, samples.getLength(), x_int);
        samples.clear();
    }
}