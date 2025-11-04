#include "ResultsDisplay.h"

ResultsDisplay::ResultsDisplay(Arduino_GFX *gfx)
  : gfx(gfx) {}

void ResultsDisplay::begin() {
  gfx->fillScreen(RGB565_BLACK);
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
}

void ResultsDisplay::showResults(const std::vector<ImpulseResult> &results) {
  // Limpiar solo el área de resultados (debajo de la franja de la hora)
  gfx->fillRect(0, 85, gfx->width(), gfx->height(), RGB565_BLACK);

  // Restaurar fuente y estilo
  gfx->setFont((const GFXfont*)NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);

  int y = 105;  // empezar a dibujar debajo de la hora

  for (const auto &res : results) {
    // Etiqueta
    gfx->setCursor(10, y);
    gfx->setTextColor(RGB565_WHITE);
    gfx->print(res.label);

    // Valor numérico
    gfx->setCursor(160, y);
    gfx->setTextColor(RGB565_YELLOW);
    gfx->print(res.value, 2);

    // Barra proporcional
    int barWidth = static_cast<int>(res.value * 180);
    gfx->fillRect(10, y + 18, barWidth, 10, RGB565_GREEN);

    y += 40;
  }
}