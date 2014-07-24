/*
 UV index meter

 Using an Adafruit Si1145 UV Sensor and a generic 16x2 LCD, display the ambient
 UV index.

  Wiring:
  - PWM pin D3
  - LCD RS pin to digital pin D7
  - LCD Enable pin to digital pin D8
  - LCD D4 pin to digital pin D9
  - LCD D5 pin to digital pin D10
  - LCD D6 pin to digital pin D11
  - LCD D7 pin to digital pin D12
  - LCD R/W pin to ground
  - 10K resistor:
      - ends to +5V and ground
      - wiper to LCD VO pin (pin 3)
  - GUVA-S12SD out to Analog pin 0
  - Si1145 SCL pin to Analog pin 5
  - Si1145 SDA pin to Analog pin 4

 Based on tutorial code by David A. Mellis (2008), Limor Fried (2009, 2014),
 and Tom Igoe (2010).
 */

#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>

// Available at https://github.com/adafruit/Adafruit_SI1145_Library
#include "Adafruit_SI1145.h"

Adafruit_SI1145 uv = Adafruit_SI1145();

float uvIndex;
sensors_event_t event;

const int S12SD_PIN = 0;    // analog

const int PWM_PIN = 3;      // digital PWM
const long CHIRP_DURATION = 80000;
const int CHIRP_PITCH = 800;

const float NORMAL_UV = 8.0;
const float SAMPLE_RATE = 1000;

// RS, E, D4 .. D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup() {
  pinMode(PWM_PIN, OUTPUT);
  chirp();

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  lcd.print("Ohai! :)");

  if (!uv.begin()) {
    lcd.setCursor(0, 0);
    lcd.print("Oh noes! Broken!");
    lcd.setCursor(0, 1);
    lcd.print("No Si1145");
    while (1);
  }

  delay(2000);

  lcd.clear();
}

void chirp() {
  // Chirp the piezo speaker using one of the PWM digital outs
  long t = 0;
  while (t < CHIRP_DURATION) {
    digitalWrite(PWM_PIN, HIGH);
    delayMicroseconds(CHIRP_PITCH / 2);
    digitalWrite(PWM_PIN, LOW);
    delayMicroseconds(CHIRP_PITCH / 2);
    t += CHIRP_PITCH;
  }
}

void printUVIndex(float uvIndex) {
  lcd.home();
  lcd.print("UV: ");
  lcd.print(uvIndex);
  lcd.print("        ");

  lcd.setCursor(10, 0);

  // The EPA UV Index, http://www2.epa.gov/sunwise/uv-index-scale
  if (uvIndex <= 2.0) {
    lcd.print("Low   ");
  }
  else if (uvIndex <= 5.0) {
    lcd.print("Mod   ");
  }
  else if (uvIndex <= 7.0) {
    lcd.print("High  ");
  }
  else if (uvIndex <= 10) {
    lcd.print("V High");
  }
  else {
    lcd.print("Yikes ");
  }
}

void getAndPrintAnalogUV() {
  lcd.setCursor(0, 1);
  lcd.print(analogRead(S12SD_PIN));
  lcd.print("       ");
}

void loop() {
  uvIndex = uv.readUV() / 100.0;

  printUVIndex(uvIndex);
  getAndPrintAnalogUV();

  //chirp();

  delay(SAMPLE_RATE);
}

