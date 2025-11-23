#ifndef HARDWARECONFIG_H
#define HARDWARECONFIG_H

// ---- Pines I2C ----
#define I2C_SDA 8
#define I2C_SCL 7

// ---- Pines I2S / Audio ----
#define I2S_MCK_PIN 19
#define I2S_BCK_PIN 20
#define I2S_LRCK_PIN 22
#define I2S_DOUT_PIN 23
#define I2S_DIN_PIN 21

// ---- Pantalla ----
#define LCD_SCK 1
#define LCD_DIN 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6
#define GFX_BL LCD_BL

// ---- Periféricos ----
#define MOTOR_VIBRATOR_PIN 18
#define BATTERY_ENABLE_PIN 15

struct ImpulseResult {
  String label;  
  float value;  
};

#endif|
