#include <driver/i2s.h>
#include <driver/adc.h>
#include <soc/syscon_reg.h>
#include "CircularQueue.h"

#define DEBUG_AUDIO

extern bool server_ws_broadcast_bin(const uint8_t*, size_t);

#define N_BUFS 4
#define RATE 8000
#define NUM_SAMPLES 512
#define ADC_CHANNEL ADC1_CHANNEL_0 // GPIO 34

struct buf8_t{
  uint8_t buf[NUM_SAMPLES];
};
CircularQueue<buf8_t>* bufs_to_ws = NULL;
CircularQueue<buf8_t>* bufs_from_ws = NULL;


void audio_setup() {
    bufs_to_ws = new CircularQueue<buf8_t>(N_BUFS);
    bufs_from_ws = new CircularQueue<buf8_t>(N_BUFS);
    
    esp_err_t ret;
    i2s_config_t i2s_config =
        {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
            .sample_rate = RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ALL_LEFT,
            .communication_format = I2S_COMM_FORMAT_I2S_MSB,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 8,
            .dma_buf_len = 1024,
            .use_apll = 1,
        };
    ret = adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_11db);
    assert(ret == ESP_OK);
    ret = adc1_config_width(ADC_WIDTH_9Bit);
    assert(ret == ESP_OK);
    ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    assert(ret == ESP_OK);
    ret = i2s_set_sample_rates(I2S_NUM_0, RATE);
    assert(ret == ESP_OK);
    ret = i2s_set_adc_mode(ADC_UNIT_1, ADC_CHANNEL);
    assert(ret == ESP_OK);
    i2s_adc_enable(I2S_NUM_0);
    ret = i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN ); // GPIO25 = DAC1 = RIGHT
    assert(ret == ESP_OK);
    vTaskDelay(1000);
}


#ifdef DEBUG_AUDIO
  #define PRINT_CADENCY 64
  int n_read = 0;
  long last_ms = 0;
  int n_write = 0;
  long last_ms_w = 0;
#endif

static const inline void audio_writebus() {
    if(bufs_from_ws->getLength()>=N_BUFS-1){
        LOG("%d bufs from ws found.");
    }
    buf8_t* buf8 = bufs_from_ws->pop();
    if(buf8){
        // Convert buf8 to 16 bit format for DAC
        static uint16_t writebuf[NUM_SAMPLES*2];
        for(int i=0; i<NUM_SAMPLES; i++){
          writebuf[i*2] = buf8->buf[i]<<8;
        }
        size_t bw;
        int ret = i2s_write(I2S_NUM_0, ((char *)writebuf), NUM_SAMPLES *2 * sizeof(uint16_t), &bw, portMAX_DELAY);
        #ifdef DEBUG_AUDIO
        {  // Loogging
          n_write++;
          if(n_write %PRINT_CADENCY == 0){
            long now = millis();
            LOG("inter-write time = %ld ms", (now-last_ms_w)/PRINT_CADENCY );
            last_ms_w = now;
          }
        }
        #endif
    }
}



static const inline void audio_readbus() {
  size_t br;
  static uint16_t buf[NUM_SAMPLES*2];
  int ret = i2s_read(I2S_NUM_0, ((char *)buf), NUM_SAMPLES *2 * sizeof(uint16_t), &br, portMAX_DELAY);
  buf8_t buf8;
  { // Convert to buf8
    uint16_t* endbuf = buf+(NUM_SAMPLES*2);
    uint16_t *ps;
    uint8_t *pd;
    for(ps=buf, pd=buf8.buf; ps<endbuf; ps+=2, pd++){
        *pd = (*ps) >> 3;
    }
  }
  bufs_to_ws->push(&buf8); // TODO Here data gets copied again. Optimize later
  #ifdef DEBUG_AUDIO
  {
    // Loogging
    n_read++;
    if(n_read %PRINT_CADENCY == 0){
      long now = millis();
      LOG("inter-read time = %ld ms", (now-last_ms)/PRINT_CADENCY );
      last_ms = now;
    }
  }
  #endif
}

static const inline void audio_recv_from_ws(unsigned char* data, size_t len) {
  buf8_t buf8;
  if(len!=NUM_SAMPLES){
    LOG("audio_recvws unexpected size %u != %u", len, NUM_SAMPLES);
    return;
  }
  memcpy(buf8.buf, data, len); // TODO Here data gets copied again. Optimize later
  bufs_from_ws->push(&buf8);
}
static const inline void audio_send_to_ws() {
    if(bufs_to_ws->getLength()>=N_BUFS-1){
        LOG("%d bufs to ws found.");
    }
    buf8_t* buf8 = bufs_to_ws->pop();
    if(buf8){
      server_ws_broadcast_bin(buf8->buf, NUM_SAMPLES);
    }
}
