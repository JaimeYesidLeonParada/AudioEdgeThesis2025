#include "CloudManager.h"
#include <ArduinoIoTCloud.h> 
#include "thingProperties.h"

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

void CloudManager::showResults(const std::vector<ImpulseResult> &results) {
   for (const auto &res : results) {
    if (res.label == "baby-crying") {
      babyCryingDetection = res.value;
    }

    if (res.label == "ambulance") {
      ambulance = res.value;
    }

    if (res.label == "fire") {
      fireAlarm = res.value;
    }

    if (res.label == "noise") {
      noise = res.value;
    }
  }
}

void onLEDStateChange()  {
  // Add your code here to act upon LEDState change
  //digitalWrite(LED_BUILTIN, lED_State);  // turn the LED on (HIGH is the voltage level)
}
