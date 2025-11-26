#include "RealTimeClock.h"
#include "SensorPCF85063.hpp"
#include "time.h"

SensorPCF85063 rtc;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5 * 3600;   // -18000 segundos
const int daylightOffset_sec = 0;       // Colombia no usa DST

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

void updateTimeWithWiFi(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  // Convertir a RTC_DateTime
  RTC_DateTime ntpDateTime(
    timeinfo.tm_year + 1900,
    timeinfo.tm_mon + 1,
    timeinfo.tm_mday,
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec
  );

  // Leer RTC actual
  RTC_DateTime datetime = rtc.getDateTime();
  rtc.setDateTime(ntpDateTime);
  Serial.println("RTC sincronizado con NTP");
}

DateTime getCurrentTime() {
  RTC_DateTime datetime = rtc.getDateTime();

  DateTime date;
  date.hours = datetime.getHour();
  date.minutes = datetime.getMinute();

  return date;
}