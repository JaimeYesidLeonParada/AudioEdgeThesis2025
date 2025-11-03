#include "CloudManager.h"
//#include "thingProperties.h"   // define initProperties() y ArduinoIoTPreferredConnection
#include <ArduinoIoTCloud.h>   // opcional, por claridad (no modificar otras cosas)

// Declaraciones externas: indicamos que existen en otro módulo (.ino)
extern void initProperties();  // función definida en thingProperties.h (incluida en el .ino)
extern WiFiConnectionHandler ArduinoIoTPreferredConnection; // variable definida en thingProperties.h

void CloudManager::begin() {
  // Registrar propiedades y callbacks definidos en thingProperties.h
  initProperties();

  // Conectar a Arduino IoT Cloud usando la conexión preferida
  // (igual que tenías en setup())
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Nivel de logs y debug info (igual que antes)
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void CloudManager::update() {
  // Llamada periódica para mantener la conexión y callbacks
  ArduinoCloud.update();
}
