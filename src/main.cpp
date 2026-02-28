#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Wire.begin(21, 22); // SDA, SCL for ESP32

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while(true);
  }

  display.clearDisplay();

  // Small font
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // start a little down from top and use a variable to space lines
  int16_t y = 0;
  display.setCursor(10, y);
  display.print("No system installed.");

  y += 10;
  display.setCursor(10, y);
  display.print("Download the latest:");

  y += 10;
  display.setCursor(10, y);
  display.print("https://github.com/planetyadv/SignalFox");

  y += 20;
  display.setCursor(10, y);
  display.print("ERROR CODE:");

  y += 10;
  display.setCursor(10, y);
  display.print("No system found");

  display.display();
}

void loop() {
  // Nothing here
}