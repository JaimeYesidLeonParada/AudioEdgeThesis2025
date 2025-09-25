#include <Arduino.h>
#include "ESP_I2S.h"
#include "Wire.h"
#include "es8311.h"
#include <Baby-Crying-Detection_inferencing.h>
#include <Arduino_GFX_Library.h>
#include "SensorPCF85063.hpp"
#include <U8g2lib.h>
#include "thingProperties.h"

// ---- Pantalla ----
#define LCD_SCK 1
#define LCD_DIN 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6
#define GFX_BL LCD_BL

// ------- WiFi ---------
const char* ssid     = "Familia Leon";
const char* password = "3186277230";
const char* deviceID = "fa7f8f34-db1a-453e-9dc9-1704d1404bb4";
const char* secretKey = "KthpL@KGJxYjuYO!PanhQm5?H";

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0, true, 240, 280,
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

const int MOTOR_VIBRATOR_PIN = 18;  // GPIO18 motor vibrador
const int BATTERY_ENABLE_PIN = 15;  // GPIO15 batería

// ---- RTC ----
SensorPCF85063 rtc;

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

// --- Inicialización RTC ---
void pcf85063_init(void) {
  if (!rtc.begin(Wire)) {
    while (1) {
      Serial.println("❌ No se detecta RTC PCF85063");
      delay(1000);
    }
  }
  RTC_DateTime datetime = rtc.getDateTime();
  if (datetime.getYear() < 2025) {
    rtc.setDateTime(2025, 9, 21, 17, 01, 0);
  }
}

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

// --- Inicialización codec ---
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

void onLEDStateChange()  {
  // Add your code here to act upon LEDState change
  //digitalWrite(LED_BUILTIN, lED_State);  // turn the LED on (HIGH is the voltage level)
}

void setupWiFi() {
  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("📣 Iniciando sistema...");

  Wire.begin(I2C_SDA, I2C_SCL);

  // Inicializa codec de audio
  if (es8311_codec_init() != ESP_OK) {
    Serial.println("❌ Error inicializando codec ES8311");
    while (1);
  }

  // Inicializa RTC
  pcf85063_init();

  setupI2S();
  setupInference(EI_CLASSIFIER_SLICE_SIZE);
  run_classifier_init();

  // ---- Pantalla ----
  if (!gfx->begin()) {
    Serial.println("❌ Error al iniciar pantalla");
    while (1);
  }
  gfx->fillScreen(RGB565_BLACK);
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

  gfx->setCursor(10, 10);
  gfx->setTextColor(RGB565_GREEN);
  gfx->setTextSize(2);
  gfx->println("Sistema listo...");
  delay(1000);

  // Pines Init
  pinMode(MOTOR_VIBRATOR_PIN, OUTPUT);
  pinMode(BATTERY_ENABLE_PIN, OUTPUT);
  digitalWrite(BATTERY_ENABLE_PIN, HIGH);
  digitalWrite(MOTOR_VIBRATOR_PIN, LOW);

  // Setup WiFI
  setupWiFi();

  // Defined in thingProperties.h
  initProperties();
  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();

  static uint32_t lastClockUpdate = 0;

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

      // ---- Reloj en pantalla ----
      // ---- Mostrar Hora Grande y Centrada ----
      if (millis() - lastClockUpdate > 1000) {
        lastClockUpdate = millis();
        RTC_DateTime datetime = rtc.getDateTime();

        // String de hora y minutos
        char horaStr[10];
        sprintf(horaStr, "%02d:%02d", datetime.getHour(), datetime.getMinute());

        // Fuente grande para números
        gfx->setFont(u8g2_font_fub30_tn);                  // Bold 30px, solo números
        uint16_t relojVerde = gfx->color565(0, 255, 179);  // Verde neón
        gfx->setTextColor(relojVerde, RGB565_BLACK);

        // Calcular ancho para centrar
        int16_t x1, y1;
        uint16_t w, h;
        gfx->getTextBounds(horaStr, 0, 0, &x1, &y1, &w, &h);

        int16_t centerX = ((gfx->width() - w) / 2) - 10;
        int16_t posY = 85;  // Altura de la hora en la pantalla

        // Limpiar franja superior
        gfx->fillRect(0, 0, gfx->width(), 70, RGB565_BLACK);

        // Dibujar hora centrada
        gfx->setCursor(centerX, posY);
        gfx->println(horaStr);
      }
      
      // ---- Restaurar fuente por defecto ----
      gfx->setFont((const GFXfont *)NULL); 
      gfx->setTextSize(2); // tamaño normal
      gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);

      // ---- Predicciones ----
      int y = 105;

       // print the predictions on console
      ei_printf("Predictions ");
      ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
          result.timing.dsp, result.timing.classification, result.timing.anomaly);
      ei_printf(": \n");

      for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        gfx->setCursor(10, y);
        gfx->setTextColor(RGB565_WHITE);
        gfx->print(result.classification[ix].label);

        gfx->setCursor(160, y);
        gfx->setTextColor(RGB565_YELLOW);
        gfx->print(result.classification[ix].value, 2);

        int barWidth = result.classification[ix].value * 180;
        gfx->fillRect(10, y + 18, barWidth, 10, RGB565_GREEN);

        y += 40;

        // Imprime el resultado de cada clase
        ei_printf("    %s: ", result.classification[ix].label);
        ei_printf_float(result.classification[ix].value);
        ei_printf("\n");

        if (result.classification[ix].label == "baby-crying") {
          babyCryingDetection = result.classification[ix].value;
        }

        if (result.classification[ix].label == "ambulance") {
          ambulance = result.classification[ix].value;
        }

        if (result.classification[ix].label == "fire") {
          fireAlarm = result.classification[ix].value;
        }

        if (result.classification[ix].label == "noise") {
          noise = result.classification[ix].value;
        }
      }

      print_results = 0;
    }
  }
}
