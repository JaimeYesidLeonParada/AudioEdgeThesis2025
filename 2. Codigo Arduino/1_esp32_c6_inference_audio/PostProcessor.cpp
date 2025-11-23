#include "PostProcessor.h"

PostProcessor::PostProcessor(const String *labels, int numLabels, int windowSize)
  : labelsCount(numLabels), wSize(windowSize)
{
  if (labelsCount <= 0) {
    windows = nullptr;
    return;
  }

  windows = new Window[labelsCount];
  for (int i = 0; i < labelsCount; ++i) {
    windows[i].label = labels[i];
    windows[i].buffer = new float[wSize];
    // inicializar buffer con 0
    for (int j = 0; j < wSize; ++j) windows[i].buffer[j] = 0.0f;
    windows[i].head = 0;
    windows[i].count = 0;
    // umbrales por defecto (puedes cambiarlos con setThresholds)
    windows[i].t1 = 0.6f; // "Parece"
    windows[i].t2 = 0.7f; // "Hay"
    windows[i].t3 = 0.8f; // "Atención"
  }
}

PostProcessor::~PostProcessor() {
  if (!windows) return;
  for (int i = 0; i < labelsCount; ++i) {
    delete[] windows[i].buffer;
  }
  delete[] windows;
}

int PostProcessor::findIndex(const String &label) const {
  if (!windows) return -1;
  for (int i = 0; i < labelsCount; ++i) {
    if (windows[i].label == label) return i;
  }
  return -1;
}

void PostProcessor::addSample(const String &label, float value) {
  int idx = findIndex(label);
  if (idx < 0) return; // etiqueta no conocida: ignorar
  Window &w = windows[idx];
  w.buffer[w.head] = value;
  w.head = (w.head + 1) % wSize;
  if (w.count < wSize) w.count++;
}

float PostProcessor::getAverage(const String &label) const {
  int idx = findIndex(label);
  if (idx < 0) return 0.0f;
  const Window &w = windows[idx];
  if (w.count == 0) return 0.0f;
  float sum = 0.0f;
  for (int i = 0; i < w.count; ++i) sum += w.buffer[i];
  return sum / w.count;
}

AlertLevel PostProcessor::getLevel(const String &label) const {
  int idx = findIndex(label);
  if (idx < 0) return ALERT_NONE;
  const Window &w = windows[idx];
  float avg = 0.0f;
  if (w.count == 0) return ALERT_NONE;
  float sum = 0.0f;
  for (int i = 0; i < w.count; ++i) sum += w.buffer[i];
  avg = sum / w.count;

  if (avg > w.t3) return ALERT_ATENCION;
  if (avg > w.t2) return ALERT_HAY;
  if (avg > w.t1) return ALERT_PARECE;
  return ALERT_NONE;
}

bool PostProcessor::setThresholds(const String &label, float t1, float t2, float t3) {
  int idx = findIndex(label);
  if (idx < 0) return false;
  if (!(t1 < t2 && t2 < t3)) return false;
  windows[idx].t1 = t1;
  windows[idx].t2 = t2;
  windows[idx].t3 = t3;
  return true;
}
