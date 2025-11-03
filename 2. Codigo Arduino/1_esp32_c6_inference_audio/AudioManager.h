#pragma once
#include <Arduino.h>
#include "ESP_I2S.h"
#include "es8311.h"
#include "HardwareConfig.h"

class AudioManager {
public:
  AudioManager();
  bool begin();                      // Inicializa el codec y el bus I2S
  void readAudio(int16_t *buffer, size_t size);

private:
  bool initCodec();
  void initI2S();

  I2SClass i2s;
};
