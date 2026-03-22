# ESP32-A1S Audio Kit V2.2 - Guida Configurazione

Progetto di test audio per **ESP32-A1S Audio Kit V2.2** con codec ES8388.

## 📋 Indice

- [Hardware](#hardware)
- [Configurazione Iniziale](#configurazione-iniziale)
- [Setup Software](#setup-software)
- [Pin Configuration](#pin-configuration)
- [Test Audio](#test-audio)
- [Troubleshooting](#troubleshooting)

---

## 🔧 Hardware

### Specifiche Scheda

- **MCU**: ESP32-WROVER-B
- **Codec Audio**: ES8388
- **Interfaccia Audio**: I2S
- **Interfaccia Controllo**: I2C
- **Outputs**:
  - Jack cuffie stereo 3.5mm
  - Speaker amplificati (opzionale)
- **Inputs**:
  - Microfono built-in
  - Line-in (opzionale)

### Componenti sulla Scheda

- 6 pulsanti programmabili
- 5 DIP switch di configurazione
- LED di stato
- Jack per cuffie
- Connettori per speaker esterni

---

## ⚙️ Configurazione Iniziale

### 🎚️ STEP 1: Switch DIP (CRITICO!)

La scheda ha **5 switch DIP** che DEVONO essere configurati correttamente:

```
Switch da ATTIVARE (su ON):
✅ KEY3   → ON
✅ DATA3  → ON
✅ CMD    → ON
❌ MTCK   → OFF (JTAG debug - non necessario)
❌ MTDO   → OFF (JTAG debug - non necessario)
```

⚠️ **IMPORTANTE**: Senza questi switch attivi, l'I2C non funzionerà e il codec ES8388 non sarà rilevato!

### 🔌 STEP 2: Alimentazione

1. Collega la scheda via **USB** al computer
2. Verifica che il **LED di alimentazione** sia acceso
3. Non serve alimentazione esterna per le cuffie

### 🎧 STEP 3: Audio Output

Collega le **cuffie al jack 3.5mm** della scheda (non servono speaker esterni per i test).

---

## 💻 Setup Software

### Requisiti

- **PlatformIO** (VS Code o CLI)
- **Driver USB-Serial** (CP2102 o CH340, di solito già inclusi)

### Compilazione e Upload

```bash
# Compila il progetto
pio run -e esp32-audio-kit

# Upload su ESP32
pio run -t upload -e esp32-audio-kit

# Upload + Monitor seriale
pio run -t upload -t monitor -e esp32-audio-kit
```

### Dipendenze

Il progetto usa solo librerie standard:

- `Arduino.h` - Framework Arduino per ESP32
- `Wire.h` - Comunicazione I2C
- `driver/i2s.h` - Driver I2S hardware ESP32

---

## 📌 Pin Configuration

### Pin I2S (Audio Data)

```cpp
I2S_MCLK_PIN    = 0   // Master Clock
I2S_BCK_PIN     = 27  // Bit Clock
I2S_LRCK_PIN    = 25  // Word Select (Left/Right)
I2S_DATA_OUT_PIN = 26 // Data Output
```

### Pin I2C (Controllo Codec)

```cpp
I2C_SDA_PIN = 33  // Data (VERIFICATA sulla tua scheda!)
I2C_SCL_PIN = 32  // Clock (VERIFICATA sulla tua scheda!)
ES8388_ADDR = 0x10 // Indirizzo I2C del codec
```

⚠️ **NOTA**: Alcuni modelli Audio Kit usano SDA=18/SCL=23. Il firmware include una **scansione automatica** che rileva la configurazione corretta.

### Pin Controllo

```cpp
PA_ENABLE_PIN = 21  // Power Amplifier Enable
```

---

## 🎵 Test Audio

### Cosa Fa il Firmware

Il programma attuale genera un **tono sinusoidale a 440Hz** (nota La) e lo trasmette continuamente alle cuffie.

### Output Seriale Atteso

```
========================================
Avvio: Test tono 440Hz su ESP32 Audio Kit
========================================
PA Enable pin 21 attivato (HIGH)
Pin enable aggiuntivi (19, 22) attivati

=== RICERCA AUTOMATICA ES8388 ===

Provo Config 1 (standard): SDA=18, SCL=23
Scansione bus I2C...
  NESSUN dispositivo I2C trovato!

Provo Config 2 (alternativa): SDA=33, SCL=32
Scansione bus I2C...
  *** Dispositivo trovato a indirizzo: 0x10 ***
  Trovati 1 dispositivi I2C

*** CONFIGURAZIONE FUNZIONANTE TROVATA! ***
*** Usa: SDA=33, SCL=32 ***

2. Inizializzazione codec ES8388...
ES8388 Reg 0x00 = 0x05
Verifica configurazione ES8388:
  Reg 0x02 (Power DAC): 0x00
  Reg 0x04 (Power Out): 0x00
  Reg 0x17 (DAC Ctrl): 0x18
  Reg 0x19 (DAC Mute): 0x00
  Reg 0x2E (Vol Left): 0x1E
ES8388 inizializzato con successo.

3. Inizializzazione I2S...
I2S inizializzato con successo.

4. Generazione buffer audio...
Generazione buffer sinusoidale 440Hz...
Buffer generato: 256 samples, amplitude: 20000

========================================
Setup completato!
DOVRESTI SENTIRE un tono 440Hz nelle cuffie.
========================================

Audio streaming: 1024 bytes scritti
Audio streaming: 1024 bytes scritti
...
```

### Cosa Aspettarsi

✅ **Tono continuo** a 440Hz (La) nelle cuffie
✅ **Volume moderato** (non troppo forte, non troppo basso)
✅ **Nessun rumore/distorsione**

---

## 🔍 Troubleshooting

### Problema: "NESSUN dispositivo I2C trovato"

**Causa**: Switch DIP non configurati correttamente

**Soluzione**:

1. Verifica che KEY3, DATA3, CMD siano su **ON**
2. Premi il pulsante **RESET** sulla scheda
3. Ricarica il firmware

---

### Problema: "I2C funziona ma non sento audio"

**Soluzioni da provare**:

1. **Verifica jack corretto**Alcune schede hanno 2 jack (uno per cuffie, uno per speaker)
2. **Prova altre cuffie**Alcune cuffie ad alta impedenza potrebbero non funzionare
3. **Verifica volume**Se le cuffie hanno controllo volume, alzalo
4. **Tocca il jack**
   Dovresti sentire un fruscio (conferma che l'output funziona)

---

### Problema: Audio distorto o con click

**Soluzioni**:

1. **Riduci ampiezza** nel codice:

   ```cpp
   // In generateSineBuffer()
   int16_t value = (int16_t)(sample * 12000); // Invece di 20000
   ```
2. **Verifica alimentazione**
   Prova una porta USB diversa (USB 3.0 fornisce più corrente)

---

### Problema: Errore upload firmware

**Soluzioni**:

1. **Tieni premuto BOOT**Mentre carichi il firmware, tieni premuto il pulsante BOOT
2. **Verifica driver USB**Assicurati che i driver CP2102/CH340 siano installati
3. **Cambia porta USB**
   Prova una porta USB diversa

---

## 📚 Riferimenti

### Datasheet

- [ESP32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)
- [ES8388 Codec Datasheet](https://www.everest-semi.com/pdf/ES8388%20DS.pdf)
- [I2S Protocol](https://en.wikipedia.org/wiki/I%C2%B2S)

### Link Utili

- [ESP32 Audio Kit Schematic](https://docs.ai-thinker.com/en/esp32-audio-kit)
- [PlatformIO ESP32 Platform](https://docs.platformio.org/en/latest/platforms/espressif32.html)

---

## 📝 Note di Sviluppo

### Parametri Audio Correnti

```cpp
Sample Rate: 44100 Hz
Bit Depth: 16-bit
Channels: Stereo (L+R)
MCLK: 11.2896 MHz (256 × Sample Rate)
Buffer Size: 256 samples
Tone Frequency: 440 Hz (La/A4)
Amplitude: 20000 (su max 32767)
```

### Prossimi Step Suggeriti

- [ ] Aggiungere controllo volume via pulsanti
- [ ] Implementare riproduzione file WAV da SD card
- [ ] Aggiungere ingresso microfono
- [ ] Implementare filtri audio (EQ, reverb)
- [ ] Bluetooth audio streaming

---

## 📄 Licenza

Questo progetto è fornito come esempio educativo. Usa a tuo rischio e pericolo.

---

## 👤 Autore

Creato per testing ESP32-A1S Audio Kit V2.2

---

**Data ultima modifica**: Marzo 2026
