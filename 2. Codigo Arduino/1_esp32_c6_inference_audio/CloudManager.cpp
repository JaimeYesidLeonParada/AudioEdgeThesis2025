#include "CloudManager.h"
#include <ArduinoIoTCloud.h> 

extern void initProperties(); 
extern WiFiConnectionHandler ArduinoIoTPreferredConnection; 

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
