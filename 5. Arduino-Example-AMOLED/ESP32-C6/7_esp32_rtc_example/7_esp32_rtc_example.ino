#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include "SensorPCF85063.hpp"

#define LCD_SCK 1
#define LCD_DIN 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

#define I2C_SDA 8  // Display Wire SDA Pin
#define I2C_SCL 7  // Display Wire SCL Pin

#define GFX_BL LCD_BL

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0 /* rotation */, true /* IPS */,
  240 /* width */, 280 /* height */,
  0 /* col offset 1 */, 20 /* row offset 1 */,
  0 /* col offset 2 */, 20 /* row offset 2 */);

SensorPCF85063 rtc;


void pcf85063_init(void) {
    
    if (!rtc.begin(Wire)) {
        while (1) {
        Serial.println("Failed to find PCF8563 - check your wiring!");
        delay(1000);
        }
    }
    RTC_DateTime datetime = rtc.getDateTime();

    if (datetime.getYear() < 2025) {
        rtc.setDateTime(2025, 1, 1, 12, 0, 0);
    }
}

void setup(void) {
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX Hello World example");

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_WHITE);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  pcf85063_init();
  gfx->setTextSize(3);
}

void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 1000) {
      lastMillis = millis();
      RTC_DateTime datetime = rtc.getDateTime();
      gfx->setCursor(0, 20);
      gfx->setTextColor( RGB565_RED, RGB565_WHITE);
      gfx->printf("  %04d-%02d-%02d\n\n", datetime.getYear(), datetime.getMonth(), datetime.getDay());
      gfx->setTextColor( RGB565_BLUE, RGB565_WHITE);
      gfx->printf("   %02d:%02d:%02d", datetime.getHour(), datetime.getMinute(), datetime.getSecond());
  }
}
