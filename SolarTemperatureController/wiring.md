# Wiring Guide — Solar Temperature Controller

## Pin Summary

| Component | Signal | Arduino Pin |
|-----------|--------|-------------|
| DHT11 | DATA | D2 |
| DHT11 | VCC | 5V |
| DHT11 | GND | GND |
| SD Module | CS | D10 |
| SD Module | MOSI | D11 |
| SD Module | MISO | D12 |
| SD Module | SCK | D13 |
| SD Module | VCC | 5V |
| SD Module | GND | GND |
| DS3231 RTC | SDA | A4 |
| DS3231 RTC | SCL | A5 |
| DS3231 RTC | VCC | 5V |
| DS3231 RTC | GND | GND |
| Fan driver | Base (via 1kΩ) | D8 |
| Motor relay (optional) | Signal | D9 |

---

## DHT11 Temperature/Humidity Sensor

```
DHT11          Arduino Uno
─────          ───────────
VCC    ─────── 5V
GND    ─────── GND
DATA   ─────── D2
```

Some DHT11 modules include a built-in pull-up resistor on the data line. If using a bare sensor, add a 10kΩ resistor between DATA and VCC.

---

## MicroSD Card Module (SPI)

```
SD Module      Arduino Uno
─────────      ───────────
VCC    ─────── 5V
GND    ─────── GND
CS     ─────── D10
MOSI   ─────── D11
MISO   ─────── D12
SCK    ─────── D13
```

Use a 3.3V or 5V-compatible SD module. Most breakout boards include level shifting.

---

## DS3231 Real Time Clock (I2C)

```
DS3231         Arduino Uno
──────         ───────────
VCC    ─────── 5V
GND    ─────── GND
SDA    ─────── A4
SCL    ─────── A5
```

Install a CR2032 coin cell on the RTC module for timekeeping during power loss.

---

## 12V Fan Driver (TIP122 + Flyback Diode)

The fan runs directly from the 12V battery. The Arduino only switches the transistor.

```
                    12V Battery (+)
                         │
                         │
                    ┌────┴────┐
                    │   Fan   │
                    │  (+)    │
                    └────┬────┘
                         │
                         │  Fan (-)
                         │
                    ┌────┴────┐
                    │Collector│
                    │  TIP122 │
                    │         │
         1kΩ        │  Base   │
    D8 ────/\/\/────┤         │
                    │ Emitter │
                    └────┬────┘
                         │
                         ├──── GND (common with Arduino)

    Flyback diode 1N5819 (across fan):
    Cathode (stripe) ──→ 12V (+)
    Anode            ──→ Collector of TIP122
```

### Component Notes

- **TIP122**: Darlington NPN transistor; handles fan current (check fan label for amperage).
- **1N5819**: Schottky flyback diode protects the transistor from inductive kickback when the fan switches off.
- **1kΩ base resistor**: Limits current into the transistor base from D8.

---

## Grinder Motor Stop (Optional — D9)

When `STOP_MOTOR_ON_CRITICAL` is enabled in `config.h`, the motor is stopped at 50°C and resumes when temperature drops to 45°C or below.

Use a relay module or transistor driver suitable for your motor current:

```
Relay Module       Arduino Uno
───────────        ───────────
VCC        ─────── 5V
GND        ─────── GND
IN/Signal  ─────── D9
```

Wire the motor through the relay's normally-open contacts so D9 HIGH = motor running, D9 LOW = motor stopped.

Set `#define STOP_MOTOR_ON_CRITICAL 0` in `config.h` if no motor is connected.

---

## Power System

```
  Solar Panel (12V)
        │
        ▼
  Solar Charge Controller
        │
        ▼
  12V Battery (+/−)
        │
        ├──────────────────────────► Fan (+)  [12V direct]
        │
        ▼
  7805 Voltage Regulator
   IN        OUT (5V)
   GND       GND
        │
        ▼
  Arduino Uno (Vin or 5V pin + GND)
        │
        ▼
  Sensors (DHT11, RTC, SD module)
```

### Power Wiring

```
12V Battery (+) ──→ 7805 IN
12V Battery (−) ──→ 7805 GND ──→ Arduino GND
7805 OUT (5V)   ──→ Arduino 5V (or Vin via regulator on Uno)
12V Battery (+) ──→ Fan positive terminal
```

### Critical: Common Ground

Connect **Arduino GND** to **battery negative (12V −)**. All modules must share the same ground reference.

Add a heat sink to the 7805 if the Arduino and modules draw significant current.

---

## Assembly Checklist

- [ ] All sensor modules powered from 5V
- [ ] SD card inserted before power-on
- [ ] CR2032 installed on DS3231
- [ ] Flyback diode orientation correct (stripe toward 12V+)
- [ ] Fan polarity correct
- [ ] Common ground between Arduino and 12V system
- [ ] No short between 12V and 5V rails

---

## Future Expansion Pins

The following pins remain available for optional enhancements:

| Pin | Suggested use |
|-----|---------------|
| D3–D7 | Buttons, buzzer, LED alarm |
| D9 | Motor relay (when STOP_MOTOR_ON_CRITICAL enabled) |
| D0–D1 | Software serial (WiFi/GSM modules) |
| A0–A3 | Battery voltage divider, extra sensors |
| A4–A5 | Already used by RTC (I2C bus can share other I2C devices) |
