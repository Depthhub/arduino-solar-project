#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>

// ==========================================
// PIN & HARDWARE CONFIGURATION
// ==========================================
#define DHT_IN_PIN       2    // Inlet DHT11 sensor
#define DHT_CHAMB_PIN    3    // Chamber DHT22 sensor
#define DHT_OUT_PIN      5    // Exhaust/Outlet DHT11 sensor
#define DHTTYPE_11       DHT11
#define DHTTYPE_22       DHT22

#define RELAY_PIN        7    // 5V Relay module control pin (Active-LOW)
#define SD_CS_PIN        10   // Chip Select pin for SD Card module

// ==========================================
// LOGGING & TIMING SETTINGS
// ==========================================
const unsigned long DISPLAY_INTERVAL = 30000UL;  // 30 seconds for Serial display & control
const unsigned long SD_LOG_INTERVAL  = 300000UL; // 5 minutes (300,000 ms) for SD logging

unsigned long lastDisplayTime = 0;
unsigned long lastLogTime     = 0;

// Initialize Sensors & Hardware
DHT dht_in(DHT_IN_PIN, DHTTYPE_11);
DHT dht_chamb(DHT_CHAMB_PIN, DHTTYPE_22);
DHT dht_out(DHT_OUT_PIN, DHTTYPE_11);

RTC_DS3231 rtc;
File dataFile;

char currentFileName[13]; // Stores 8.3 format filename (e.g., "D092526.CSV")
int currentDay = -1;      // Tracks date changes for daily file rotation

// ==========================================
// HELPER: PSYCHROMETRIC ENTHALPY (kJ/kg)
// ==========================================
float calculateEnthalpy(float temp, float rh) {
  if (isnan(temp) || isnan(rh)) return 0.0;
  float p_sat = 0.61078 * exp((17.27 * temp) / (temp + 237.3)); // Saturation vapor pressure (kPa)
  float p_v = (rh / 100.0) * p_sat;                             // Vapor pressure (kPa)
  float w = 0.622 * (p_v / (101.325 - p_v));                   // Humidity ratio (kg water/kg dry air)
  return (1.006 * temp) + w * (2501.0 + 1.805 * temp);          // Specific enthalpy (kJ/kg)
}

// ==========================================
// HELPER: DAILY FILE ROTATION & HEADERS
// ==========================================
void updateDailyFileName(DateTime now) {
  // Enforces 8.3 filename format: "DMMDDYY.CSV" (e.g., D092526.CSV)
  snprintf(currentFileName, sizeof(currentFileName), "D%02d%02d%02d.CSV", now.month(), now.day(), now.year() % 100);

  // If the date changed or file does not exist, write CSV headers
  if (now.day() != currentDay || !SD.exists(currentFileName)) {
    currentDay = now.day();
    dataFile = SD.open(currentFileName, FILE_WRITE);
    if (dataFile) {
      dataFile.println(F("Timestamp,Tin_C,RHin_%,h_in_kJkg,Tchamber,RHchamb,h_chamb,Tout_C,RHout_%,h_out_kJkg,Fan_Mode,Relay_State"));
      dataFile.close();
      Serial.print(F("New daily log file created: "));
      Serial.println(currentFileName);
    } else {
      Serial.println(F("Error creating new daily file!"));
    }
  }
}

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(9600);

  // Configure Relay Pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Ensure Active-LOW relay is OFF on boot (HIGH = OFF)

  // Initialize Sensors
  dht_in.begin();
  dht_chamb.begin();
  dht_out.begin();

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println(F("RTC Module Not Found!"));
  }

  // =========================================================================
  // RTC TIME SYNC
  // Automatically sets the RTC to your computer's compile time if the RTC 
  // loses power (e.g., dead coin cell battery).
  // =========================================================================
  if (rtc.lostPower()) {
    Serial.println(F("RTC lost power! Syncing to computer compile time..."));
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize SD Card
  Serial.print(F("Initializing SD card... "));
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("Card initialization failed!"));
  } else {
    Serial.println(F("SD Card initialized successfully."));
  }

  // Set up initial filename for today
  DateTime now = rtc.now();
  updateDailyFileName(now);
}

// ==========================================
// MAIN LOOP
// ==========================================
void loop() {
  unsigned long currentMillis = millis();

  // Static variables persist between loop iterations, allowing independent tasks to share them
  static char timeBuffer[20] = "N/A";
  static float Tin = 0, RHin = 0, Tchamb = 0, RHchamb = 0, Tout = 0, RHout = 0;
  static float hin = 0, hchamb = 0, hout = 0;
  static const char* fanMode = "OFF";
  static int relayState = 0;

  // ------------------------------------------------------------------------
  // TASK 1: SERIAL MONITOR DISPLAY & CONTROL (EVERY 30 SECONDS)
  // ------------------------------------------------------------------------
  if (currentMillis - lastDisplayTime >= DISPLAY_INTERVAL || lastDisplayTime == 0) {
    lastDisplayTime = currentMillis;

    DateTime now = rtc.now();
    updateDailyFileName(now); // Check if date shifted at midnight

    // Format Timestamp String (YYYY/MM/DD HH:MM:SS)
    snprintf(timeBuffer, sizeof(timeBuffer), "%04d/%02d/%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());

    // Read Sensors
    Tin = dht_in.readTemperature();
    RHin = dht_in.readHumidity();
    Tchamb = dht_chamb.readTemperature();
    RHchamb = dht_chamb.readHumidity();
    Tout = dht_out.readTemperature();
    RHout = dht_out.readHumidity();

    // Calculate Enthalpies
    hin = calculateEnthalpy(Tin, RHin);
    hchamb = calculateEnthalpy(Tchamb, RHchamb);
    hout = calculateEnthalpy(Tout, RHout);

    // ==========================================
    // MULTI-TIER SETPOINT MATRIX EVALUATION
    // ==========================================
    fanMode = "OFF";

    if (isnan(Tchamb)) {
      fanMode = "ON"; // FAIL-SAFE: Chamber sensor broken/unplugged. Force fan ON to prevent baking!
    }
    else if (Tchamb >= 50.0) {
      fanMode = "ON";
    } 
    else if (Tchamb >= 45.0 && Tchamb < 50.0) {
      if (RHchamb > 70.0) {
        fanMode = "ON";          // High Moisture Extraction
      } else if (RHchamb >= 55.0) {
        fanMode = "INTERMITT";   // Modulated Extraction
      } else {
        fanMode = "OFF";         // Finishing Phase (<55% RH)
      }
    } 
    else if (Tchamb >= 35.0 && Tchamb < 45.0) {
      if (RHchamb > 70.0) {
        fanMode = "ON";          // Active Purge
      } else if (RHchamb >= 60.0) {
        fanMode = "INTERMITT";   // Balanced Drying
      } else {
        fanMode = "OFF";         // Heat Accumulation (<60% RH)
      }
    } 
    else { // Tchamb < 35.0°C
      if (RHchamb >= 80.0) {
        fanMode = "INTERMITT";   // Moisture Flush (High-humidity morning spike)
      } else {
        fanMode = "OFF";         // Pre-Heating (<80% RH)
      }
    }

    // ==========================================
    // EXECUTE ACTIVE-LOW RELAY OUTPUT
    // ==========================================
    static int intermittStep = 0; // Tracks the four 30-second cycles (0 to 3)

    if (fanMode == "OFF") {
      digitalWrite(RELAY_PIN, HIGH); // HIGH = De-energizes Active-LOW relay (Fan OFF)
      relayState = 0;               // Logged as 0 (OFF)
      intermittStep = 0;            // Reset so it starts fresh next time
    } 
    else if (fanMode == "ON") {
      digitalWrite(RELAY_PIN, LOW);  // LOW = Energizes Active-LOW relay (Fan ON)
      relayState = 1;               // Logged as 1 (ON)
      intermittStep = 0;            // Reset
    }
    else if (fanMode == "INTERMITT") {
      // Option 1: 30s ON, 90s OFF (Total 120s = 4 steps of 30s each)
      if (intermittStep == 0) {
        digitalWrite(RELAY_PIN, LOW); // Step 0: Fan ON for 30s
        relayState = 1;
      } else {
        digitalWrite(RELAY_PIN, HIGH); // Steps 1, 2, 3: Fan OFF for 90s
        relayState = 0;
      }
      
      // Advance the cycle step (0 -> 1 -> 2 -> 3 -> 0)
      intermittStep++;
      if (intermittStep >= 4) {
        intermittStep = 0;
      }
    }

    // ==========================================
    // DISPLAY LIVE READINGS ON SERIAL MONITOR
    // ==========================================
    Serial.println(F("----------------------------------------------------------------------"));
    Serial.print(F("TIME    -> ")); Serial.println(timeBuffer);
    Serial.print(F("INLET   -> T: ")); Serial.print(Tin, 1); Serial.print(F("C | RH: ")); Serial.print(RHin, 1); Serial.print(F("% | h: ")); Serial.print(hin, 2); Serial.println(F(" kJ/kg"));
    Serial.print(F("CHAMBER -> T: ")); Serial.print(Tchamb, 1); Serial.print(F("C | RH: ")); Serial.print(RHchamb, 1); Serial.print(F("% | h: ")); Serial.print(hchamb, 2); Serial.println(F(" kJ/kg"));
    Serial.print(F("OUTLET  -> T: ")); Serial.print(Tout, 1); Serial.print(F("C | RH: ")); Serial.print(RHout, 1); Serial.print(F("% | h: ")); Serial.print(hout, 2); Serial.println(F(" kJ/kg"));
    Serial.print(F("SYSTEM  -> Mode: ")); Serial.print(fanMode); Serial.print(F(" | Relay: ")); Serial.print(relayState == 1 ? "ON (LOW)" : "OFF (HIGH)");
    Serial.print(F(" | File: ")); Serial.println(currentFileName);
  } // END OF TASK 1

  // ------------------------------------------------------------------------
  // TASK 2: LOG DATA TO SD CARD (EVERY 5 MINUTES)
  // ------------------------------------------------------------------------
  if (currentMillis - lastLogTime >= SD_LOG_INTERVAL || lastLogTime == 0) {
    lastLogTime = currentMillis;

    // Wait until we have a valid timestamp before logging the first time
    if (timeBuffer[0] != 'N') {
      dataFile = SD.open(currentFileName, FILE_WRITE);
      
      // SD Hot-Swap Recovery Mechanism
      if (!dataFile) {
        Serial.println(F("Status  -> ERROR writing to SD. Attempting recovery..."));
        SD.end();
        delay(100);
        SD.begin(SD_CS_PIN);
        dataFile = SD.open(currentFileName, FILE_WRITE);
      }

      if (dataFile) {
        // 1. Formatted Timestamp
        dataFile.print(timeBuffer); dataFile.print(',');

        // 2. Inlet Data
        dataFile.print(Tin, 1); dataFile.print(',');
        dataFile.print(RHin, 1); dataFile.print(',');
        dataFile.print(hin, 2); dataFile.print(',');

        // 3. Chamber Data
        dataFile.print(Tchamb, 1); dataFile.print(',');
        dataFile.print(RHchamb, 1); dataFile.print(',');
        dataFile.print(hchamb, 2); dataFile.print(',');

        // 4. Exhaust Data
        dataFile.print(Tout, 1); dataFile.print(',');
        dataFile.print(RHout, 1); dataFile.print(',');
        dataFile.print(hout, 2); dataFile.print(',');

        // 5. System Status
        dataFile.print(fanMode); dataFile.print(',');
        dataFile.println(relayState);

        dataFile.close(); // Save and close file
        Serial.println(F("Status  -> Saved to SD card successfully (5-min log)."));
      } else {
        Serial.println(F("Status  -> SD Recovery failed!"));
      }
    }
  }
}
