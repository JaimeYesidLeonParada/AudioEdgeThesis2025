#include <Arduino.h>
#include "ESP_I2S.h"

#include "esp_check.h"

#include "Wire.h"
#include "es8311.h"

#include "OneButton.h"

#define BOOT_PIN 9

#define I2C_SDA 8
#define I2C_SCL 7

#define I2S_NUM I2S_NUM_0
#define I2S_MCK_PIN 19   
#define I2S_BCK_PIN 20   
#define I2S_LRCK_PIN 22  
#define I2S_DOUT_PIN 23  
#define I2S_DIN_PIN 21   

#define EXAMPLE_SAMPLE_RATE (16000)
#define EXAMPLE_MCLK_MULTIPLE (256)  // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)
#define EXAMPLE_VOICE_VOLUME (70)

I2SClass i2s;

OneButton boot_button(BOOT_PIN, true);


SemaphoreHandle_t es8311_recording_BinarySemaphore;
SemaphoreHandle_t es8311_stop_BinarySemaphore;

void setupI2S() {
  i2s.setPins(I2S_BCK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN, I2S_DIN_PIN, I2S_MCK_PIN);
  // Initialize the I2S bus in standard mode
  if (!i2s.begin(I2S_MODE_STD, EXAMPLE_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    Serial.println("Failed to initialize I2S bus!");
    return;
  }
}


static esp_err_t es8311_codec_init(void) {
  es8311_handle_t es_handle = es8311_create(I2C_NUM_0, ES8311_ADDRRES_0);
  ESP_RETURN_ON_FALSE(es_handle, ESP_FAIL, TAG, "es8311 create failed");
  const es8311_clock_config_t es_clk = {
    .mclk_inverted = false,
    .sclk_inverted = false,
    .mclk_from_mclk_pin = true,
    .mclk_frequency = EXAMPLE_MCLK_FREQ_HZ,
    .sample_frequency = EXAMPLE_SAMPLE_RATE
  };

  ESP_ERROR_CHECK(es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));
  ESP_RETURN_ON_ERROR(es8311_voice_volume_set(es_handle, EXAMPLE_VOICE_VOLUME, NULL), TAG, "set es8311 volume failed");
  ESP_RETURN_ON_ERROR(es8311_microphone_config(es_handle, false), TAG, "set es8311 microphone failed");

  return ESP_OK;
}


void longPressStop() {
  Serial.println("BOOT Button longPress stop");
  xSemaphoreGive(es8311_stop_BinarySemaphore);
}  // longPressStop


void longPressStart() {
  Serial.println("BOOT Button longPress start");
  xSemaphoreGive(es8311_recording_BinarySemaphore);
}  // longPressStart


void es8311_test_task(void *arg)
{
  const int limit_size_max = 1600;
  uint8_t data[limit_size_max];
  while (1)
  {
    if (xSemaphoreTake(es8311_recording_BinarySemaphore, portMAX_DELAY) == pdTRUE)
    {
      while (1)
      {
        i2s.readBytes((char *)data, limit_size_max);
        i2s.write(data, limit_size_max);
        if (xSemaphoreTake(es8311_stop_BinarySemaphore, 0) == pdTRUE)
          break;
      }
    }
  }
}

void setup() {

  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  es8311_codec_init();

  setupI2S();
  Serial.println("I2S Initialized");
  es8311_recording_BinarySemaphore = xSemaphoreCreateBinary();
  es8311_stop_BinarySemaphore = xSemaphoreCreateBinary();

  boot_button.attachLongPressStart(longPressStart);
  boot_button.attachLongPressStop(longPressStop);
  xTaskCreate(es8311_test_task, "es8311_test_task", 1024 * 4, NULL, 5, NULL);
}

void loop() {
  boot_button.tick();
  delay(10);
}
