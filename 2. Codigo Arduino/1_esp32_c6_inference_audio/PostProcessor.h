#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <Arduino.h>
#include "HardwareConfig.h"

// Nivel de alerta para una clase
/*enum AlertLevel {
  ALERT_NONE = 0,    // sin mensaje
  ALERT_PARECE,      // > umbral1  (ej. "Parece que...")
  ALERT_HAY,         // > umbral2  (ej. "Hay un...")
  ALERT_ATENCION     // > umbral3  (ej. "Atención...")
};*/

class PostProcessor {
public:
  // labels: arreglo de String con las etiquetas que vas a usar (ej "LLanto","Ambulancia")
  // numLabels: cuantas etiquetas
  // windowSize: cuantas últimas muestras promediar (ej. 5)
  PostProcessor(const String *labels, int numLabels, int windowSize);

  ~PostProcessor();

  // Agrega una muestra (label debe ser exacto a uno pasado en el constructor)
  void addSample(const String &label, float value);

  // Devuelve el promedio actual para la label (si no existe devuelve 0)
  float getAverage(const String &label) const;

  // Devuelve el nivel de alerta según umbrales configurados para la label
  AlertLevel getLevel(const String &label) const;

  // Permite ajustar los umbrales para una etiqueta (t1 < t2 < t3)
  bool setThresholds(const String &label, float t1, float t2, float t3);

private:
  struct Window {
    String label;
    float *buffer;    // buffer dinámico de tamaño windowSize
    int head;         // índice para insertar (circular)
    int count;        // cuantas muestras acumuladas (hasta windowSize)
    // umbrales
    float t1, t2, t3;
  };

  Window *windows;
  int labelsCount;
  int wSize;

  // busca índice de label en windows, -1 si no existe
  int findIndex(const String &label) const;
};

#endif // POSTPROCESSOR_H
