#include <Wire.h>
#include "HWCDC.h"

HWCDC USBSerial;

#define I2C_SDA 4
#define I2C_SCL 3

void setup() {
  USBSerial.begin(115200);
  delay(1000);
  USBSerial.println("🔍 Escaneando dispositivos I2C...");

  Wire.begin(I2C_SDA, I2C_SCL);

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      USBSerial.print("✅ Dispositivo I2C encontrado en dirección 0x");
      if (address < 16) USBSerial.print("0");
      USBSerial.println(address, HEX);
    }
  }

  USBSerial.println("🔎 Escaneo I2C terminado.");
}

void loop() {
}
