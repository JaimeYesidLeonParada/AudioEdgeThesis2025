#include "ClockDisplay.h"
#include "RealTimeClock.h"

ClockDisplay::ClockDisplay(Arduino_GFX *gfx) {
  this->gfx = gfx;
  lastClockUpdate = 0;
}

void ClockDisplay::begin() {
  pcf85063_init();
}

void ClockDisplay::update() {
  // Limpiar solo el área de resultados (debajo de la franja de la hora)
  gfx->fillRect(0, 0, gfx->width(), 85, RGB565_BLACK);

    DateTime datetime = getCurrentTime();
    char horaStr[10];
    sprintf(horaStr, "%02d:%02d", datetime.hours, datetime.minutes);

    gfx->setFont(u8g2_font_fub30_tn);
    uint16_t relojVerde = gfx->color565(0, 255, 179);
    gfx->setTextColor(relojVerde, RGB565_BLACK);

    int16_t x1, y1;
    uint16_t w, h;
    gfx->getTextBounds(horaStr, 0, 0, &x1, &y1, &w, &h);

    int16_t centerX = ((gfx->width() - w) / 2) - 10;
    int16_t posY = 85;

    gfx->fillRect(0, 0, gfx->width(), 70, RGB565_BLACK);
    gfx->setCursor(centerX, posY);
    gfx->println(horaStr);

    gfx->setFont((const GFXfont *)NULL);
    gfx->setTextSize(2);
    gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
}

void ClockDisplay::refreshTime() {
  updateTimeWithWiFi();
}
