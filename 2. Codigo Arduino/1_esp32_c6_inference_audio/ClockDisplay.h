#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <U8g2lib.h>

class ClockDisplay {
public:
  ClockDisplay(Arduino_GFX *gfx);
  void begin();
  void update();

private:
  Arduino_GFX *gfx;
  uint32_t lastClockUpdate;
};

#endif
