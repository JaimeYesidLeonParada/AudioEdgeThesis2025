#ifndef RESULTS_DISPLAY_H
#define RESULTS_DISPLAY_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <edge-impulse-sdk/classifier/ei_classifier_types.h> // Para ei_impulse_result_t

class ResultsDisplay {
public:
  ResultsDisplay(Arduino_GFX *gfx);
  void begin();
  void showResults(const ei_impulse_result_t &result);
  void showProbabilities(const ei_impulse_result_t &result);

private:
  Arduino_GFX *gfx;
};

#endif
