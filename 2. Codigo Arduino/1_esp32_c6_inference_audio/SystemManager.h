#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "HardwareConfig.h"

class SystemManager {
public:
  SystemManager(Arduino_GFX *gfx);
  void begin();
  void vibrate(AlertLevel lvl);

private:
  Arduino_GFX *gfx;
  void vibratePattern(int pulses, int pulseDuration);
};

#endif // SYSTEMMANAGER_H
