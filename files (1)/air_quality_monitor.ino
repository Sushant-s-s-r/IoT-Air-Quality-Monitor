/*
  ============================================================
  IoT Air Quality Monitoring & Automated Purification System
  ============================================================
  Author  : Anubha Deepti Charchi
  Board   : ESP32 (WROOM-32)
  Sensors : MQ135 (Air Quality), DHT11 (Temp & Humidity)
  Display : SSD1306 OLED (128x64, I2C)
  Cloud   : Blynk IoT
  ============================================================

  Pin Configuration:
    MQ135  AO --> GPIO 34 (ADC1_CH6)
    DHT11  DATA --> GPIO 4
    OLED   SDA --> GPIO 21
    OLED   SCL --> GPIO 22
    RELAY  IN  --> GPIO 26
    LED    (optional status) --> GPIO 2
*/

// ─── Libraries ───────────────────────────────────────────
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Air Quality Monitor"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ─── WiFi Credentials ────────────────────────────────────
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ─── Pin Definitions ─────────────────────────────────────
#define MQ135_PIN    34
#define DHT_PIN       4
#define DHT_TYPE    DHT11
#define RELAY_PIN    26
#define STATUS_LED    2

// ─── OLED Setup ──────────────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_ADDR    0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ─── Sensor Objects ──────────────────────────────────────
DHT dht(DHT_PIN, DHT_TYPE);

// ─── Thresholds ──────────────────────────────────────────
const int   AQI_GOOD       = 100;   // Raw ADC — below this: air is clean
const int   AQI_MODERATE   = 200;   // Below this: moderate pollution
const int   AQI_UNHEALTHY  = 300;   // Above this: trigger purification
const float TEMP_HIGH      = 35.0;  // °C — overheat alert threshold
const float HUMIDITY_HIGH  = 80.0;  // % — high humidity alert

// ─── Blynk Virtual Pins ──────────────────────────────────
// V0 → AQI (raw ADC value mapped 0-100)
// V1 → Temperature
// V2 → Humidity
// V3 → Relay Status (0/1)
// V4 → Air Quality Label (string)
// V5 → Notification LED widget

// ─── Globals ─────────────────────────────────────────────
int   aqiRaw        = 0;
float temperature   = 0.0;
float humidity      = 0.0;
bool  relayActive   = false;
bool  manualOverride = false;       // Blynk button can force relay state

BlynkTimer timer;

// ─── Manual Relay Override from Blynk App ────────────────
BLYNK_WRITE(V6) {
  int val = param.asInt();
  manualOverride = (val == 1);
  if (manualOverride) {
    digitalWrite(RELAY_PIN, HIGH);
    relayActive = true;
  }
}

// ─── Read MQ135 (ADC averaging for noise reduction) ──────
int readMQ135() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(MQ135_PIN);
    delay(5);
  }
  return (int)(sum / 10);
}

// ─── Map raw ADC to AQI-like 0-500 scale ─────────────────
int mapToAQI(int rawADC) {
  // ESP32 ADC: 0-4095 (12-bit). MQ135 increases with pollution.
  return map(rawADC, 0, 4095, 0, 500);
}

// ─── Air Quality Label ───────────────────────────────────
String getAQILabel(int aqi) {
  if (aqi < 50)  return "Good";
  if (aqi < 100) return "Moderate";
  if (aqi < 150) return "Unhealthy (Sensitive)";
  if (aqi < 200) return "Unhealthy";
  if (aqi < 300) return "Very Unhealthy";
  return "Hazardous";
}

// ─── Relay / Purifier Control ────────────────────────────
void controlPurifier(int aqi) {
  if (manualOverride) return;  // Manual control takes priority

  if (aqi > AQI_UNHEALTHY && !relayActive) {
    digitalWrite(RELAY_PIN, HIGH);
    relayActive = true;
    Serial.println("[RELAY] Purifier ON — AQI exceeded threshold");
    Blynk.logEvent("purifier_on", String("AQI: ") + aqi + " — Purifier activated");
  } else if (aqi <= AQI_GOOD && relayActive) {
    digitalWrite(RELAY_PIN, LOW);
    relayActive = false;
    Serial.println("[RELAY] Purifier OFF — Air quality restored");
    Blynk.logEvent("purifier_off", "Air quality returned to safe levels");
  }
}

// ─── Update OLED Display ─────────────────────────────────
void updateDisplay(int aqi, float temp, float hum, bool relayOn) {
  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("  AIR QUALITY MONITOR");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // AQI
  display.setCursor(0, 13);
  display.print("AQI : ");
  display.setTextSize(2);
  display.setCursor(38, 11);
  display.print(aqi);
  display.setTextSize(1);

  // Label
  display.setCursor(0, 29);
  display.print(getAQILabel(aqi).substring(0, 20));

  // Temp & Humidity
  display.setCursor(0, 40);
  display.print("T:");
  display.print(temp, 1);
  display.print((char)247);  // degree symbol
  display.print("C  H:");
  display.print(hum, 0);
  display.print("%");

  // Relay status
  display.setCursor(0, 52);
  display.print("Purifier: ");
  display.print(relayOn ? "ON " : "OFF");

  // WiFi/Blynk indicator
  display.setCursor(100, 52);
  display.print(Blynk.connected() ? "  WF" : "  --");

  display.display();
}

// ─── Send Data to Blynk ──────────────────────────────────
void sendToBlynk(int aqi, float temp, float hum, bool relayOn) {
  Blynk.virtualWrite(V0, aqi);
  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, hum);
  Blynk.virtualWrite(V3, relayOn ? 1 : 0);
  Blynk.virtualWrite(V4, getAQILabel(aqi));
}

// ─── Main Sensor Read + Control Loop ─────────────────────
void readAndControl() {
  // Read sensors
  aqiRaw    = readMQ135();
  int aqi   = mapToAQI(aqiRaw);
  temperature = dht.readTemperature();
  humidity    = dht.readHumidity();

  // Validate DHT readings
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("[DHT11] Read failed — retrying");
    return;
  }

  // Serial debug
  Serial.printf("[SENSOR] AQI: %d | Temp: %.1f°C | Hum: %.0f%% | Relay: %s\n",
    aqi, temperature, humidity, relayActive ? "ON" : "OFF");

  // Control logic
  controlPurifier(aqi);

  // Output
  updateDisplay(aqi, temperature, humidity, relayActive);
  sendToBlynk(aqi, temperature, humidity, relayActive);

  // Status LED blink if unhealthy
  if (aqi > AQI_UNHEALTHY) {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
  } else {
    digitalWrite(STATUS_LED, LOW);
  }
}

// ─── Setup ───────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== IoT Air Quality Monitor Boot ===");

  // Pin modes
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(STATUS_LED, LOW);

  // DHT init
  dht.begin();

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] SSD1306 not found — check wiring");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 25);
  display.println("Connecting WiFi...");
  display.display();

  // Blynk + WiFi connect
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println("[WiFi] Connected: " + WiFi.localIP().toString());

  // OLED boot message
  display.clearDisplay();
  display.setCursor(10, 20);
  display.println("System Ready!");
  display.setCursor(10, 35);
  display.println(WiFi.localIP().toString());
  display.display();
  delay(2000);

  // Timer: read every 3 seconds
  timer.setInterval(3000L, readAndControl);

  Serial.println("[SYSTEM] Setup complete — monitoring started");
}

// ─── Loop ────────────────────────────────────────────────
void loop() {
  Blynk.run();
  timer.run();
}
