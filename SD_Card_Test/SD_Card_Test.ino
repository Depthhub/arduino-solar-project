// SD-only test — upload this alone to test SD card wiring
// If this works, re-upload SolarTemperatureController.ino

#include <SPI.h>
#include <SD.h>

const int CS_PIN = 10;

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Serial.println(F("=== SD CARD TEST ==="));
  Serial.println(F("Expected wiring:"));
  Serial.println(F("  GND->GND  MISO->D12  CLK->D13"));
  Serial.println(F("  MOSI->D11  CS->D10   3V3->5V"));
  Serial.println();

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();
  delay(500);

  bool ok = false;
  if (SD.begin(4000000, CS_PIN)) ok = true;
  else if (SD.begin(1000000, CS_PIN)) ok = true;
  else if (SD.begin(400000, CS_PIN)) ok = true;

  if (!ok) {
    Serial.println(F("FAIL - SD not detected"));
    Serial.println(F("Check wires and card insertion"));
    return;
  }

  Serial.println(F("SUCCESS - SD card detected!"));

  File f = SD.open("test.txt", FILE_WRITE);
  if (f) {
    f.println(F("Hello from Arduino"));
    f.close();
    Serial.println(F("Wrote test.txt OK"));
  } else {
    Serial.println(F("FAIL - could not write file"));
  }
}

void loop() {
  delay(5000);
  Serial.println(F("SD test running..."));
}
