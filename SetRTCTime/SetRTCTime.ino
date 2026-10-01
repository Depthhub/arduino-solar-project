// One-time sketch: sets DS3231 RTC from PC clock at compile time
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  if (!rtc.begin()) {
    Serial.println(F("ERROR: RTC not found — check SDA/A4, SCL/A5, VCC, GND"));
    while (true) { delay(1000); }
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.println(F("RTC time set successfully!"));
  printNow();

  if (rtc.lostPower()) {
    Serial.println(F("Note: RTC had lost power — battery may be missing or dead."));
  } else {
    Serial.println(F("RTC battery OK."));
  }
}

void loop() {
  delay(3000);
  printNow();
}

void printNow() {
  DateTime now = rtc.now();
  Serial.print(F("Current RTC time: "));
  Serial.print(now.day());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.year());
  Serial.print(' ');
  Serial.print(now.hour());
  Serial.print(':');
  if (now.minute() < 10) Serial.print('0');
  Serial.print(now.minute());
  Serial.print(':');
  if (now.second() < 10) Serial.print('0');
  Serial.println(now.second());
}
