#include "ResultsDisplay.h"

ResultsDisplay::ResultsDisplay(Arduino_GFX *gfx)
  : gfx(gfx), postProcessor(nullptr) {}

void ResultsDisplay::begin() {
  gfx->fillScreen(RGB565_BLACK);
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
}

void ResultsDisplay::setPostProcessor(PostProcessor *pp) {
  postProcessor = pp;
}

String ResultsDisplay::getAlertMessage(const String &label, AlertLevel lvl) const {
  if (lvl == ALERT_NONE) return String();

  // Normalizar label (acepta "baby-crying" o "LLanto", y "ambulance" o "Ambulancia")
  String l = label;
  l.toLowerCase();

  bool isLlanto = (l == "llanto" || l == "baby-crying" || l == "baby crying");
  bool isAmbulancia = (l == "ambulancia" || l == "ambulance");

  // Mensajes por etiqueta y nivel (español)
  if (isLlanto) {
    if (lvl == ALERT_PARECE) return String("Parece que\nun bebe llora");
    if (lvl == ALERT_HAY)    return String("Hay un bebe\nllorando");
    if (lvl == ALERT_ATENCION) return String("ATENCION:\nun bebe\nllora");
  } else if (isAmbulancia) {
    if (lvl == ALERT_PARECE) return String("Parece una\nambulancia");
    if (lvl == ALERT_HAY)    return String("Hay una\nambulancia\ncerca");
    if (lvl == ALERT_ATENCION) return String("ATENCION:\nambulancia\nsonando");
  }

  // fallback genérico
  if (lvl == ALERT_PARECE) return String("Parece: ") + label;
  if (lvl == ALERT_HAY)    return String("Hay: ") + label;
  if (lvl == ALERT_ATENCION) return String("ATENCION: ") + label;
  return String();
}


AlertLevel ResultsDisplay::showResults(const std::vector<ImpulseResult> &results) {
  // Limpiar área bajo el reloj
  gfx->fillRect(0, 85, gfx->width(), gfx->height(), RGB565_BLACK);

  // ==== 1. Consultar promedios y niveles desde PostProcessor ====
  String alertLabel = "";
  AlertLevel alertLevel = ALERT_NONE;

  if (postProcessor != nullptr) {
    for (const auto &res : results) {

      if (res.label == "baby-crying" || res.label == "ambulance") {

        AlertLevel lvl = postProcessor->getLevel(res.label);

        if (lvl != ALERT_NONE) {
          alertLabel = res.label;
          alertLevel = lvl;
          break;
        }
      }
    }
  }

  // ==== 2. Si hay alerta, mostrar mensaje ====
  if (alertLevel != ALERT_NONE) {
    String msg = getAlertMessage(alertLabel, alertLevel);

    Serial.println(">>> Mostrando ALERTA: " + msg);
    Serial.print(">>> Promedio = ");
    Serial.println(postProcessor->getAverage(alertLabel));

    gfx->setFont(u8g2_font_fub14_tf); 
    gfx->setTextColor(RGB565_RED, RGB565_BLACK);

    int16_t x1, y1;
    uint16_t w, h;
    gfx->getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);

    int16_t centerX = 0; 
    int16_t centerY = 140;

    gfx->setCursor(centerX, centerY);
    gfx->println(msg);
    delay(1000);

    return alertLevel;
  }

  // ==== 3. NO hay alerta → mostrar barras ====
  gfx->setFont((const GFXfont*)NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);

  int y = 105;

  for (const auto &res : results) {
    gfx->setCursor(10, y);
    gfx->setTextColor(RGB565_WHITE);
    gfx->print(res.label);

    gfx->setCursor(160, y);
    gfx->setTextColor(RGB565_YELLOW);
    gfx->print(res.value, 2);

    int barWidth = static_cast<int>(res.value * 180);
    gfx->fillRect(10, y + 18, barWidth, 10, RGB565_GREEN);

    y += 40;
  }

  return alertLevel;
}
