#include "AudioManager.h"

#define SAMPLE_RATE     16000
#define MCLK_MULTIPLE   256
#define MCLK_FREQ_HZ    (SAMPLE_RATE * MCLK_MULTIPLE)
#define SAMPLE_BITS     I2S_DATA_BIT_WIDTH_16BIT
#define SLOT_MODE       I2S_SLOT_MODE_MONO
#define SLOT_SELECT     I2S_STD_SLOT_LEFT

AudioManager::AudioManager() {}

bool AudioManager::begin() {
  if (!initCodec()) {
    Serial.println("❌ Error inicializando codec ES8311");
    return false;
  }

  initI2S();
  Serial.println("✅ AudioManager inicializado correctamente");
  return true;
}

bool AudioManager::initCodec() {
  es8311_handle_t es_handle = es8311_create(I2C_NUM_0, ES8311_ADDRRES_0);
  if (!es_handle) return false;

  const es8311_clock_config_t clk_cfg = {
    .mclk_inverted = false,
    .sclk_inverted = false,
    .mclk_from_mclk_pin = true,
    .mclk_frequency = MCLK_FREQ_HZ,
    .sample_frequency = SAMPLE_RATE
  };

  if (es8311_init(es_handle, &clk_cfg, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16) != ESP_OK)
    return false;

  es8311_voice_volume_set(es_handle, 70, NULL);
  es8311_microphone_config(es_handle, false);
  return true;
}

void AudioManager::initI2S() {
  i2s.setPins(I2S_BCK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN, I2S_DIN_PIN, I2S_MCK_PIN);
  if (!i2s.begin(I2S_MODE_STD, SAMPLE_RATE, SAMPLE_BITS, SLOT_MODE, SLOT_SELECT)) {
    Serial.println("❌ Error iniciando I2S");
    while (1);
  }
}

void AudioManager::readAudio(int16_t *buffer, size_t size) {
  i2s.readBytes((char *)buffer, size * sizeof(int16_t));
}
