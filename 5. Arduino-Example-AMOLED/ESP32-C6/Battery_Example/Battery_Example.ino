#include <Arduino_GFX_Library.h>

#define BAT_ADC_PIN 0
#define BAT_EN_PIN 15

#define LCD_SCK   1
#define LCD_DIN   2
#define LCD_CS    5
#define LCD_DC    3
#define LCD_RST   4
#define LCD_BL    6

#define GFX_BL LCD_BL

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0 /* rotation */, true /* IPS */,
  240 /* width */, 280 /* height */,
  0 /* col offset 1 */, 20 /* row offset 1 */,
  0 /* col offset 2 */, 20 /* row offset 2 */);

void setup(void)
{
  pinMode(BAT_EN_PIN, OUTPUT);
  digitalWrite(BAT_EN_PIN, HIGH);

#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX Hello World example");

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_WHITE);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  
  gfx->setTextSize(2);
  analogReadResolution(12);
}

void loop()
{
  gfx->setCursor(0, 50);
  // read the analog / millivolts value for pin 2:
  int analogValue = analogRead(BAT_ADC_PIN);
  int analogVolts = analogReadMilliVolts(BAT_ADC_PIN);
  
  // print out the values you read:
  gfx->setTextColor(RGB565_RED, RGB565_WHITE);
  gfx->printf(" ADC value: %4d\n\n", analogValue);
  gfx->setTextColor(RGB565_BLUE, RGB565_WHITE);
  gfx->printf(" Voltage: %4d mV\n", analogVolts * 3);

  delay(100);  // delay in between reads for clear read from serial
}
