#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

class SystemManager {
public:
  SystemManager(Arduino_GFX *gfx);
  void begin();

private:
  Arduino_GFX *gfx;
};

#endif // SYSTEMMANAGER_H
