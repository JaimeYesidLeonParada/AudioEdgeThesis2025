#include "SystemManager.h"
#include <Wire.h>

SystemManager::SystemManager(Arduino_GFX *gfx) {
  this->gfx = gfx;
}

void SystemManager::begin() {
  // Inicialización del bus I2C para sensores (RTC)
  Wire.begin(I2C_SDA, I2C_SCL);

  // Inicializa pantalla
  if (!gfx->begin()) {
    Serial.println("❌ Error al iniciar pantalla (SystemManager)");
    while (1) delay(1000);
  }
  gfx->fillScreen(RGB565_BLACK);

  // Backlight y pines
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

  // Mensaje inicial
  gfx->setCursor(10, 10);
  gfx->setTextColor(RGB565_GREEN);
  gfx->setTextSize(2);
  gfx->println("Sistema listo...");
  delay(1000);

  // Pines Init: motor vibrador y habilitar batería
  pinMode(MOTOR_VIBRATOR_PIN, OUTPUT);
  pinMode(BATTERY_ENABLE_PIN, OUTPUT);
  digitalWrite(BATTERY_ENABLE_PIN, HIGH);
  digitalWrite(MOTOR_VIBRATOR_PIN, LOW);
}

void SystemManager::vibrate(AlertLevel lvl) {
  switch (lvl) {
    case ALERT_NONE:
      digitalWrite(MOTOR_VIBRATOR_PIN, LOW);
      break;

    case ALERT_PARECE:
      vibratePattern(1, 200);  // 1 pulso suave
      break;

    case ALERT_HAY:
      vibratePattern(2, 250);  // 2 pulsos medianos
      break;

    case ALERT_ATENCION:
      vibratePattern(3, 300);  // 3 pulsos fuertes
      break;
    }
}

void SystemManager::vibratePattern(int pulses, int pulseDuration) {
    for (int i = 0; i < pulses; i++) {
        digitalWrite(MOTOR_VIBRATOR_PIN, HIGH);
        delay(pulseDuration);
        digitalWrite(MOTOR_VIBRATOR_PIN, LOW);
        delay(100);
    }
}


