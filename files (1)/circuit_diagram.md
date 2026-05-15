# Circuit Diagram — Wiring Notes

## ESP32 to Component Connections

### MQ135 Gas Sensor
```
MQ135 VCC  → ESP32 VIN (5V from USB)
MQ135 GND  → ESP32 GND
MQ135 AO   → Voltage Divider → ESP32 GPIO 34
MQ135 DO   → (Not used — we use analog output)
```
**Voltage Divider for AO (5V → ~2.5V):**
```
MQ135 AO ─── 10kΩ ─── GPIO 34
                   │
                 10kΩ
                   │
                 GND
```

### DHT11 Temperature & Humidity
```
DHT11 VCC  → ESP32 3.3V
DHT11 GND  → ESP32 GND
DHT11 DATA → ESP32 GPIO 4

Pull-up resistor:
GPIO 4 ─── 10kΩ ─── 3.3V
```

### SSD1306 OLED (I2C)
```
OLED VCC → ESP32 3.3V
OLED GND → ESP32 GND
OLED SDA → ESP32 GPIO 21
OLED SCL → ESP32 GPIO 22
```

### Relay Module
```
Relay VCC → ESP32 VIN (5V)
Relay GND → ESP32 GND
Relay IN  → ESP32 GPIO 26

Relay COM + NO → Mist purifier power circuit
```

### Status LED (optional)
```
LED Anode (long leg) → 220Ω resistor → ESP32 GPIO 2
LED Cathode           → GND
```

## Power Notes
- ESP32 powered via USB (5V)
- VIN pin provides 5V for relay and MQ135 heater
- 3.3V pin for OLED and DHT11
- Do NOT power relay coil from 3.3V — use 5V VIN

## Safety
- Always use a flyback diode across relay coil if driving inductive loads
- Ensure mist purifier is rated for the relay's switching capacity
- Keep mains wiring well away from the ESP32 if using AC-powered purifier
