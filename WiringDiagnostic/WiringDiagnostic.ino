// Wiring diagnostic — upload this to find RTC and SD problems
// Disconnect fan/12V for this test. USB power only.

#include <Wire.h>
#include <SPI.h>
#include <SD.h>

const int SD_CS = 10;

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println(F("========================================"));
  Serial.println(F(" WIRING DIAGNOSTIC"));
  Serial.println(F("========================================"));
  Serial.println();

  // --- Power rail reminder ---
  Serial.println(F("[1] BREADBOARD POWER (most common mistake)"));
  Serial.println(F("    Arduino 5V  -> breadboard RED (+) row"));
  Serial.println(F("    Arduino GND -> breadboard BLUE (-) row"));
  Serial.println(F("    Without these, modules get NO power!"));
  Serial.println();

  // --- I2C / RTC test ---
  Serial.println(F("[2] RTC / I2C TEST (SDA=A4, SCL=A5)"));
  Wire.begin();
  Wire.setWireTimeout(1000, true);

  bool found68 = false;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("    Found I2C device at 0x"));
      if (addr < 16) Serial.print('0');
      Serial.println(addr, HEX);
      if (addr == 0x68) found68 = true;
    }
  }

  if (found68) {
    Serial.println(F("    RTC: DETECTED at 0x68 - wiring OK"));
  } else {
    Serial.println(F("    RTC: NOT FOUND"));
    Serial.println(F("    Fix: SDA->A4, SCL->A5, VCC->5V, GND->GND"));
    Serial.println(F("    Try swapping SDA and SCL once"));
  }
  Serial.println();

  // --- SD / SPI test ---
  Serial.println(F("[3] SD CARD TEST (CS=D10, MOSI=D11, MISO=D12, CLK=D13)"));
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin();
  delay(500);

  bool sdOk = false;
  if (SD.begin(4000000, SD_CS)) sdOk = true;
  else if (SD.begin(1000000, SD_CS)) sdOk = true;
  else if (SD.begin(400000, SD_CS)) sdOk = true;

  if (sdOk) {
    Serial.println(F("    SD: DETECTED - wiring OK"));
    File f = SD.open("diag.txt", FILE_WRITE);
    if (f) {
      f.println(F("test"));
      f.close();
      Serial.println(F("    SD: Write test PASSED"));
    } else {
      Serial.println(F("    SD: Found but WRITE FAILED"));
    }
  } else {
    Serial.println(F("    SD: NOT FOUND"));
    Serial.println(F("    Fix: use 8GB card, 3V3->5V, CS->D10"));
    Serial.println(F("    Try swapping MISO(D12) and MOSI(D11)"));
  }
  Serial.println();

  // --- DHT pin check ---
  Serial.println(F("[4] PIN SUMMARY"));
  Serial.println(F("    DHT11 DATA -> D2"));
  Serial.println(F("    RTC SDA    -> A4"));
  Serial.println(F("    RTC SCL    -> A5"));
  Serial.println(F("    SD CS      -> D10"));
  Serial.println(F("    SD MOSI    -> D11"));
  Serial.println(F("    SD MISO    -> D12"));
  Serial.println(F("    SD CLK     -> D13"));
  Serial.println();

  Serial.println(F("========================================"));
  Serial.println(F(" Diagnostic complete. Read results above."));
  Serial.println(F("========================================"));
}

void loop() {
  delay(10000);
}
