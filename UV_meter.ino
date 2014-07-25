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

 Initially derived from tutorial code by David A. Mellis (2008), Limor Fried
 (2009, 2014), and Tom Igoe (2010).
 */

#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>

// Available at https://github.com/adafruit/Adafruit_SI1145_Library
#include "Adafruit_SI1145.h"

Adafruit_SI1145 uv = Adafruit_SI1145();

float uvIndex = 0.0;
int analogUV = 0;

const int S12SD_PIN = 0;    // analog

const int PWM_PIN = 3;      // digital PWM

const long CHIRP_DURATION = 80000;
const int CHIRP_PITCH = 800;

const float NORMAL_UV = 8.0;
const float TOO_DARK = -4.0;

const long SAMPLE_RATE = 1000; // ms
const long ONE_SECOND_MS = 1000;

long normSecond = 1000;
long now;

long time = 0;
long accumTime = 0;

double normTime = 1000.0;
double accumNormTime = 0;

// RS, E, D4 .. D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup() {
  pinMode(PWM_PIN, OUTPUT);
  chirp();

  // Serial.begin(9600);

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

  uvIndex = uv.readUV() / 100.0;
  analogUV = analogRead(S12SD_PIN);

  delay(2000);

  time = millis();
  accumTime = time;
  normTime = time;
  accumNormTime = time;

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

void printStats(float uvIndex, float analogUV) {
  // Truncate to one decimal place
  float stops = (int)(uvIndex*10) / 10.0;
  float delta = stops - NORMAL_UV;

  // Print UV index
  lcd.home();
  lcd.print("UV: ");
  lcd.print(uvIndex);
  lcd.print("            ");

  // Print straigh analog UV reading
  lcd.setCursor(10, 0);
  lcd.print(analogUV);
  lcd.print("       ");

  // Print exposure compensation (still theoretical :)
  lcd.setCursor(0, 1);
  lcd.print("Exp: ");

  if (delta < TOO_DARK) {
    lcd.print("Too dark");
    return;
  } else if (delta >= 0) {
    lcd.print("+");
  }
  lcd.print(delta);
  lcd.print("          ");
}

long calcNormSecond(float uvIndex) {
  float scale = pow(2, abs(uvIndex - NORMAL_UV));
  return (long)((uvIndex >= NORMAL_UV) ?
                (1000.0 / scale) :
                (1000.0 * scale));
}

void loop() {
  // Every second, print UV levels
  if ((accumTime - time) > ONE_SECOND_MS) {
    uvIndex = uv.readUV() / 100.0;
    analogUV = analogRead(S12SD_PIN);
    normSecond = calcNormSecond(uvIndex);

    printStats(uvIndex, analogUV);

    accumTime = millis();
    time = accumTime;
  }

  // Every normative-exposure second, chirp
  if ((accumNormTime - normTime) > normSecond) {
    chirp();

    accumNormTime = millis();
    normTime = accumNormTime;
  }

  now = millis();
  accumTime += (now - accumTime);
  accumNormTime += (now - accumNormTime);
}

