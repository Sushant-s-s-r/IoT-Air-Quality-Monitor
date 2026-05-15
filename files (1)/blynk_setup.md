# Blynk IoT Setup Guide

## Step 1 — Create Account
Go to https://blynk.cloud and sign up for a free account.

## Step 2 — Create Template
1. Click **+ New Template**
2. Name: `Air Quality Monitor`
3. Hardware: `ESP32`
4. Connection: `WiFi`

## Step 3 — Add Datastreams
Go to **Datastreams** tab → **+ New Datastream → Virtual Pin**

| # | Virtual Pin | Name | Data Type | Min | Max | Unit |
|---|---|---|---|---|---|---|
| 1 | V0 | AQI | Integer | 0 | 500 | AQI |
| 2 | V1 | Temperature | Double | -10 | 60 | °C |
| 3 | V2 | Humidity | Double | 0 | 100 | % |
| 4 | V3 | Relay Status | Integer | 0 | 1 | — |
| 5 | V4 | AQI Label | String | — | — | — |
| 6 | V6 | Manual Override | Integer | 0 | 1 | — |

## Step 4 — Build Dashboard (Web)
Go to **Web Dashboard** → drag and drop:
- **Gauge** → V0 (AQI) — set range 0-500, color zones: green <100, yellow <200, red >200
- **Chart** → V1 (Temperature) + V2 (Humidity) — last 1 hour
- **LED** → V3 (Relay Status) — label ON/OFF
- **Button** → V6 (Manual Override) — switch mode

## Step 5 — Build Mobile Dashboard
In the Blynk app (iOS/Android):
- **Value Display** → V0, V1, V2
- **LED Widget** → V3
- **Button** → V6 (Hold/Switch)
- **SuperChart** → V0 time series

## Step 6 — Add Events (Notifications)
Go to **Events** tab → **+ New Event**:

| Event Code | Name | Type |
|---|---|---|
| `purifier_on` | Purifier Activated | Info |
| `purifier_off` | Air Quality Restored | Info |

Enable **Push Notifications** for both events in the mobile app.

## Step 7 — Create Device
1. Go to **Devices** → **+ New Device**
2. Select your template
3. Copy `BLYNK_TEMPLATE_ID` and `BLYNK_AUTH_TOKEN`
4. Paste them into the `.ino` sketch

## Troubleshooting
- **Device offline**: Check WiFi credentials, ensure Blynk library is ≥ 1.3.2
- **No sensor data**: Open Serial Monitor (115200 baud) to debug
- **OLED blank**: Check I2C address — try 0x3C or 0x3D
- **Relay not triggering**: Check GPIO 26 wiring and relay VCC (must be 5V)
