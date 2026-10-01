# Test Plan — Solar Temperature Controller

Use this checklist to verify the system after assembly and firmware upload.

**Prerequisites:**
- Arduino Uno programmed with `SolarTemperatureController.ino`
- Serial Monitor open at **115200 baud**
- All modules wired per [wiring.md](wiring.md)
- RTC set to correct date/time

---

## Test 1 — DHT11 Readings

**Objective:** Verify temperature and humidity readings appear on Serial Monitor.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Power on the system | Startup banner shows `DHT11 sensor: OK` |
| 2 | Wait for first reading cycle (2 s) | Temperature and humidity values displayed |
| 3 | Compare with a reference thermometer | Values within DHT11 tolerance (±2°C, ±5% RH) |

**Pass criteria:** Valid numeric temperature and humidity every 2 seconds.

---

## Test 2 — RTC Timestamps

**Objective:** Verify accurate date and time from DS3231.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Check startup banner | `RTC: OK` |
| 2 | Compare Serial date/time with known clock | Matches within 1 second |
| 3 | Wait 60 seconds | Time increments correctly |

**Pass criteria:** Date format `DD/MM/YYYY`, time format `HH:MM:SS`.

---

## Test 3 — SD Card CSV Logging

**Objective:** Confirm log file creation and correct CSV format.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Insert formatted MicroSD card before power-on | Startup shows `SD card: OK` |
| 2 | Run for at least 30 seconds | No SD write errors on Serial |
| 3 | Remove SD card and open `datalog.csv` on PC | Header: `Date,Time,Temperature,Humidity,Fan,Alert` |
| 4 | Verify data rows | Date `DD-MM-YYYY`, numeric temp/humidity, Fan `ON`/`OFF`, Alert status |

**Pass criteria:** CSV file exists with header and at least 15 data rows after 30 s.

---

## Test 4 — SD Card Removed (Fail-Safe)

**Objective:** System continues when SD card is absent.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Power off, remove SD card, power on | `SD card: ERROR` on Serial |
| 2 | Wait for several reading cycles | Temperature/humidity still displayed |
| 3 | Confirm no hang or reset | System runs continuously |

**Pass criteria:** Monitoring and Serial output continue without SD card.

---

## Test 5 — Fan Turns ON Above Threshold

**Objective:** Fan activates when temperature exceeds ON threshold (default 30°C).

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Note current temperature on Serial | Below 30°C initially |
| 2 | Apply warm air to DHT11 (hair dryer on low, warm breath) | Temperature rises |
| 3 | When temp ≥ 30°C | Serial shows `Fan: ON` |
| 4 | Verify fan physically spins | Audible/mechanical confirmation |

**Pass criteria:** Fan turns ON at or above `FAN_ON_TEMP` (30°C default).

---

## Test 6 — Fan Turns OFF Below Threshold (Hysteresis)

**Objective:** Fan deactivates only after temperature drops to OFF threshold (default 28°C).

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | With fan ON from Test 5 | Fan running |
| 2 | Remove heat source, allow cooling | Temperature decreases |
| 3 | When temp drops to 29°C | Fan remains ON (hysteresis zone) |
| 4 | When temp ≤ 28°C | Serial shows `Fan: OFF`, fan stops |

**Pass criteria:** Fan turns OFF at or below `FAN_OFF_TEMP` (28°C default), not before.

---

## Test 5b — High Temperature Warning

**Objective:** System reports HIGH TEMP WARNING at 40°C while fan stays ON.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Raise temperature to ≥ 40°C | Serial shows `Alert: HIGH TEMP WARNING` |
| 2 | Verify fan state | `Fan: ON` |
| 3 | Check CSV log | Alert column reads `HIGH TEMP WARNING` |

**Pass criteria:** Warning appears at `HIGH_TEMP_WARNING` (40°C default); fan remains ON.

---

## Test 5c — Critical Alert and Motor Stop

**Objective:** Critical alert at 50°C stops the motor (if enabled).

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Raise temperature to ≥ 50°C | Serial shows `Alert: CRITICAL ALERT` |
| 2 | Verify fan | `Fan: ON` |
| 3 | Verify motor (if wired) | `Motor: STOPPED` |
| 4 | Allow cooling to ≤ 45°C | `Motor: RUNNING` resumes |

**Pass criteria:** Critical alert at 50°C; motor stops and recovers at `MOTOR_RECOVERY_TEMP` (45°C default).

---

## Test 7 — RTC Disconnected (Fail-Safe)

**Objective:** Program continues without RTC module.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Power off, disconnect DS3231 SDA/SCL or module power | — |
| 2 | Power on | `RTC: ERROR` and `RTC error: module not detected` |
| 3 | Wait for reading cycles | Date/Time show `N/A` |
| 4 | Confirm temp/humidity still read | Sensor data continues |

**Pass criteria:** No crash; sensor readings continue with `N/A` timestamps.

---

## Test 8 — 24-Hour Continuous Run

**Objective:** Long-term stability, complete log, stable fan control.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Install SD card, set RTC, power from stable supply | System starts normally |
| 2 | Run continuously for 24 hours | No resets or hangs |
| 3 | Check Serial periodically | Readings every 2 s throughout |
| 4 | After 24 h, inspect `datalog.csv` | ~43,200 data rows; Alert column populated |
| 5 | Review fan ON/OFF events | No rapid chattering; hysteresis respected |

**Pass criteria:**
- Zero unexpected reboots
- Log file complete with no gaps > 10 seconds
- Fan state changes only at threshold crossings

---

## Test Results Log

| Test | Date | Tester | Pass/Fail | Notes |
|------|------|--------|-----------|-------|
| 1 | | | | |
| 2 | | | | |
| 3 | | | | |
| 4 | | | | |
| 5 | | | | |
| 6 | | | | |
| 7 | | | | |
| 8 | | | | |

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| `Sensor read failed` repeatedly | DHT11 wiring, wrong pin | Check D2, 5V, GND connections |
| `RTC: ERROR` | I2C wiring, dead CR2032 | Check A4/A5, replace battery |
| `SD card error` | Card not FAT32, bad wiring | Format SD as FAT32; check SPI pins |
| Fan always OFF | Transistor wiring, insufficient base current | Check D8 → 1kΩ → base, common GND |
| Fan always ON | Short circuit, stuck transistor | Verify D8 reads LOW when fan should be off |
| Erratic readings | Power supply noise | Add capacitor near DHT11; check 7805 output |
