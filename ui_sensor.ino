#include <avr/wdt.h>
#include <stdint.h>
#include <Adafruit_GFX.h>
#include "TouchScreen.h"
#include <MCUFRIEND_kbv.h>
#include "TouchScreen.h"
#include <Wire.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <DHT.h>
#include <EEPROM.h>

// Pin definitions
#define YP A1
#define XM A2
#define YM 7
#define XP 6
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

// Color definitions
#define BLACKM 0x18E5
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define DARKOREN BLACKM
#define MAROON 0x7800

// Touchscreen pressure thresholds
#define MINPRESSURE 80
#define MAXPRESSURE 2000

// Initialize touchscreen and TFT display
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
MCUFRIEND_kbv tft;
DHT dht(48, DHT11);

// Define sensor positions and labels
const int numSensors = 4;
const int sensorPositions[numSensors][2] = {
  { 10, 10 },
  { 165, 10 },
  { 10, 125 },
  { 165, 125 }
};
const char* sensorLabels[numSensors] = {
  "PLN",
  "Batt ",
  "Batt B",
  "Temperature"
};
String pln, batt1, batt2, suhu;

const char* satuan[numSensors] = {
  "volt",
  "volt",
  "%",
  "c"
};

// Define maximum values for sensors
const float maxValues[numSensors] = {
  20.0,  // Max value for Battery A
  11.0,  // Max value for Battery B
  11.0,  // Max value for Humidity
  30.0   // Max value for Temperature
};

void setup() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  Wire.begin(8);
  Wire.onReceive(receiveEvent);

  Serial.begin(115200);
  delay(2500);
  tft.reset();
  tft.begin(0x9341);
  tft.setRotation(1);
  tft.fillScreen(BLACKM);
  delay(1000);
}

void loop() {
  delay(500);
  String sensorValues[numSensors] = {
    getBatteryAValue(),
    getBatteryBValue(),
    getSensorCValue(),
    getSensorDValue()
  };

  for (int i = 0; i < numSensors; i++) {
    float sensorValueFloat = sensorValues[i].toFloat();
    displaySensor(sensorPositions[i][0], sensorPositions[i][1], sensorLabels[i], sensorValues[i], satuan[i], sensorValueFloat > maxValues[i]);
  }
}

void receiveEvent(int howMany) {
  String df = "";
  while (0 < Wire.available()) {
    char c = Wire.read();
    df = df + c;
  }
  Serial.print("data terima : ");
  Serial.println(df);
  pln = df.substring(0, 6);
  batt1 = df.substring(7, 12);
  batt2 = df.substring(13, 18);
  suhu = df.substring(19, 24);
}

void displaySensor(int x, int y, const char* label, String sensorValue, String satuan, bool isOverMax) {
  uint16_t bgColor = isOverMax ? RED : WHITE;         // Change background color if over max
  uint16_t textColor = isOverMax ? WHITE : DARKOREN;  // Change text color if over max

  tft.fillRoundRect(x, y, 145, 90, 10, bgColor);
  tft.setCursor(x + 10, y + 10);
  tft.setTextSize(2);
  tft.setTextColor(textColor);
  tft.print(label);

  tft.setCursor(x + 10, y + 45);
  tft.setTextSize(2);
  tft.setTextColor(textColor);
  tft.print(String(sensorValue) + " " + satuan);
}

// Placeholder functions for sensor readings, replace with actual sensor reading logic
String getBatteryAValue() {
  return pln;
}

String getBatteryBValue() {
  return batt1;
}

String getSensorCValue() {
  return batt2;
}

String getSensorDValue() {
  return suhu;
}
