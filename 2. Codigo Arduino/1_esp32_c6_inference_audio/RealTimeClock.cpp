#include "RealTimeClock.h"
#include "SensorPCF85063.hpp"

SensorPCF85063 rtc;

// --- Inicialización RTC ---
void pcf85063_init(void) {
  if (!rtc.begin(Wire)) {
    Serial.println("❌ No se detecta RTC PCF85063");
    while (1) {
      delay(1000);
    }
  }
  RTC_DateTime datetime = rtc.getDateTime();
  if (datetime.getYear() < 2024) {
    rtc.setDateTime(2025, 11, 3, 12, 32, 0);
  }
}

DateTime getCurrentTime() {
  RTC_DateTime datetime = rtc.getDateTime();

  DateTime date;
  date.hours = datetime.getHour();
  date.minutes = datetime.getMinute();

  return date;
}