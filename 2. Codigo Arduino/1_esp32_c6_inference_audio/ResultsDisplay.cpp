#include "ResultsDisplay.h"

ResultsDisplay::ResultsDisplay(Arduino_GFX *gfx)
  : gfx(gfx) {}

void ResultsDisplay::begin() {
  gfx->fillScreen(RGB565_BLACK);
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
}

void ResultsDisplay::showProbabilities(const ei_impulse_result_t &result) {
  // Limpiar el área donde estarán las barras
  gfx->fillRect(0, 220, gfx->width(), gfx->height() - 220, RGB565_BLACK);

  int y = 220;
  gfx->setTextSize(1);

  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    const char *label = result.classification[i].label;
    float value = result.classification[i].value;

    // Mostrar nombre de la clase
    gfx->setCursor(10, y);
    gfx->setTextColor(RGB565_WHITE);
    gfx->print(label);

    // Dibujar barra proporcional
    int barWidth = value * 180;
    gfx->fillRect(90, y, barWidth, 8, RGB565_CYAN);

    // Mostrar valor numérico
    gfx->setCursor(280 - 40, y);
    gfx->setTextColor(RGB565_YELLOW);
    gfx->printf("%.2f", value);

    y += 40; // separación entre barras
  }

  // Log para verificar en serial
  Serial.println("📈 Mostrando barras de probabilidad en pantalla...");
}

void ResultsDisplay::showResults(const ei_impulse_result_t& result) {
  gfx->fillScreen(RGB565_BLACK);

  // Restaurar fuente y estilo
  gfx->setFont((const GFXfont*)NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);

  int y = 105;

  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    gfx->setCursor(10, y);
    gfx->setTextColor(RGB565_WHITE);
    gfx->print(result.classification[ix].label);

    gfx->setCursor(160, y);
    gfx->setTextColor(RGB565_YELLOW);
    gfx->print(result.classification[ix].value, 2);

    int barWidth = result.classification[ix].value * 180;
    gfx->fillRect(10, y + 18, barWidth, 10, RGB565_GREEN);

    y += 40;
  }
}
