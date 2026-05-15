# 🌫️ IoT Air Quality Monitoring & Automated Purification System

> **ESP32 · Arduino IDE · Blynk IoT · MQ135 · DHT11 · SSD1306 OLED · Relay Module**

A real-time embedded IoT system that continuously monitors indoor air quality, temperature, and humidity — and automatically triggers a mist purification unit when pollution crosses a safe threshold. All live readings are streamed to a Blynk mobile dashboard over Wi-Fi.

---

## 📸 Features

- **Real-time air quality sensing** via MQ135 gas sensor (CO₂, NH₃, benzene, smoke)
- **Temperature & humidity monitoring** via DHT11
- **Live OLED display** showing AQI, temp, humidity, and purifier status
- **Threshold-based automation** — purifier relay triggers automatically when AQI > threshold
- **Manual override** from Blynk mobile app
- **Cloud telemetry** via Blynk IoT — live gauges, charts, and event logs
- **Blynk event notifications** when purifier activates/deactivates
- **Noise-reduced ADC readings** (10-sample averaging on MQ135)

---

## 🧰 Hardware Required

| Component | Specification | Qty |
|---|---|---|
| ESP32 Dev Board | WROOM-32 (38-pin) | 1 |
| MQ135 Gas Sensor | Analog output | 1 |
| DHT11 Sensor | Temperature & Humidity | 1 |
| OLED Display | SSD1306 128×64, I2C | 1 |
| Relay Module | 5V, 1-channel | 1 |
| Mist Purifier / Fan | 5V or 12V DC | 1 |
| LED (optional) | Status indicator | 1 |
| Resistors | 10kΩ (DHT pull-up) | 1 |
| Breadboard + Jumper Wires | — | — |
| Power Supply | 5V/2A USB or adapter | 1 |

---

## 🔌 Circuit / Pin Connections

```
ESP32 GPIO 34  ──────  MQ135 AO (Analog Out)
ESP32 GPIO 4   ──────  DHT11 DATA (with 10kΩ pull-up to 3.3V)
ESP32 GPIO 21  ──────  OLED SDA
ESP32 GPIO 22  ──────  OLED SCL
ESP32 GPIO 26  ──────  Relay IN
ESP32 GPIO 2   ──────  Status LED (optional)
ESP32 3.3V     ──────  MQ135 VCC, DHT11 VCC, OLED VCC
ESP32 GND      ──────  All GND lines
```

> ⚠️ **Note:** MQ135 requires a 5V supply for the heater — connect MQ135 VCC to 5V (VIN on ESP32 board from USB), but AO output is 0–5V. Use a voltage divider (10kΩ + 10kΩ) to bring it to 0–2.5V safe for ESP32 ADC. Or power MQ135 heater from 5V and take signal from AO directly into GPIO 34 (ESP32 ADC is 3.3V max — check your sensor's output level before connecting directly).

---

## 📦 Libraries Required

Install via **Arduino IDE → Sketch → Include Library → Manage Libraries**:

| Library | Version | Install Name |
|---|---|---|
| Blynk | ≥ 1.3.2 | `Blynk` |
| DHT sensor library | ≥ 1.4.4 | `DHT sensor library` by Adafruit |
| Adafruit GFX | ≥ 1.11.5 | `Adafruit GFX Library` |
| Adafruit SSD1306 | ≥ 2.5.7 | `Adafruit SSD1306` |

---

## ☁️ Blynk Setup

1. Sign up at [blynk.cloud](https://blynk.cloud)
2. Create a new **Template** → name it `Air Quality Monitor`
3. Add the following **Datastreams** (Virtual Pins):

| Virtual Pin | Name | Type | Range |
|---|---|---|---|
| V0 | AQI | Integer | 0–500 |
| V1 | Temperature | Double | 0–60 |
| V2 | Humidity | Double | 0–100 |
| V3 | Relay Status | Integer | 0–1 |
| V4 | AQI Label | String | — |
| V6 | Manual Override | Integer | 0–1 |

4. Create a **Device** from your template → copy `BLYNK_TEMPLATE_ID` and `BLYNK_AUTH_TOKEN`
5. Build a **Dashboard** with: Gauge (V0), Chart (V1, V2), LED (V3), Button (V6)

---

## ⚙️ Configuration

Open `src/air_quality_monitor.ino` and update:

```cpp
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

**Threshold tuning** (adjust to your environment):

```cpp
const int AQI_GOOD      = 100;   // Purifier turns OFF below this
const int AQI_UNHEALTHY = 300;   // Purifier turns ON above this
```

---

## 🚀 Upload Instructions

1. Install [Arduino IDE](https://www.arduino.cc/en/software) (v2.x recommended)
2. Add ESP32 board support:
   - Preferences → Additional Board URLs → paste:
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager → search `esp32` → Install
3. Select **Board:** `ESP32 Dev Module`
4. Select correct **Port** (COMx or /dev/ttyUSBx)
5. Upload speed: `115200`
6. Upload the sketch

---

## 📊 How It Works

```
MQ135 → ADC Read (10-sample avg) → Map to AQI (0-500)
DHT11 → Read Temp & Humidity

Every 3 seconds:
  ├── Update OLED display
  ├── Send to Blynk cloud (V0–V4)
  └── Check thresholds:
        AQI > 300 → Relay HIGH → Purifier ON
        AQI < 100 → Relay LOW  → Purifier OFF
        Manual override (V6) → bypasses auto logic
```

---

## 🧠 Embedded Concepts Applied

- **ADC conversion** — 12-bit ESP32 ADC, averaging for noise reduction
- **Sensor interfacing** — I2C (OLED), one-wire-like (DHT11), analog (MQ135)
- **Relay switching** — digital GPIO controlling mains/DC loads safely
- **Cloud communication** — Blynk IoT over TCP/Wi-Fi, virtual pin telemetry
- **Timer-based scheduling** — non-blocking `BlynkTimer` instead of `delay()`
- **Manual vs automatic control** — override logic with state preservation

---

## 📁 Project Structure

```
IoT-Air-Quality-Monitor/
├── src/
│   └── air_quality_monitor.ino   ← Main Arduino sketch
├── docs/
│   ├── circuit_diagram.md        ← Wiring notes
│   └── blynk_setup.md            ← Blynk dashboard guide
├── diagrams/
│   └── system_architecture.png   ← System block diagram
├── images/
│   └── (add your hardware photos here)
├── README.md
└── LICENSE
```

---

## 🔮 Future Improvements

- [ ] Add PM2.5 sensor (e.g. PMS5003) for particulate matter
- [ ] Store historical data to Firebase / InfluxDB
- [ ] Add buzzer alert for hazardous AQI
- [ ] OTA (Over-the-Air) firmware updates
- [ ] ML-based anomaly detection on AQI trends
- [ ] Add CO sensor (MQ7) for carbon monoxide monitoring

---

## 📄 License

MIT License — free to use and modify with attribution.

---

## 👤 Author

**Anubha Deepti Charchi**  
B.Tech CSE, KIIT University, Bhubaneswar  
[LinkedIn](https://linkedin.com/in/anubha-charchi-258085230) · [GitHub](https://github.com/Anubhacharchi)
