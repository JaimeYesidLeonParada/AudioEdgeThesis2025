#include <Arduino.h>
#include "ESP_I2S.h"
#include "Wire.h"
#include "es8311.h"
#include <Baby-Crying-Detection_inferencing.h>  // Tu modelo

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
    while (1);
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
  delay(500);  // Espera por si el monitor serial tarda
  Serial.println("📣 Iniciando sistema...");

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("✅ Wire iniciado");

  if (es8311_codec_init() != ESP_OK) {
    Serial.println("❌ Error inicializando codec ES8311");
    while (1);
  } else {
    Serial.println("✅ Codec ES8311 inicializado");
  }

  setupI2S();
  Serial.println("✅ I2S iniciado");

  setupInference(EI_CLASSIFIER_SLICE_SIZE);
  Serial.println("✅ Buffers de inferencia listos");

  run_classifier_init();
  Serial.println("✅ Clasificador Edge Impulse inicializado");

  Serial.println("🚀 Sistema listo para inferencia continua");
}


void loop() {
  //Serial.println("⏺️ Leyendo audio...");
  i2s.readBytes((char *)audio_buffer, SAMPLE_BUFFER_SIZE * sizeof(int16_t));
  //Serial.println("🎧 Audio leído");

  audio_inference_callback(SAMPLE_BUFFER_SIZE * sizeof(int16_t));
  //Serial.println("📥 Audio enviado al buffer de inferencia");

  if (inference.buf_ready == 1) {
    //Serial.println("⚡ Buffer listo, ejecutando inferencia...");
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
      Serial.println("✅ Predicciones:");
      for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        Serial.print("  ");
        Serial.print(result.classification[ix].label);
        Serial.print(": ");
        Serial.println(result.classification[ix].value, 4);
      }
      print_results = 0;
    }
  }
}

