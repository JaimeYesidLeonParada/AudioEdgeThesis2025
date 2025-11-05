#ifndef CLOUDMANAGER_H
#define CLOUDMANAGER_H

#include <Arduino.h>
#include "HardwareConfig.h"

extern void initProperties();
extern void ArduinoCloud_begin();
extern void ArduinoCloud_update();

class CloudManager {
public:
  CloudManager() {}
  void begin();
  void update();
  void showResults(const std::vector<ImpulseResult> &results);

private:
  // nada por ahora; añadir estado aquí solo si lo necesitas
};

#endif // CLOUDMANAGER_H
