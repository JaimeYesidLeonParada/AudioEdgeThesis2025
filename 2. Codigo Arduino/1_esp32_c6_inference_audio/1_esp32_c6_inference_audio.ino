#include <Arduino.h>
#include "ESP_I2S.h"
#include "Wire.h"
#include "es8311.h"
#include <Baby-Crying-Detection_inferencing.h>

// ---- Pantalla con Arduino_GFX_Library ----
#include <Arduino_GFX_Library.h>

#define LCD_SCK 1
#define LCD_DIN 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6
#define GFX_BL LCD_BL

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0 /* rotation */, true /* IPS */,
  240 /* width */, 280 /* height */,
  0, 20, 0, 20);

// ---- Configuración del micrófono ES8311 ----
#define I2C_SDA 8
#define I2C_SCL 7

#define I2S_MCK_PIN 19
#define I2S_BCK_PIN 20
#define I2S_LRCK_PIN 22
#define I2S_DOUT_PIN 23
#define I2S_DIN_PIN 21

#define I2S_NUM I2S_NUM_0
#define SAMPLE_RATE 16000
#define MCLK_MULTIPLE 256
#define MCLK_FREQ_HZ (SAMPLE_RATE * MCLK_MULTIPLE)
#define SAMPLE_BITS I2S_DATA_BIT_WIDTH_16BIT
#define SLOT_MODE I2S_SLOT_MODE_MONO
#define SLOT_SELECT I2S_STD_SLOT_LEFT

I2SClass i2s;
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

// --- Inicialización del codec ---
esp_err_t es8311_codec_init() {
  es8311_handle_t es_handle = es8311_create(I2C_NUM_0, ES8311_ADDRRES_0);
  if (!es_handle) return ESP_FAIL;

  const es8311_clock_config_t clk_cfg = {
    .mclk_inverted = false,
    .sclk_inverted = false,
    .mclk_from_mclk_pin = true,
    .mclk_frequency = MCLK_FREQ_HZ,
    .sample_frequency = SAMPLE_RATE
  };

  ESP_ERROR_CHECK(es8311_init(es_handle, &clk_cfg, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));
  ESP_ERROR_CHECK(es8311_voice_volume_set(es_handle, 70, NULL));
  ESP_ERROR_CHECK(es8311_microphone_config(es_handle, false));

  return ESP_OK;
}

void setupI2S() {
  i2s.setPins(I2S_BCK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN, I2S_DIN_PIN, I2S_MCK_PIN);
  if (!i2s.begin(I2S_MODE_STD, SAMPLE_RATE, SAMPLE_BITS, SLOT_MODE, SLOT_SELECT)) {
    Serial.println("Error: I2S init failed");
    while (1)
      ;
  }
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

  Wire.begin(I2C_SDA, I2C_SCL);
  if (es8311_codec_init() != ESP_OK) {
    Serial.println("❌ Error inicializando codec ES8311");
    while (1)
      ;
  }

  setupI2S();
  setupInference(EI_CLASSIFIER_SLICE_SIZE);
  run_classifier_init();

  // ---- Inicializar pantalla ----
  if (!gfx->begin()) {
    Serial.println("❌ Error al iniciar pantalla");
    while (1)
      ;
  }
  gfx->fillScreen(RGB565_BLACK);
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

  gfx->setCursor(10, 10);
  gfx->setTextColor(RGB565_GREEN);
  gfx->setTextSize(2);
  gfx->println("Sistema listo...");
  delay(1000);
}

void loop() {
  i2s.readBytes((char *)audio_buffer, SAMPLE_BUFFER_SIZE * sizeof(int16_t));
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
      gfx->fillScreen(RGB565_BLACK);
      gfx->setCursor(10, 10);
      gfx->setTextColor(RGB565_CYAN);
      gfx->setTextSize(2);
      gfx->println("Predicciones:");

      int y = 40;
      for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        // Etiqueta
        gfx->setCursor(10, y);
        gfx->setTextColor(RGB565_WHITE);
        gfx->print(result.classification[ix].label);

        // Valor numérico (confidence)
        gfx->setCursor(160, y);
        gfx->setTextColor(RGB565_YELLOW);
        gfx->print(": ");
        gfx->print(result.classification[ix].value, 2);  // muestra por ejemplo 0.85

        // Barra de progreso
        int barWidth = result.classification[ix].value * 180;
        gfx->fillRect(10, y + 18, barWidth, 10, RGB565_GREEN);

        y += 40;
      }
      print_results = 0;
    }
  }
}
