#include <Arduino.h>
#include <Wire.h>
#include "driver/i2s.h"

// --- Pin definiti per ESP32 Audio Kit V2.2 (controlla poi con il tuo schema) ---
#define I2S_MCLK_PIN    0     // Master Clock - FONDAMENTALE!
#define I2S_BCK_PIN     5
#define I2S_LRCK_PIN    25
#define I2S_DATA_OUT_PIN 26

#define I2C_SDA_PIN     18
#define I2C_SCL_PIN     23

// Indirizzo I2C del codec ES8388 (tipico)
#define ES8388_ADDR     0x10

// Parametri audio
#define SAMPLE_RATE     44100
#define TONE_FREQ       440.0f    // LA 440 Hz
#define NUM_SAMPLES     256       // campioni per buffer (piccolo buffer circolare)

// Buffer sinusoidale (stereo, 16 bit)
int16_t sineBuffer[NUM_SAMPLES * 2];  // 2 canali

// ----------------- Funzione per scrivere nei registri ES8388 via I2C -----------------
void es8388WriteReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ES8388_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

// ----------------- Inizializzazione ES8388 (configurazione completa) -----------------
void initES8388() {
  Serial.println("Inizializzazione ES8388...");
  
  // 1. Reset del chip
  es8388WriteReg(0x00, 0x80);
  delay(100);
  es8388WriteReg(0x00, 0x00);
  delay(50);

  // 2. Clock Control (reg 0x01-0x08)
  es8388WriteReg(0x01, 0x58);  // VMIDSEL=50K, enable analog
  es8388WriteReg(0x02, 0xF3);  // Power up DAC L&R, enable LDO
  es8388WriteReg(0x08, 0x00);  // Slave mode, MCLK from external source
  
  // 3. DAC Control (reg 0x17-0x1B)
  es8388WriteReg(0x17, 0x18);  // I2S 16-bit
  es8388WriteReg(0x18, 0x02);  // DAC normal polarity
  es8388WriteReg(0x19, 0x00);  // DAC unmute
  es8388WriteReg(0x1A, 0x00);  // DAC volume left = 0dB
  es8388WriteReg(0x1B, 0x00);  // DAC volume right = 0dB
  
  // 4. Output mixer control (reg 0x26-0x2B)
  es8388WriteReg(0x26, 0x00);  // Disable DAC to mono mixer
  es8388WriteReg(0x27, 0xB8);  // Enable DAC to LOUT mixer
  es8388WriteReg(0x28, 0x38);  // LOUT mixer gain
  es8388WriteReg(0x29, 0x38);  // ROUT mixer gain  
  es8388WriteReg(0x2A, 0xB8);  // Enable DAC to ROUT mixer
  
  // 5. Output volume control (reg 0x2E-0x30)
  es8388WriteReg(0x2E, 0x1C);  // LOUT1 volume (aumenta per volume maggiore, max ~0x1E)
  es8388WriteReg(0x2F, 0x1C);  // ROUT1 volume
  es8388WriteReg(0x30, 0x1C);  // LOUT2 volume
  es8388WriteReg(0x31, 0x1C);  // ROUT2 volume
  
  // 6. Power management outputs (reg 0x03-0x04)
  es8388WriteReg(0x03, 0x00);  // Power up left and right channel
  es8388WriteReg(0x04, 0x3C);  // Power up LOUT1/ROUT1 (headphone outputs)
  
  delay(50);
  Serial.println("ES8388 inizializzato.");
}

// ----------------- Configurazione I2S -----------------
void initI2S() {
  Serial.println("Inizializzazione I2S...");
  
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_I2S_MSB,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 4,
      .dma_buf_len = 256,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
      .mck_io_num = I2S_MCLK_PIN,      // MCLK - FONDAMENTALE!
      .bck_io_num = I2S_BCK_PIN,
      .ws_io_num = I2S_LRCK_PIN,
      .data_out_num = I2S_DATA_OUT_PIN,
      .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);
  
  Serial.println("I2S inizializzato.");
}

// ----------------- Creazione buffer sinusoidale -----------------
void generateSineBuffer() {
  float phaseIncrement = 2.0f * PI * TONE_FREQ / SAMPLE_RATE;
  float phase = 0.0f;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    float sample = sinf(phase);
    // Ampiezza moderata per evitare distorsione
    int16_t value = (int16_t)(sample * 16000);

    // Stereo: stesso valore su L e R
    sineBuffer[2 * i]     = value; // Left
    sineBuffer[2 * i + 1] = value; // Right

    phase += phaseIncrement;
    if (phase >= 2.0f * PI) {
      phase -= 2.0f * PI;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Avvio: onda sinusoidale su earphones...");

  // I2C per ES8388
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  initES8388();

  // I2S verso codec
  initI2S();

  // Prepara buffer sinusoidale
  generateSineBuffer();

  Serial.println("Setup completato. Dovresti sentire il tono nelle cuffie.");
}

void loop() {
  size_t bytesWritten;
  // Scrive il buffer continuamente
  i2s_write(I2S_NUM_0, (const char*)sineBuffer, sizeof(sineBuffer), &bytesWritten, portMAX_DELAY);
}
