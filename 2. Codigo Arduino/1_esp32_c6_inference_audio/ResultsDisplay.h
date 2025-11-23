#ifndef RESULTS_DISPLAY_H
#define RESULTS_DISPLAY_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "HardwareConfig.h"
#include "PostProcessor.h"

class ResultsDisplay {
public:
  ResultsDisplay(Arduino_GFX *gfx);
  void begin();
  AlertLevel showResults(const std::vector<ImpulseResult> &results);
  void setPostProcessor(PostProcessor *pp);

private:
  Arduino_GFX *gfx;
  PostProcessor *postProcessor = nullptr;
  String getAlertMessage(const String &label, AlertLevel lvl) const;
};

#endif