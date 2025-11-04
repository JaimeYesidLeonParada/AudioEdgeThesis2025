#ifndef RESULTS_DISPLAY_H
#define RESULTS_DISPLAY_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "HardwareConfig.h"

class ResultsDisplay {
public:
  ResultsDisplay(Arduino_GFX *gfx);
  void begin();
  void showResults(const std::vector<ImpulseResult> &results);

private:
  Arduino_GFX *gfx;
};

#endif