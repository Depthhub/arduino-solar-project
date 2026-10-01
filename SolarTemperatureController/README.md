# Solar Temperature Controller

Arduino Uno firmware for solar-powered environmental monitoring with automatic fan cooling, SD card data logging, and real-time clock timestamps.

## Features

- Monitors ambient temperature and humidity (DHT11) every 2 seconds
- Multi-tier fan control with temperature warnings and optional motor protection
- Logs readings to MicroSD card in CSV format with DS3231 timestamps
- Continues operating when SD card or RTC is unavailable
- Serial Monitor output for live debugging

## Hardware Requirements

| Component | Purpose |
|-----------|---------|
| Arduino Uno R3 | Main controller |
| DHT11 | Temperature and humidity sensor |
| DS3231 RTC module | Accurate date/time with battery backup |
| MicroSD card module | CSV data logging |
| TIP122 transistor | Fan driver |
| 1N5819 diode | Flyback protection across fan |
| 1kΩ resistor | Base resistor for TIP122 |
| 12V DC fan | Cooling |
| Relay or transistor (optional) | Grinder motor stop at critical temperature |
| 12V solar panel + charge controller + battery | Power supply |
| 7805 voltage regulator | 5V for Arduino |

See [wiring.md](wiring.md) for full connection details.

## Required Libraries

Install via **Arduino IDE → Sketch → Include Library → Manage Libraries**:

1. **DHT sensor library** by Adafruit
2. **Adafruit Unified Sensor** by Adafruit
3. **RTClib** by Adafruit

Built-in libraries (no install needed): `SD`, `SPI`, `Wire`

## Installation and Upload

1. Clone or copy the `SolarTemperatureController` folder into your Arduino sketchbook directory.
2. Install the libraries listed above.
3. Open `SolarTemperatureController.ino` in Arduino IDE.
4. Select **Tools → Board → Arduino Uno**.
5. Select the correct **Tools → Port**.
6. Click **Upload**.

## Temperature Control Logic

| Temperature | Fan | Alert | Motor |
|-------------|-----|-------|-------|
| Below 30°C | OFF | NORMAL | Running |
| 30–40°C | ON | COOLING | Running |
| At/above 40°C | ON | HIGH TEMP WARNING | Running |
| At/above 50°C | ON | CRITICAL ALERT | Stopped (optional) |

A 28°C hysteresis band prevents rapid fan switching near the 30°C threshold.

## Configuration

Edit [config.h](config.h) to change settings:

```cpp
const float FAN_ON_TEMP = 30.0;          // Fan ON at/above this (°C)
const float FAN_OFF_TEMP = 28.0;         // Fan OFF at/below this (°C)
const float HIGH_TEMP_WARNING = 40.0;    // High temperature warning
const float CRITICAL_TEMP = 50.0;        // Critical alert threshold
const float MOTOR_RECOVERY_TEMP = 45.0;  // Resume motor after cooling
#define STOP_MOTOR_ON_CRITICAL 1         // Set to 0 to disable motor stop
const unsigned long SAMPLE_INTERVAL_MS = 2000;
const char LOG_FILENAME[] = "datalog.csv";
```

## RTC First-Time Setup

The DS3231 RTC must be set once after installation or when its CR2032 backup battery is replaced.

**Option A — RTClib example sketch:**
1. Open **File → Examples → RTClib → ds3231**.
2. Upload to set the current date/time.

**Option B — Compile-time sync:**
1. Set `#define SET_RTC_ON_COMPILE 1` in `config.h`.
2. Upload once (sets RTC from your computer's clock at compile time).
3. Set `SET_RTC_ON_COMPILE` back to `0` and upload again.

## Serial Monitor Output

Open Serial Monitor at **115200 baud**. Example output:

```
Date:
20/07/2026

Time:
14:25:16

Temperature:
30.5°C

Humidity:
66%

Fan:
ON

Alert:
HIGH TEMP WARNING

Motor:
RUNNING

-----------------------
```

## CSV Log Format

File: `datalog.csv` on the SD card root.

```
Date,Time,Temperature,Humidity,Fan,Alert
20-07-2026,10:00:00,28.4,67,OFF,NORMAL
20-07-2026,10:00:02,31.0,66,ON,COOLING
20-07-2026,10:00:04,42.1,65,ON,HIGH TEMP WARNING
20-07-2026,10:00:06,51.3,64,ON,CRITICAL ALERT
```

Remove the SD card and read it on a computer to view logged data.

## Error Handling

| Condition | System behavior |
|-----------|-----------------|
| DHT11 read failure | Warning on Serial; retries next cycle |
| RTC not detected | Logs `N/A` for date/time; monitoring continues |
| SD card missing | Serial error; monitoring and fan control continue |

## Project Structure

```
SolarTemperatureController/
├── SolarTemperatureController.ino   # Main sketch
├── config.h                         # Configuration constants
├── TemperatureSensor.h / .cpp       # DHT11 module
├── RTCManager.h / .cpp              # DS3231 module
├── SDLogger.h / .cpp                # SD card CSV logging
├── FanController.h / .cpp           # Fan hysteresis control
├── README.md
├── wiring.md
└── test_plan.md
```

## Testing

Follow the procedures in [test_plan.md](test_plan.md).

## License

Open source — use and modify freely for personal and educational projects.
