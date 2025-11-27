#include <Arduino.h>
#include "es8311.h"
#include <Baby-Crying-Detection_inferencing.h>
#include <Arduino_GFX_Library.h>
#include <U8g2lib.h>
#include "ClockDisplay.h"
#include "CloudManager.h"
#include "ResultsDisplay.h"
#include "SystemManager.h"
#include "HardwareConfig.h"
#include "AudioManager.h"
#include "ESPNOWManager.h"
#include "WiFi.h"

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0, true, 240, 280,
  0, 20, 0, 20);

ClockDisplay clockDisplay(gfx);
CloudManager cloudManager;
ResultsDisplay resultsDisplay(gfx);
SystemManager systemManager(gfx);
AudioManager audioManager;
PostProcessor *pp;

#define SAMPLE_BUFFER_SIZE 2048
int16_t audio_buffer[SAMPLE_BUFFER_SIZE];

typedef struct {
  signed short *buffers[2];
  unsigned char buf_select;
  unsigned char buf_ready;
  unsigned int buf_count;
  unsigned int n_samples;
} inference_t;

static inference_t inference;
static bool debug_nn = false;
static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);

// --- Alimentador del buffer para inferencia ---
void audio_inference_callback(uint32_t n_bytes) {
  for (int i = 0; i < n_bytes / 2; i++) {
    inference.buffers[inference.buf_select][inference.buf_count++] = audio_buffer[i] * 8;
    if (inference.buf_count >= inference.n_samples) {
      inference.buf_select ^= 1;
      inference.buf_count = 0;
      inference.buf_ready = 1;
    }
  }
}

// --- Proveedor de datos para el modelo ---
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
  numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);
  return 0;
}

void setupInference(uint32_t n_samples) {
  inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));
  inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));
  inference.buf_select = 0;
  inference.buf_count = 0;
  inference.buf_ready = 0;
  inference.n_samples = n_samples;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("📣 Iniciando sistema...");

  systemManager.begin();
  if (!audioManager.begin()) {
    while (1);
  }

  setupInference(EI_CLASSIFIER_SLICE_SIZE);
  run_classifier_init();
  cloudManager.begin();
  clockDisplay.begin();
  resultsDisplay.begin();
  setupPostProcessor();
}

void loop() {
  cloudManager.update();
  showTime();
  loopInference();
  sendDataESPNOW();
  checkESPNOWMessages();
}

void checkESPNOWMessages(){
  String msg = checkMessagesReceived();

  if (msg.length() == 0) {
    // No hay mensaje nuevo, salir sin dibujar
    return;
  }
  // Limpiar área bajo el reloj
  gfx->fillRect(0, 85, gfx->width(), gfx->height(), RGB565_BLACK);
  gfx->setFont(u8g2_font_fub11_tf); 
  gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);

  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);

  int16_t centerX = 0;   // ajusta según tu pantalla
  int16_t centerY = 140; // ajusta según tu pantalla

  gfx->setCursor(centerX, centerY);
  gfx->println(msg);
  delay(3000);
}

void sendDataESPNOW() {
  static bool configured = false;

  if (WiFi.status() == WL_CONNECTED ) {
    if (configured == false) {
      setupESPNOW();
      clockDisplay.refreshTime();
      configured = true;
      return;
    }
  } else {
    return;
  }

  static unsigned long last_debug = 0;
  if (millis() - last_debug > 10000) {
    last_debug = millis();
    checkMasterESPNOW();
  }

  static uint32_t lastESPNOWUpdate = 0;
  if (millis() - lastESPNOWUpdate > 40000) {
    sendMessageESPNOW();
    
    lastESPNOWUpdate = millis();
  }
}

void showTime() {
  static uint32_t lastClockUpdate = 0;
  if (millis() - lastClockUpdate > 60000 || lastClockUpdate == 0) {
    clockDisplay.update();
    lastClockUpdate = millis();
  }
}

void loopInference() {
  audioManager.readAudio(audio_buffer, SAMPLE_BUFFER_SIZE);
  audio_inference_callback(SAMPLE_BUFFER_SIZE * sizeof(int16_t));

  if (inference.buf_ready == 1) {
    inference.buf_ready = 0;

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
    signal.get_data = &microphone_audio_signal_get_data;

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);

    if (r != EI_IMPULSE_OK) {
      Serial.print("❌ Fallo en inferencia: ");
      Serial.println((int)r);
      return;
    }

    if (++print_results >= EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW) {     
      std::vector<ImpulseResult> results;

       for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        String label = result.classification[ix].label;
        float value = result.classification[ix].value;

        pp->addSample(label, value);

        ImpulseResult ir;
        ir.label = label;
        ir.value = value;
        results.push_back(ir);
      }

      AlertLevel lvl = resultsDisplay.showResults(results);
      systemManager.vibrate(lvl);
      cloudManager.showResults(results);

      print_results = 0;
    }
  }
}

void setupPostProcessor() {
  String labels[EI_CLASSIFIER_LABEL_COUNT];
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; ++i) {
    labels[i] = String(ei_classifier_inferencing_categories[i]);
  }

  // Ventana de promedio para mostrar la alerta
  const int windowSize = 4;

  // Construir el PostProcessor con el array de String y la cuenta de etiquetas
  pp = new PostProcessor(labels, EI_CLASSIFIER_LABEL_COUNT, windowSize);
  resultsDisplay.setPostProcessor(pp);
}
