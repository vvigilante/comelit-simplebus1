#include <driver/i2s.h>
#include <driver/adc.h>
#include <soc/syscon_reg.h>

extern bool server_ws_broadcast_bin(const uint8_t*, size_t);

#define RATE 8000
#define NUM_SAMPLES 2000
#define ADC_CHANNEL ADC1_CHANNEL_0 // GPIO 34

static QueueHandle_t i2s_event_queue;

void audio_setup() {
    esp_err_t ret;
    i2s_config_t i2s_config =
        {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),  // I2S receive mode with ADC
            .sample_rate = RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ALL_LEFT,
            .communication_format = I2S_COMM_FORMAT_PCM,
            .intr_alloc_flags = 0,
            .dma_buf_count = 8,
            .dma_buf_len = 1000,
            .use_apll = 1,
        };
    ret = adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_11db);
    assert(ret == ESP_OK);

    ret = adc1_config_width(ADC_WIDTH_12Bit);
    assert(ret == ESP_OK);
    ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 1, &i2s_event_queue);
    assert(ret == ESP_OK);
    
    //ret = i2s_set_clk(I2S_NUM_0, RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    //assert(ret == ESP_OK);
    ret = i2s_set_sample_rates(I2S_NUM_0, RATE);
    assert(ret == ESP_OK);
    
    ret = i2s_set_adc_mode(ADC_UNIT_1, ADC_CHANNEL);
    assert(ret == ESP_OK);
    i2s_adc_enable(I2S_NUM_0);
    vTaskDelay(1000);
}

#define N_BUFS 4
uint16_t i2s_read_buff[N_BUFS][NUM_SAMPLES];
size_t bytes_read[N_BUFS];
int cur_buf = 0;

#define PRINT_CADENCY 64
int n_read = 0;
long last_ms = 0;

static const inline void audio_sampling() {
  system_event_t evt;
  if (xQueueReceive(i2s_event_queue, &evt, portMAX_DELAY) == pdPASS) {
    if (evt.event_id==2) {
        size_t br;
        uint16_t* buf = i2s_read_buff[cur_buf];
        int ret = i2s_read(I2S_NUM_0, ((char *)buf), NUM_SAMPLES * sizeof(uint16_t), &br, portMAX_DELAY);
        bytes_read[cur_buf] = br;
        cur_buf = (cur_buf+1) % N_BUFS;
        // Loogging
        n_read++;
        if(n_read %PRINT_CADENCY == 0){
          long now = millis();
          LOG("inter-read time = %ld ms", (now-last_ms)/PRINT_CADENCY );
          last_ms = now;
        }
    }
  }
}

uint16_t transmitbuf[NUM_SAMPLES/2];
static const inline void audio_plotting() {
  int last_buf = cur_buf;
  int first_buf = (cur_buf+1)%N_BUFS;
  for(int i=first_buf; i!=last_buf; i=(i+1)%N_BUFS){
    size_t br = bytes_read[i];
    uint16_t* buf = i2s_read_buff[i];
    if(br>0){
      uint16_t* endbuf = buf+NUM_SAMPLES;
      uint16_t *ps, *pd;
      for(ps=buf, pd=transmitbuf; ps<endbuf; ps+=2, pd++){
          *pd = *ps;
      }
      server_ws_broadcast_bin((const uint8_t*)transmitbuf, NUM_SAMPLES/2);
      bytes_read[i] = 0;
    }
  }
}
